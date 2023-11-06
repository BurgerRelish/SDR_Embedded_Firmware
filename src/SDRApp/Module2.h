#pragma once

#ifndef SDR_MODULE_H
#define SDR_MODULE_H

#include <ArduinoJson.h>
#include <ps_stl.h>
#include "../rule_engine/RuleEngineBase.h"
#include "sdr_semantics.h"
#include "Reading.h"
#include "StatusChange.h"

namespace sdr {

class Module: public re::RuleEngineBase {
    private:
    ps::string module_id;

    ps::deque<Reading> readings;
    ps::deque<StatusChange> status_changes;

    public:
    Module(std::shared_ptr<re::FunctionStorage>& function_storage):
        re::RuleEngineBase(MODULE_TAG_LIST, function_storage)
    {
        
    }

    void serialize(JsonArray& array);


};

}

#endif