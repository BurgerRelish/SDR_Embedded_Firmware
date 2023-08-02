#pragma once

#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include "Evaluator.h"
#include "Executor.h"

class RuleEngine {
    private:
    public:
    RuleEngine(SDRUnit* global_vars, Module* module_vars, ps_queue<Rule>* rule_input, xQueueHandle comms_queue, xQueueHandle control_queue, xQueueHandle rule_engine_queue) {

    }
};

#endif