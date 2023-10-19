#include "Scheduler.h"
#include <time.h>

SchedulerItem::SchedulerItem(JsonObject& object){
    module_id = object["module_id"].as<ps::string>();
    state = object["state"].as<bool>();
    timestamp = object["timestamp"].as<uint64_t>();
    period = object["period"].as<uint64_t>();
    count = object["count"].as<int32_t>();
}

SchedulerItem::SchedulerItem(ps::string module_id, bool state, uint64_t timestamp, uint64_t period, int32_t count) {
    this -> module_id = module_id;
    this -> state = state;
    this -> timestamp = timestamp;
    this -> period = period;
    this -> count = count;
}

Scheduler::Scheduler() {
    items = ps::vector<SchedulerItem>();
}

Scheduler::Scheduler(JsonArray& array) {
    items = ps::vector<SchedulerItem>();
    load(array);
}

/**
 * @brief Get the current epoch timestamp.
 * 
 * @return uint64_t 
 */
uint64_t Scheduler::getEpoch() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return 0;
    }

    return (uint64_t) mktime(&timeinfo);
}

/**
 * @brief Load the scheduler from a JSON array.
 * 
 * @param array 
 */
void Scheduler::load(JsonArray& array) {
    for (JsonObject item : array) {
        items.push_back(SchedulerItem(item));
    }
}

/**
 * @brief Save the scheduler to a JSON array.
 * 
 * @param array 
 */
void Scheduler::save(JsonArray& array) {
    for (SchedulerItem item : items) {
        JsonObject object = array.createNestedObject();
        object["module_id"] = item.module_id;
        object["state"] = item.state;
        object["timestamp"] = item.timestamp;
        object["period"] = item.period;
        object["count"] = item.count;
    }
}

/**
 * @brief Check if any items are due to be executed. If so, return them.
 * 
 * @return ps::vector<SchedulerItem> The items to be executed.
 */
ps::vector<SchedulerItem> Scheduler::check() {
    ps::vector<SchedulerItem> ret = ps::vector<SchedulerItem>();
    uint64_t now = getEpoch();

    if (now == 0) return ret; // Failed to get the time, return the empty array.

    for (size_t i = 0; i < items.size(); i++) { // Iterate over the items.
        if (items[i].timestamp <= now) { // If the item is due to be executed.
            ret.push_back(items[i]); // Add the item to the return array.
            if (items[i].count > 0) items[i].count --; // Decrement the count if it is not infinite.

            if (items[i].count == 0) {
                items.erase(items.begin() + i); // Remove the item if it has been executed the required number of times.
                i--;
                continue;
            }

            items[i].timestamp = now + items[i].period; // Increment the timestamp by the period.
        }
    }

    return ret;
}

/**
 * @brief Clear all items from the scheduler.
*/
void Scheduler::clear() {
    items.clear();
}

/**
 * @brief Add an item to the scheduler.
 * 
 * @param item 
 */
void Scheduler::add(SchedulerItem item) {
    items.push_back(item);
}

/**
 * @brief Clear all items for a specific module.
 * 
 * @param module_id 
 */
void Scheduler::clearModule(ps::string module_id) {
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].module_id == module_id) {
            items.erase(items.begin() + i);
            i--;
        }
    }
}

QueueHandle_t queue;
SemaphoreHandle_t scheduler_semaphore;

void schedulerTask(void* parent) {
    queue = xQueueCreate(10, sizeof(ps::vector<SchedulerItem>));
    Scheduler* scheduler = (Scheduler*) parent;
    ps::vector<SchedulerItem> items;

    while (true) {
        
        items = scheduler -> check();
        
        if (items.size() > 0) {
            auto outgoing = new ps::vector<SchedulerItem>(items);
            xQueueSend(queue, outgoing, portMAX_DELAY);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}