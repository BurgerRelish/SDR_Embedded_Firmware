#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "json_allocator.h"
#include <ps_stl.h>

class SchedulerItem{
    public:
    ps::string module_id; // The ID of the module to set the state of.
    bool state; // State to set the relay to.
    uint64_t timestamp; // Timestamp after which the state should be set.
    uint64_t period; // Period at which the item should repeat.
    int32_t count; // Number of times the item should repeat. -1 for infinite, 0 or 1 for once.

    SchedulerItem(JsonObject& object);
    SchedulerItem(ps::string module_id, bool state, uint64_t timestamp, uint64_t period, int32_t count);
};

class Scheduler {
    private:
    ps::vector<SchedulerItem> items;
    uint64_t getEpoch();

    public:
    Scheduler();
    Scheduler(JsonArray& array);

    void load(JsonArray& array);
    void save(JsonArray& array);

    void add(SchedulerItem item);
    void clearModule(ps::string module_id);
    void clear();

    ps::vector<SchedulerItem> check();
};

