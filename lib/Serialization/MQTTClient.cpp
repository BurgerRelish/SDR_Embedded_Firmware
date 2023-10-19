#include "MQTTClient.h"
#include <strings.h>
#include <esp_brotli.h>

struct CallbackQueueItem { // The structure of the compressed messages from the PubSubClient callback.
    char* topic;
    char* payload;
    uint32_t length;
};

struct IncomingMessage { // Structure of messages provided by the task after decompression.
    char* topic;
    char* message;
};

struct OutgoingMessage { // Structure of messages added to the task send queue.
    size_t topic_number;
    char* message;
};

QueueHandle_t callback_queue; // Queue for message ingestion.

void communicationTask(void* parent);

/**
 * ==============================================
 * |                                            |
 * |                 MQTTClient                 |
 * |                                            |
 * ==============================================
 */

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
bool MQTTClient::begin(const char* clientid, const char* token) {
    client_id = ps::string(clientid);
    auth_token = ps::string(token);

    return xTaskCreate(
        communicationTask,
        "Communication Task",
        64 * 1024,
        this,
        2,
        &task_handle
    ) == pdTRUE;
}

/**
 * @brief Connect to the MQTT Broker. Blocks until connection is successful, checking every 5 seconds.
 * 
 */
void MQTTClient::connect() {
    while (! mqtt_client -> connected()) {
    ESP_LOGE("MQTT", "Attempting to connect...");

    // Attempt to connect
    if (mqtt_client -> connect(client_id.c_str(), auth_token.c_str(), "0")) { // No password for JWT authentication.
        ESP_LOGI("MQTT", "Connected.");
    } else {
        ESP_LOGE("MQTT", "Failed to connect to broker, state: %d.", mqtt_client -> state());
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
}

/**
 * @brief Get a new serializer object to load message into.
 * 
 * @param topic_number - The number of the topic to publish to when the class goes out of scope.
 * @param size - The size of the underlying JSON document.
 * @return std::shared_ptr<MessageSerializer> 
 */
std::shared_ptr<MessageSerializer> MQTTClient::createMessage(size_t topic_number,size_t size) {
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

    if (xQueueSendToBack(outgoing_messages_queue, &new_message, 250 / portTICK_PERIOD_MS) != pdTRUE) ESP_LOGE("MQTT", "Failed to place message on send queue.");

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
std::shared_ptr<MessageDeserializer> MQTTClient::getMessage() {
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
 * @brief Get the body of the auth token. 
 * 
 * @param token 
 * @return ps::string The JSON formatted, base64 decoded string of the body contents.
 */
ps::string MQTTClient::get_token_body(ps::string& token) {
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
ps::string MQTTClient::replace_placeholder(const ps::string& client_id, const ps::string& topic) {
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
 * @brief Loads the topics in the ACL from the auth token. Subscribes to required topics.
 * 
 * @param client_id 
 * @param auth_token 
 */
void MQTTClient::decode_token() {
    auto document = DynamicPSRAMJsonDocument(1024);
    ps::string token_body = get_token_body(auth_token);
    ESP_LOGV("MQTT", "Got token body: %s", token_body.c_str());
    auto result = deserializeJson(document, &token_body[0]);
    if (result != DeserializationError::Ok) ESP_LOGE("MQTT", "Failed to deserialize topics: %d.", result);

    broker_url = document["address"].as<ps::string>();
    broker_port = document["port"].as<uint16_t>();

    auto pub = document["acl"]["pub"].as<JsonArray>();
    auto sub = document["acl"]["sub"].as<JsonArray>();
    auto all = document["acl"]["all"].as<JsonArray>();

    for (auto topic : pub) {
        ps::string topic_str = topic.as<ps::string>();
        topic_str = replace_placeholder(client_id, topic_str);
        publish_topics.push_back(topic_str);
        ESP_LOGI("MQTT", "Publish topic: %s", topic_str.c_str());
    }

    for (auto topic : sub) {
        ps::string topic_str = topic.as<ps::string>();
        topic_str = replace_placeholder(client_id, topic_str);
        subscribe_topics.push_back(topic_str);
        ESP_LOGI("MQTT", "Subscribe topic: %s", topic_str.c_str());
    }

    for (auto topic : all) {
        ps::string topic_str = topic.as<ps::string>();
        topic_str = replace_placeholder(client_id, topic_str);
        publish_topics.push_back(topic);
        subscribe_topics.push_back(topic);  
        ESP_LOGI("MQTT", "All topic: %s", topic_str.c_str());
    }

}

/**
 * ==============================================
 * |                                            |
 * |             MessageSerializer              |
 * |                                            |
 * ==============================================
 */

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

/**
 * ==============================================
 * |                                            |
 * |            MessageDeserializer             |
 * |                                            |
 * ==============================================
 */

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
 * ==============================================
 * |                                            |
 * |                 RTOS TASK                  |
 * |                                            |
 * ==============================================
 */

ps::string get_message_payload(CallbackQueueItem& message);
ps::string compress_message(OutgoingMessage& message);
void mqtt_callback(char* topic, uint8_t* payload, uint32_t length);

/**
 * @brief RTOS task for the MQTTClient class which handles the MQTT publish & subscribe, as well as compresssion & decompression.
 * 
 * @param parent 
 */
void communicationTask(void* parent) {
    ESP_LOGI("RTOS", "Communications task started.");

    MQTTClient* client = (MQTTClient*) parent;

    callback_queue = xQueueCreate(5, sizeof(CallbackQueueItem));

    // Fetch broker and topic details from token.
    client -> decode_token();

    // Initialize the MQTT Client
    client -> mqtt_client = ps::make_shared<PubSubClient>();
    client -> mqtt_client -> setClient(client -> conn_client);
    client -> mqtt_client -> setCallback(mqtt_callback);
    client -> mqtt_client -> setBufferSize(16384);
    client -> mqtt_client -> setServer(client -> broker_url.c_str(), client -> broker_port);

    // Connect to the Broker.
    client -> connect();

    ESP_LOGI("MQTT", "Decoding JWT for ACL...");

    // Subscribe to topics as per ACL.
    for (auto topic : client -> subscribe_topics) {
        client -> mqtt_client -> subscribe(topic.c_str());
    }

    // Buffers to store messages from queues.

    IncomingMessage incoming_message;
    OutgoingMessage outgoing_message;
    CallbackQueueItem compressed_message;

    uint64_t elapsed_tm = 0;

    while(1) {
        elapsed_tm = millis();

        // Check that we are still connected to the broker, else block until we are.
        if (!client -> mqtt_client -> connected()) client -> connect();

        // Check for messages to send.
        if (xQueueReceive(client -> outgoing_messages_queue, &outgoing_message, 50 / portTICK_PERIOD_MS) == pdTRUE ) {
            // Compress the message, then publish it to the topic.
            uint64_t start_tm = millis();
            auto message = compress_message(outgoing_message);
            ESP_LOGV("MQTT", "Sending Message: %s", message.c_str());
            if (!client -> mqtt_client -> publish(client -> publish_topics.at(outgoing_message.topic_number).c_str(), message.c_str())) ESP_LOGE("MQTT", "Failed to publish message.");
            ESP_LOGV("MQTT", "Compression took %ums.", millis() - start_tm);
        }

        // Service MQTT client.
        client -> mqtt_client -> loop();

        // Check for received messages.
        if (xQueueReceive(callback_queue, &compressed_message, 50 / portTICK_PERIOD_MS) == pdTRUE ) {
            // Decompress the message, and then copy it to a new buffer, then send to the incoming message queue.
            uint64_t start_tm = millis();
            ps::string payload = get_message_payload(compressed_message);

            // Copy string to RAM and put pointer on queue.
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
 * @brief Get the message payload from the callback queue item. Decompresses it. Returns the data if successful, else an empty string if failed.
 *  Frees the memory after conversion to ps::string.
 * @param message 
 * @return ps::string - Decompressed message payload.
 */
ps::string get_message_payload(CallbackQueueItem& message) {
    ps::string str = message.payload;
    free(message.payload);
    ESP_LOGI("MQTT", "Decompress message: %s", str.c_str());
    return brotli::decompress(str);
}

/**
 * @brief Compresses a message with brotli, then returns the string ready to be published.
 * Frees the memory after conversion to ps::string.
 * @param message 
 * @return ps::string - Empty if failed.
 */
ps::string compress_message(OutgoingMessage& message) {
    ps::string buffer = message.message;
    free(message.message);

    return brotli::compress(buffer);
}

/**
 * @brief Callback from PubSubClient on message received. Copys the message to PSRAM and puts it on the task callback_queue.
 * 
 * @param topic Topic message was received on.
 * @param payload Message Contents.
 * @param length The length of the payload.
 */
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
