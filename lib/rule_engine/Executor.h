#pragma once

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "../sdr_containers.h"
#include "ps_priority_queue.h"
#include "Language.h"
#include "Semantics.h"

class Executor {
    private:
        ps_priority_queue<Command> command_list;
        ps_stack<Token> current_command;
        
        ps_string command_name;
        ps_vector<Token> parameters;

        xQueueHandle comms_queue;
        xQueueHandle control_queue;
        xQueueHandle rule_engine_queue;

        bool next_command;

        int delay_time;
        bool wait_for_delay;
        uint64_t delay_start_time;

        void load_next_command();
        void ON();
        void OFF();
        void RESTART();
        void DELAY();

        /**
         * @brief Clears any commands in the queue.
        */
        void CLRQUE() {
            while(!current_command.empty()) {
                current_command.pop();
            }

            while(!command_list.empty()) {
                command_list.pop();
            }
        }

        void PUBSTAT();
        void REQUPD();
        void PUBREAD();
        void NOTIFY(ps_string& message);

    public:
        Executor(xQueueHandle _comms_queue, xQueueHandle _control_queue, xQueueHandle _rule_engine_queue):
        comms_queue(_comms_queue),
        control_queue(_control_queue),
        rule_engine_queue(_rule_engine_queue),
        next_command(false)
        {}

        void loopExecutor();
};

#endif