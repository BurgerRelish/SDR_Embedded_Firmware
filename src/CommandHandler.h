#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ps_stl.h>
#include "Scheduler.h"
#include "MQTTClient.h"
#include "./App/Unit.h"
#include "./App/Module.h"

class CommandHandler{
    private:
    std::shared_ptr<Unit> unit;
    std::shared_ptr<Scheduler> scheduler;

    void loadUnitRules(JsonObject&);
    void loadModuleRules(JsonArray&);

    void handleRuleEngineCommand(JsonObject& object);
    void handleSchedulerCommand(JsonObject& object);
    void handleControlUnitParameters(JsonObject& object);
    void handleTOUPricing(JsonObject& object);

    public:
    bool save_required = false;
    CommandHandler();

    void begin(std::shared_ptr<Unit> unit, std::shared_ptr<Scheduler> scheduler);
    void handle(std::shared_ptr<MessageDeserializer> deserializer);
};