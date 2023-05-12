#include <MQTTClient.h>

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


MQTTClient::MQTTClient(TinyGsmClient * client)
{
    gsm_client = client;

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

        mqtt_details.push_back(
            std::string( 
                mqtt_store -> getString(
                    current_topic.str().c_str()
                ).c_str()
            )
        );
    }

    delete mqtt_store;

    return;
}


void MQTTClient::begin()
{


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
    return mqtt_details.at(CLIENT_DEVICE_NAME);
}

/**
 * The function returns the username of the MQTT client.
 * 
 * @return A string representing the username of the MQTT client. The value is retrieved from the
 * `mqtt_details` map using the `CLIENT_USERNAME` key.
 */
std::string MQTTClient::getUsername()
{
    return mqtt_details.at(CLIENT_USERNAME);
}

/**
 * This function returns the password of the MQTT client.
 * 
 * @return A string representing the password of the MQTT client, which is retrieved from the
 * `mqtt_details` map using the `CLIENT_PASSWORD` key.
 */
std::string MQTTClient::getPassword()
{
    return mqtt_details.at(CLIENT_PASSWORD);
}

/**
 * This function returns the egress topic of an MQTT client.
 * 
 * @return A string representing the egress topic of the MQTT client.
 */
std::string MQTTClient::getEgressTopic()
{
    return mqtt_details.at(CLIENT_EGRESS_TOPIC);
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

    for (size_t i = CLIENT_EGRESS_TOPIC + 1; i < mqtt_details.size(); i++) ret.push_back(mqtt_details.at(i));

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
        mqtt_details.at(CLIENT_DEVICE_NAME) = new_value;
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
        mqtt_details.at(CLIENT_USERNAME) = new_value;
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
        mqtt_details.at(CLIENT_PASSWORD) = new_value;
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
        mqtt_details.at(CLIENT_EGRESS_TOPIC) = new_value;
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
        mqtt_details.resize(CLIENT_EGRESS_TOPIC + 1);

        for (size_t i = 0; i < new_values.size(); i++) mqtt_details.push_back(new_values.at(i));
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

    mqtt_store -> putUInt(MQTT_DETAILS_LOOKUP, mqtt_details.size());

    for (size_t i = 0; i <= mqtt_details.size(); i++)
    {
        current_topic.clear();

        current_topic << MQTT_DETAILS_PREFIX << i;

        mqtt_store -> putString(current_topic.str().c_str(), String(mqtt_details.at(i).c_str()));
    }

    delete mqtt_store;

    return;
}

void MQTTClient::connectMQTT()
{
    for (int i = 0; i < mqtt_details.size(); i++) if (mqtt_details.at(i).empty()) throw SDRException("Not all MQTT Details are configured.");

    if(mqtt_client == nullptr) mqtt_client = new PubSubClient(mqtt_details.at(BROKER_URL).c_str(), broker_port, callbackMQTT, *(gsm_client) ); // Init PubSubClient
    if(! mqtt_client -> connected()) mqtt_client -> connect(mqtt_details.at(CLIENT_DEVICE_NAME).c_str(), mqtt_details.at(CLIENT_USERNAME).c_str(), mqtt_details.at(CLIENT_USERNAME).c_str());
    
    return;
}

void callbackMQTT(char* topic, byte* payload, unsigned int len)
{

    return;
}


void taskMQTT(void * parent_class)
{
    auto parent = (MQTTClient *) parent_class;

    parent -> connectMQTT();

    while(true)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
    return; 
}

