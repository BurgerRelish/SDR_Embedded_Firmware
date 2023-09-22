#include "MQTTClient.h"
#include <strings.h>
#include <esp_brotli.h>

struct CallbackQueueItem {
    char* topic;
    char* payload;
    uint32_t length;
};

struct IncomingMessage {
    char* topic;
    char* message;
};

struct OutgoingMessage {
    size_t topic_number;
    char* message;
};

QueueHandle_t callback_queue;

void communicationTask(void* parent);

MQTTClient::MQTTClient(Client& connection) : conn_client(connection) {
    incoming_messages_queue = xQueueCreate(5, sizeof(IncomingMessage));
    outgoing_messages_queue = xQueueCreate(5, sizeof(OutgoingMessage));
}


MQTTClient::~MQTTClient() {
    vTaskDelete(task_handle);
    vQueueDelete(incoming_messages_queue);
    vQueueDelete(outgoing_messages_queue);
    vQueueDelete(callback_queue);
}

/**
 * @brief Creates the RTOS task for the MQTT client.
 * 
 * @return true 
 * @return false 
 */
bool MQTTClient::begin(const char* clientid, const char* token, const char* url, uint16_t port) {
    client_id = ps::string(clientid);
    auth_token = ps::string(token);
    broker_url = ps::string(url);
    broker_port = port;

    return xTaskCreate(
        communicationTask,
        "Communication Task",
        64 * 1024,
        this,
        1,
        &task_handle
    ) == pdTRUE;
}


/**
 * @brief Get a new serializer object to load message into.
 * 
 * @param topic_number - The number of the topic to publish to when the class goes out of scope.
 * @param size - The size of the underlying JSON document.
 * @return std::shared_ptr<MessageSerializer> 
 */
std::shared_ptr<MessageSerializer> MQTTClient::new_outgoing_message(size_t topic_number,size_t size) {
    return ps::make_shared<MessageSerializer>(MQTTClient::shared_from_this(), topic_number, size);
}

/**
 * @brief Put a message on the send queue.
 * 
 * @param message 
 */
void MQTTClient::send_message(size_t topic, ps::string message) {
    char* str = (char*) heap_caps_calloc(message.size() + 1, sizeof(char), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    memccpy(str, message.c_str(), 0, message.size());

    ESP_LOGV("MQTT", "Added message to send queue: %s", str);

    OutgoingMessage new_message = {
        topic,
        str
    };

    if (xQueueSendToBack(outgoing_messages_queue, &new_message, portMAX_DELAY) != pdTRUE) ESP_LOGE("MQTT", "Failed to place message on send queue.");

    return;
}

/**
 * @brief Get the number of incoming messages in the queue.
 * 
 * @return The number of items in the receive queue.
 */
size_t MQTTClient::incoming_message_count() {
    return uxQueueMessagesWaiting(incoming_messages_queue);
}

/**
 * @brief Get a new message from the incoming message queue. Returns a null std::shared_ptr<MessageDeserializer> if queue receive failed.
 * 
 * @return std::shared_ptr<MessageDeserializer> The message.
 */
std::shared_ptr<MessageDeserializer> MQTTClient::get_incoming_message() {
    IncomingMessage new_message;

    if (xQueueReceive(incoming_messages_queue, &new_message, 50 / portTICK_PERIOD_MS) != pdTRUE) {
        free(new_message.topic);
        free(new_message.message);
        return std::shared_ptr<MessageDeserializer>();
    }
    ps::string new_string = new_message.message;
    auto ret = ps::make_shared<MessageDeserializer>(new_message.topic, new_message.message);

    free(new_message.topic);
    free(new_message.message);

    return ret;
}

/**
 * ==============================================
 * |                                            |
 * |                 RTOS TASK                  |
 * |                                            |
 * ==============================================
 */

std::tuple<ps::vector<ps::string>, ps::vector<ps::string>, ps::vector<ps::string>> get_topics(ps::string& client_id, ps::string& token);
ps::string get_token_body(ps::string& token);
ps::vector<ps::string> load_topics(std::shared_ptr<PubSubClient> client, ps::string client_id, ps::string auth_token);
ps::string get_message_payload(CallbackQueueItem& message);
ps::string compress_message(OutgoingMessage& message);

void mqtt_callback(char* topic, uint8_t* payload, uint32_t length) {
    size_t len_topic = strlen(topic);
    CallbackQueueItem new_message = {
        // Allocate memory on PSRAM for strings.
        (char*) heap_caps_calloc(len_topic + 1, sizeof(char), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM),
        (char*) heap_caps_calloc(length + 1, sizeof(char), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM),
        length
    };

    // Copy data to allocated memory
    memccpy(new_message.topic, topic, 0, len_topic);
    memccpy(new_message.payload, payload, 0, length);
    
    // Put new message on queue
    xQueueSendToBack(callback_queue, &new_message, portMAX_DELAY);
    return;
}

void communicationTask(void* parent) {
    ESP_LOGI("RTOS", "Communications task started.");
    MQTTClient* client = (MQTTClient*) parent;
    callback_queue = xQueueCreate(5, sizeof(CallbackQueueItem));
    auto mqtt_client = ps::make_shared<PubSubClient>();
    mqtt_client -> setClient(client -> conn_client);
    mqtt_client -> setCallback(mqtt_callback);
    mqtt_client -> setBufferSize(16384);
    mqtt_client -> setServer(client -> broker_url.c_str(), client -> broker_port);

    while (!mqtt_client -> connected()) {
        ESP_LOGE("MQTT", "Attempting to connect...");

        // Attempt to connect
        if (mqtt_client -> connect(client -> client_id.c_str(), client -> auth_token.c_str(), "0")) { // No password for JWT authentication.
            ESP_LOGI("MQTT", "Connected.");
        } else {
            ESP_LOGE("MQTT", "Failed to connect to broker, state: %d.", mqtt_client -> state());
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
    ESP_LOGI("MQTT", "Decoding JWT for ACL...");

    /* Fetch and connect to topics as per ACL in auth token. */
    ps::vector<ps::string> publish_topics = load_topics(mqtt_client, client -> client_id, client -> auth_token);

    IncomingMessage incoming_message;
    OutgoingMessage outgoing_message;
    CallbackQueueItem compressed_message;

    uint64_t elapsed_tm = 0;

    while(1) {
        elapsed_tm = millis();
        // Check for messages to send.
        if (xQueueReceive(client -> outgoing_messages_queue, &outgoing_message, 50 / portTICK_PERIOD_MS) == pdTRUE ) {
            /* Compress the message, then publish it to the topic. */
            uint64_t start_tm = millis();
            auto message = compress_message(outgoing_message);
            ESP_LOGV("MQTT", "Sending Message: %s", message.c_str());
            if (!mqtt_client -> publish(publish_topics.at(outgoing_message.topic_number).c_str(), message.c_str())) ESP_LOGE("MQTT", "Failed to publish message.");
            ESP_LOGV("MQTT", "Compression took %ums.", millis() - start_tm);
        }

        // Service MQTT client.
        mqtt_client -> loop();

        // Check for received messages.
        if (xQueueReceive(callback_queue, &compressed_message, 50 / portTICK_PERIOD_MS) == pdTRUE ) {
            /* Decompress the message, and then copy it to a new buffer, then send to the incoming message queue.*/
            uint64_t start_tm = millis();
            ps::string payload = get_message_payload(compressed_message);

            incoming_message.message = (char*) heap_caps_calloc(payload.size() + 1, sizeof(char), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
            incoming_message.topic = compressed_message.topic;
            memccpy(incoming_message.message, payload.c_str(), 0, payload.size());
            ESP_LOGV("MQTT", "Decompression took %ums.", millis() - start_tm);
            xQueueSendToBack(client -> incoming_messages_queue, &incoming_message, portMAX_DELAY);
        }

        elapsed_tm = millis() - elapsed_tm;
        ESP_LOGV("RTOS", "Comms task looped, took %ums", elapsed_tm);
        vTaskDelay((1000 - elapsed_tm) / portTICK_PERIOD_MS );
    }

    ESP_LOGE("RTOS", "Communication task escaped loop.");
    vTaskDelete(NULL);
}

/**
 * @brief Get the body of the auth token. 
 * 
 * @param token 
 * @return ps::string The JSON formatted, base64 decoded string of the body contents.
 */
ps::string get_token_body(ps::string& token) {
    size_t body_start = token.find_first_of('.') + 1; // Skip the found character.
    size_t body_end = token.find_last_of('.');
    ps::string body = token.substr(body_start, body_end - body_start);
    return base64_decode(body);
}

/**
 * @brief Replace the ${clientid} placeholder in the topic.
 * 
 * @param client_id 
 * @param topic 
 * @return std::string 
 */
ps::string replace_placeholder(const ps::string& client_id, const ps::string& topic) {
    // Find the position of '${clientid}' in the topic string
    size_t pos = topic.find("${clientid}");

    // If the placeholder is found, replace it with the client_id
    if (pos != ps::string::npos) {
        return topic.substr(0, pos) + client_id + topic.substr(pos + 11);
    }

    // If the placeholder is not found, return the original topic string
    return topic;
}

/**
 * @brief Get the topics from the auth token ACL.
 * 
 * @param token - Auth token to get ACL from.
 * @return std::tuple<ps::vector<ps::string>, ps::vector<ps::string>, ps::vector<ps::string>> Publish, Subscribe and All topics.
 */
std::tuple<ps::vector<ps::string>, ps::vector<ps::string>, ps::vector<ps::string>> get_topics(ps::string& client_id, ps::string& token) {
    auto document = DynamicPSRAMJsonDocument(1024);
    ps::string token_body = get_token_body(token);
    ESP_LOGV("MQTT", "Got token body: %s", token_body.c_str());
    auto result = deserializeJson(document, &token_body[0]);
    if (result != DeserializationError::Ok) ESP_LOGE("MQTT", "Failed to deserialize topics: %d.", result);

    auto pub = document["acl"]["pub"].as<JsonArray>();
    auto sub = document["acl"]["sub"].as<JsonArray>();
    auto all = document["acl"]["all"].as<JsonArray>();

    ps::vector<ps::string> publish_topics;
    for (auto topic : pub) {
        ps::string topic_str = topic.as<ps::string>();
        topic_str = replace_placeholder(client_id, topic_str);
        publish_topics.push_back(topic_str);
        ESP_LOGI("MQTT", "Publish topic: %s", topic_str.c_str());
    }

    ps::vector<ps::string> subscribe_topics;
    for (auto topic : sub) {
        ps::string topic_str = topic.as<ps::string>();
        topic_str = replace_placeholder(client_id, topic_str);
        subscribe_topics.push_back(topic_str);
        ESP_LOGI("MQTT", "Subscribe topic: %s", topic_str.c_str());
    }

    ps::vector<ps::string> all_topics;
    for (auto topic : all) {
        ps::string topic_str = topic.as<ps::string>();
        topic_str = replace_placeholder(client_id, topic_str);
        all_topics.push_back(topic_str);
        ESP_LOGI("MQTT", "All topic: %s", topic_str.c_str());
    }

    return std::make_tuple(publish_topics, subscribe_topics, all_topics);
}

/**
 * @brief Loads the topics in the ACL from the auth token. Subscribes to required topics.
 * 
 * @param client_id 
 * @param auth_token 
 * @return ps::string Publish topic list.
 */
ps::vector<ps::string> load_topics(std::shared_ptr<PubSubClient> client, ps::string client_id, ps::string auth_token) {
    
    auto topics = get_topics(client_id, auth_token);
    ps::vector<ps::string> publish_topics = std::get<0>(topics);

    for (auto topic : std::get<1>(topics)) { // Subscribe topics
        client -> subscribe(topic.c_str());
    }

    for (auto topic : std::get<2>(topics)) { // All topics
        client -> subscribe(topic.c_str());
        publish_topics.push_back(topic);  
    }
    
    return publish_topics;
}

/**
 * @brief Get the message payload from the callback queue item. Attempts to deserialize it with JSON, then deocmpresses it. Returns the string directly if no encoding specified, else an empty string.
 *  Frees the memory after conversion to ps::string.
 * @param message 
 * @return ps::string - Decompressed message payload.
 */
ps::string get_message_payload(CallbackQueueItem& message) {
    ps::string str = message.payload;
    free(message.payload);
    ESP_LOGI("MQTT", "Got message: %s", str.c_str());
    DynamicPSRAMJsonDocument temp_doc(JSON_DOCUMENT_SIZE);

    auto result = deserializeJson(temp_doc, &str[0]);
    if (result != DeserializationError::Ok) {
        ESP_LOGE("MQTT", "Deserialization Error: %d", result);
        return ps::string();
    }

    ps::string decompress_str;
    if (temp_doc.containsKey("enc")) {
        if (temp_doc["enc"] == "br") {
            decompress_str = temp_doc["msg"].as<ps::string>();
            return brotli::decompress(decompress_str);
        } else {
            ESP_LOGE("MQTT", "Unknown message encoding. Ignoring...");
            return ps::string();
        }
    }

    ESP_LOGI("MQTT", "Direct message.");
    return str;

}

/**
 * @brief Compresses a message, places it in the correct transmission format, then returns the string ready to be published.
 * Frees the memory after conversion to ps::string.
 * @param message 
 * @return ps::string 
 */
ps::string compress_message(OutgoingMessage& message) {
    ps::string buffer = message.message;
    free(message.message);

    ps::string compressed_message = brotli::compress(buffer);

    if (compressed_message.empty()) return ps::string();

    DynamicPSRAMJsonDocument document(JSON_DOCUMENT_SIZE);

    document["enc"] = "br";
    document["msg"] = compressed_message.c_str();

    ps::ostringstream serialized_message;
    if (serializeJson(document, serialized_message) == 0) ESP_LOGE("MQTT", "JSON Serialization failed.");

    return serialized_message.str();
}



MessageDeserializer::MessageDeserializer(const char* topic,const char* message) : document(DynamicPSRAMJsonDocument(JSON_DOCUMENT_SIZE)) {
    uint64_t start_time = millis();

    MessageDeserializer::topic = ps::string(topic);
    MessageDeserializer::message = ps::string(message);
    
    auto result = deserializeJson(document, &message[0]);
    if (result != DeserializationError::Ok) {
        ESP_LOGE("MQTT", "Deserialization Error: %d", result);
    }

    start_time = millis() - start_time;
    ESP_LOGV("MQTT", "Incoming message on topic: %s", MessageDeserializer::topic.c_str());
    ESP_LOGV("MQTT", "Incoming message: %s", MessageDeserializer::message.c_str());
    ESP_LOGV("MQTT", "Deserialized JSON. [Took %uus]", start_time);
}


/**
 * @brief Construct a new Message Serializer:: Message Serializer object. The JSON Document is automatically serialized, compressed, and sent using the provided MQTT Client to the
 * server on the provided topic number when this class is destructed/runs out of scope.
 * 
 * @param client 
 * @param json_document_size 
 */
MessageSerializer::MessageSerializer(std::shared_ptr<MQTTClient> client, size_t topic_number, size_t json_document_size) :
    mqtt_client(client),
    document(DynamicPSRAMJsonDocument(json_document_size)),
    topic(topic_number)
{
}

MessageSerializer::~MessageSerializer() {
    ps::ostringstream message;
    serializeJson(document, message);
    mqtt_client->send_message(topic, message.str());
}
