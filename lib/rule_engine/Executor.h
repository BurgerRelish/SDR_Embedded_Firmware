// #pragma once

// #ifndef EXECUTOR_H
// #define EXECUTOR_H

// #include <memory>
// #include <functional>
// #include <initializer_list>
// #include <tuple>


// #include "../sdr_containers/SDRUnit.h"
// #include "../sdr_containers/SDRModule.h"
// #include "../ps_stl/ps_stl.h"

// #include "Language.h"
// #include "Semantics.h"
// #include "Evaluator.h"
// #include "VariableStorage.h"
// #include "ShuntingYard.h"

// class Executor {
//     private:
//         ps::priority_queue<Command> command_list;
//         OriginType cmd_origin_type;
//         std::shared_ptr<Evaluator> cmd_origin;
//         ps::queue<Token> current_command;
        
//         ps::string command_name;
//         ps::vector<Token> parameters;

//         xQueueHandle comms_queue;
//         xQueueHandle control_queue;
//         xQueueHandle rule_engine_queue;

//         bool next_command;

//         int delay_time;
//         bool wait_for_delay;
//         uint64_t delay_start_time;

//         void load_next_command();
//         void ON();
//         void OFF();
//         void RESTART();
//         void DELAY();

//         /**
//          * @brief Clears any commands in the queue.
//         */
//         void CLRQUE() {
//             while(!current_command.empty()) {
//                 current_command.pop();
//             }

//             while(!command_list.empty()) {
//                 command_list.pop();
//             }
//         }

//         void PUBSTAT(int window);
//         void REQUPD(int window);
//         void PUBREAD(int window);
//         void COMMS_NOTIFY(ps::string& message);

//     public:
//         Executor(xQueueHandle _comms_queue, xQueueHandle _control_queue, xQueueHandle _rule_engine_queue):
//         comms_queue(_comms_queue),
//         control_queue(_control_queue),
//         rule_engine_queue(_rule_engine_queue),
//         next_command(false)
//         {}

//         void addCommand(Command cmd) {
//             command_list.push(cmd);
//         };

//         void addCommands(ps::queue<Command> commands) {
//             while(!commands.empty()) {
//                 command_list.push(commands.front());
//                 commands.pop();
//             }
//         }

//         void loopExecutor();
// };

// namespace re {

// class ExecutorDelay {
//     private:
//     uint64_t delay_end;

//     public:
//     bool check() {
//         if (esp_timer_get_time() >= delay_end) return true;
//         return false;
//     }

//     void delay(uint32_t delay_seconds) {
//         delay_end = esp_timer_get_time() + 1000000 * delay_seconds;
//     }
// };

// class BaseFunction {
//     private:        
//         ps::unordered_map<ps::string, std::function<bool(ExecutorDelay&, SDR::VariableStorage&, ps::vector<Token>&)>> function_map;

//     public:
//         void add(const ps::string& identifier, std::function<bool(ExecutorDelay&, SDR::VariableStorage&, ps::vector<Token>&)> function) {
//             function_map[identifier] = function;
//         }

//         bool execute(const ps::string& identifier, ExecutorDelay& delay,  SDR::VariableStorage& vars, ps::vector<Token>& args) {
//             auto lambda = function_map.find(identifier);

//             if (lambda == function_map.end()) {
//                 ESP_LOGD("Map Search", "Invalid Function Identifier.");
//                 return false;
//             }

//             try {
//                 return lambda -> second(delay, vars, args);
//             } catch (...) {
//                 ESP_LOGD("Exception", "Lambda execution.");
//                 return false;
//             }

//             return true;
//         }

// };

// class Command: private Lexer {
//     private:
//     ps::vector<Token> tokens;
//     ps::vector<Token> current_command;
//     BaseFunction& function;
//     SDR::VariableStorage& variables;
//     ExecutorDelay delay_class;
//     int priority;

//     void load_next_function() {
//         ps::queue<Token> temp;
//         current_command.clear();

//         for (auto& token : tokens) {
//             if (token.lexeme == COMMAND_SEPARATOR) break;
//             temp.push(token);
//         }
        

//         for (auto& token : current_command) {
//             if (token.type == IDENTIFIER || token.type == NUMERIC_LITERAL || token.type == STRING_LITERAL || token.type == ARRAY)
//             current_command.push_back(token);
//             if (token.lexeme == COMMAND_SEPARATOR) break;
//         }
//     }

//     public:
//     Command(const ps::string& command_str, int priority) {
//         auto token_queue = Lexer::tokenize(command_str);
        
//         while (!token_queue.empty()) {
//             tokens.push_back(token_queue.front());
//             token_queue.pop();
//         }
//     }

//     void loop() {
//         if (!delay_class.check()) return; // Check if delay is active.
//         if(function.execute()) 
//     }
// };



// template <typename FnArgType>
// class Evaluator : public BaseFunction<FnArgType>, public Expression, private Lexer, private ShuntingYard {
//     private:
//     VariableStorage& variables;
//     FunctionClass fn_queue;

//     public:
//     Evaluator(const ps::string& expression, BaseFunction<FnArgType> functions, VariableStorage& (vars) : variables(vars), BaseFunction<ArgType>(functions), Expression(expression) {

//     }

//     void loop () {
//         if(Expression::evaluate()) {
//             BaseFunction::execute();
//         }
//     }


// }

// }


// #endif