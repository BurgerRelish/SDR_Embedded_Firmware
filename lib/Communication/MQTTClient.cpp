#include <MQTTClient.h>

constexpr const char* TAG = "MQTTClient";

constexpr const char* MQTT_DETAILS_LOOKUP = "num_values";
constexpr const char* MQTT_DETAILS_PREFIX = "details";

/**
 * @note The MQTT details are stored in the following order:
 * 0 - Broker URL
 * 1 - Device Name
 * 2 - Username
 * 3 - Password
 * 4 - Outgress Topic
 * 5+ - Subscribe Topics
*/

constexpr size_t BROKER_URL = 0;
constexpr const char* BROKER_PORT = "broker_port";
constexpr size_t CLIENT_DEVICE_NAME = 1;
constexpr size_t CLIENT_USERNAME = 2;
constexpr size_t CLIENT_PASSWORD = 3;
constexpr size_t CLIENT_EGRESS_TOPIC = 4;

std::vector<std::string> * MQTTClient::mqtt_rx_buffer = nullptr;
std::vector<std::string> * MQTTClient::mqtt_tx_buffer = nullptr;
StaticSemaphore_t MQTTClient::MQTT_rx_queue_buffer;
SemaphoreHandle_t MQTTClient::MQTT_rx_queue_semaphore = nullptr;
StaticSemaphore_t MQTTClient::MQTT_tx_queue_buffer;
SemaphoreHandle_t MQTTClient::MQTT_tx_queue_semaphore = nullptr;
TaskHandle_t MQTTClient::handleMQTT = nullptr;

std::vector<std::string> * MQTTClient::mqtt_details = nullptr;
Client * MQTTClient::network_client = nullptr;
PubSubClient * MQTTClient::mqtt_client = nullptr;

MQTTClient::MQTTClient()
{
    if(network_client == nullptr) throw SDRException("No networking client present.");

    recallMQTTDetails();
}

MQTTClient::MQTTClient(Client * client)
{
    network_client = client;

    recallMQTTDetails();

    return;
}

MQTTClient::~MQTTClient()
{
}


void MQTTClient::begin()
{
    if(mqtt_details->size() < 5) throw SDRException("Not all MQTT Parameters are present.");
    if(network_client == nullptr) throw SDRException("No Networking Client is present.");

    initBuffers();

    if(MQTT_rx_queue_semaphore == nullptr)
    {
        MQTT_rx_queue_semaphore = xSemaphoreCreateBinaryStatic(&MQTT_rx_queue_buffer); // Create rx buffer semaphore
        xSemaphoreGive(MQTT_rx_queue_semaphore);
    }

    if (MQTT_tx_queue_semaphore == nullptr)
    {
        MQTT_tx_queue_semaphore = xSemaphoreCreateBinaryStatic(&MQTT_tx_queue_buffer); // Create tx buffer semaphore
        xSemaphoreGive(MQTT_tx_queue_semaphore);
    }

    if (handleMQTT == nullptr)
    {
        xTaskCreatePinnedToCore( // Create the MQTT core task on core 1.
            taskMQTT,
            "MQTTClient RTOS Task",
            32000,
            this,
            tskIDLE_PRIORITY,
            &handleMQTT,
            1
        );
    }

    return;
}

/**
 * This function returns the device name stored in the MQTT client details.
 * 
 * @return A string representing the device name of the MQTT client. The device name is retrieved from
 * the `mqtt_details` map using the `CLIENT_DEVICE_NAME` key.
 */
std::string MQTTClient::getDeviceName()
{
    return mqtt_details -> at(CLIENT_DEVICE_NAME);
}

/**
 * The function returns the username of the MQTT client.
 * 
 * @return A string representing the username of the MQTT client. The value is retrieved from the
 * `mqtt_details` map using the `CLIENT_USERNAME` key.
 */
std::string MQTTClient::getUsername()
{
    return mqtt_details -> at(CLIENT_USERNAME);
}

/**
 * This function returns the password of the MQTT client.
 * 
 * @return A string representing the password of the MQTT client, which is retrieved from the
 * `mqtt_details` map using the `CLIENT_PASSWORD` key.
 */
std::string MQTTClient::getPassword()
{
    return mqtt_details -> at(CLIENT_PASSWORD);
}

/**
 * This function returns the egress topic of an MQTT client.
 * 
 * @return A string representing the egress topic of the MQTT client.
 */
std::string MQTTClient::getEgressTopic()
{
    return mqtt_details -> at(CLIENT_EGRESS_TOPIC);
}

/**
 * This function returns a vector of MQTT ingress topics by iterating through the mqtt_details vector
 * and pushing back each topic after CLIENT_EGRESS_TOPIC.
 * 
 * @return A vector of strings containing the ingress topics of an MQTT client. The function iterates
 * through the `mqtt_details` vector starting from the index after `CLIENT_EGRESS_TOPIC` and pushes
 * each element into the `ret` vector. Finally, the `ret` vector is returned.
 */
std::vector<std::string> MQTTClient::getIngressTopics()
{
    std::vector<std::string> ret;

    for (size_t i = CLIENT_EGRESS_TOPIC + 1; i < mqtt_details -> size(); i++) ret.push_back(mqtt_details -> at(i));

    return ret;
}

/**
 * This function sets the device name for an MQTT client and writes the updated details.
 * 
 * @param new_value new_value is a string parameter that represents the new value to be set for the
 * MQTT device name. This function updates the MQTT device name in the mqtt_details map with the new
 * value provided.
 * 
 * @return void.
 */
void MQTTClient::setDeviceName(std::string new_value)
{
    try
    {
        mqtt_details -> at(CLIENT_DEVICE_NAME) = new_value;
    }   
    catch (std::out_of_range &e)
    { 
        log_e("Failed to set MQTT Device Name. Exception: %s\n", e.what());
    }

    writeMQTTDetails();

    return;
}

/**
 * This function sets the MQTT username and writes the updated details.
 * 
 * @param new_value A string representing the new value to be set for the MQTT client's username.
 * 
 * @return void.
 */
void MQTTClient::setUsername(std::string new_value)
{
    try
    {
        mqtt_details -> at(CLIENT_USERNAME) = new_value;
    }   
    catch (std::out_of_range &e)
    { 
        log_e("Failed to set MQTT Username. Exception: %s\n", e.what());
    }

    writeMQTTDetails();

    return;
}

/**
 * This function sets the password for an MQTT client and writes the updated details.
 * 
 * @param new_value The new password value that is being set for the MQTT client.
 * 
 * @return void
 */
void MQTTClient::setPassword(std::string new_value)
{
    try
    {
        mqtt_details -> at(CLIENT_PASSWORD) = new_value;
    }   
    catch (std::out_of_range &e)
    { 
        log_e("Failed to set MQTT Password. Exception: %s\n", e.what());
    }

    writeMQTTDetails();

    return;
}

/**
 * This function sets the MQTT egress topic and logs an error if it fails.
 * 
 * @param new_value A string representing the new value to be set for the MQTT egress topic.
 * 
 * @return nothing (void).
 */
void MQTTClient::setEgressTopic(std::string new_value)
{
    try
    {
        mqtt_details -> at(CLIENT_EGRESS_TOPIC) = new_value;
    }   
    catch (std::out_of_range &e)
    { 
        log_e("Failed to set MQTT Egress Topic. Exception: %s\n", e.what());
    }

    writeMQTTDetails();

    return;
}

/**
 * This function sets the ingress topics for an MQTT client.
 * 
 * @param new_values A vector of strings containing the new values for the MQTT client's ingress
 * topics.
 * 
 * @return void.
 */
void MQTTClient::setIngressTopics(std::vector<std::string> new_values)
{
    try
    {
        mqtt_details -> resize(CLIENT_EGRESS_TOPIC + 1);

        for (size_t i = 0; i < new_values.size(); i++) mqtt_details -> push_back(new_values.at(i));
    }
    catch(std::out_of_range &e)
    {
        log_e("Failed to set Ingress Topics. Exception: %s\n", e.what());
    }

    writeMQTTDetails();
    
    return;
}

/**
 * This function writes MQTT details to non-volatile storage.
 * 
 * @return void.
 */
void MQTTClient::writeMQTTDetails()
{
    std::ostringstream current_topic;

    auto mqtt_store = new Preferences;
    mqtt_store -> begin(MQTT_STORAGE_NVS_PATH);

    mqtt_store -> putUInt(MQTT_DETAILS_LOOKUP, mqtt_details -> size());

    for (size_t i = 0; i <= mqtt_details -> size(); i++)
    {
        current_topic.clear();

        current_topic << MQTT_DETAILS_PREFIX << i;

        mqtt_store -> putString(current_topic.str().c_str(), String(mqtt_details -> at(i).c_str()));
    }

    delete mqtt_store;

    return;
}

void MQTTClient::connectMQTT()
{
    for (int i = 0; i < mqtt_details -> size(); i++) if (mqtt_details -> at(i).empty()) throw SDRException("Not all MQTT Details are configured.");

    if(mqtt_client == nullptr) mqtt_client = new PubSubClient(mqtt_details -> at(BROKER_URL).c_str(), broker_port, callbackMQTT, *(network_client) ); // Init PubSubClient
    if(! mqtt_client -> connected()) mqtt_client -> connect(mqtt_details -> at(CLIENT_DEVICE_NAME).c_str(), mqtt_details -> at(CLIENT_USERNAME).c_str(), mqtt_details -> at(CLIENT_USERNAME).c_str());
    
    return;
}

/**
 * The function receives an MQTT message, converts the payload to a string, and pushes it to the rx buffer
 * while taking and giving the rx buffer semaphore.
 * 
 * @param topic The topic of the MQTT message received.
 * @param payload A pointer to the message payload received over MQTT.
 * @param len len is an unsigned integer that represents the length of the payload received in the MQTT
 * message.
 * 
 * @return void.
 */
void callbackMQTT(char* topic, byte* payload, unsigned int len)
{
    ESP_LOGI(TAG, "Recieved MQTT Message");

    auto xHigherPriorityTaskWoken = pdFALSE; // Take RX Buffer Semaphore.
    xSemaphoreTakeFromISR(MQTTClient::MQTT_rx_queue_semaphore, &xHigherPriorityTaskWoken);

    std::string message(static_cast<char*>(static_cast<void*>(payload)), len);  // Convert payload to std::string
    MQTTClient::mqtt_rx_buffer->push_back(message);  // Push message to mqtt_rx_buffer

    ESP_LOGD(TAG, "Recieved Message \"%s\" on Topic \"%s\"\n", message.c_str(), topic);

    xSemaphoreGiveFromISR(MQTTClient::MQTT_rx_queue_semaphore, &xHigherPriorityTaskWoken); // Give the semaphore back.

    return;
}


void MQTTClient::initBuffers()
{
    if (mqtt_rx_buffer == nullptr) mqtt_rx_buffer = new std::vector<std::string>;
    if (mqtt_tx_buffer == nullptr) mqtt_tx_buffer = new std::vector<std::string>;
}

void MQTTClient::recallMQTTDetails()
{
    if (mqtt_details == nullptr) mqtt_details = new std::vector<std::string>; // Only recall details from NVS on first run.
    else return;

    auto mqtt_store = new Preferences;
    mqtt_store -> begin(MQTT_STORAGE_NVS_PATH);

    size_t number_of_topics = mqtt_store -> getUInt(MQTT_DETAILS_LOOKUP); // Get the number of topics in storage.

    broker_port = mqtt_store -> getUInt(BROKER_PORT);

    if (number_of_topics < CLIENT_EGRESS_TOPIC + 1) number_of_topics = CLIENT_EGRESS_TOPIC + 1;

    std::ostringstream current_topic;

    for(int i = 0; i < number_of_topics; i++) // Load MQTT Details from NVS.
    {
        current_topic.clear();
        current_topic << MQTT_DETAILS_PREFIX << i;

        mqtt_details -> push_back(
            std::string( 
                mqtt_store -> getString(
                    current_topic.str().c_str()
                ).c_str()
            )
        );
    }

    delete mqtt_store;
}

void taskMQTT(void * parent_class)
{
    ESP_LOGI(TAG, "RTOS Task Started.");

    auto parent = (MQTTClient *) parent_class;

    try
    {
        parent -> connectMQTT(); // Connect MQTT Broker
    }
    catch(const SDRException &e)
    {
        ESP_LOGE(TAG, "Failed to start MQTT Client, RTOS task aborting! Exception: %s\n", e.what());
        vTaskDelete(nullptr);

        return;
    }

    auto connection_time = xTaskGetTickCount();

    while(! MQTTClient::mqtt_client -> connected()) // Wait while we connect to MQTT Broker.
    {
        vTaskDelay(50 / portTICK_PERIOD_MS);

        if (xTaskGetTickCount() - connection_time > (30000 / portTICK_PERIOD_MS))
        {
            ESP_LOGE(TAG, "Timeout connecting to MQTT broker, RTOS task aborting!\n");
            vTaskDelete(nullptr);
            return;
        }
    }

    ESP_LOGI(TAG, "RTOS task setup completed!");

    while(true)
    {

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(nullptr);
    return; 
}

