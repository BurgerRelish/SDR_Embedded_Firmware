#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <Arduino.h>
#include <string>
#include <vector>

enum ResultDestination{none, app, control, ast};
enum CommandType{direct, timed};

/* Main App Task Message Structure */
enum AppMessageType {ReadingPacket, CommandPacket, ASTNode};

struct AppQueueMessage{
    enum AppMessageType type;
    void * data;
};

struct ReadingPacket{
    double* voltage;
    double* frequency;

    double* active_power;
    double* reactive_power;
    double* apparent_power;
    double* power_factor;

    double* kwh_usage;
};

struct CommandPacket{

};

/* Comms Task Message Structure */
enum CommsMessageType{http, publish};

struct CommsQueueMessage{
    enum CommsMessageType type;
    enum ResultDestination destination;
    void * data;
};

/* AST Task Message Structure */
enum ASTMessageAction{append, replace};

struct ASTQueueMessage{
    enum ASTMessageAction action;
    uint8_t priority;
    std::string* command;
};

enum Action{DIRECT_ON, DIRECT_OFF, SCHEDULE_ON, SCHEDULE_OFF, STATUS_REQUEST, PUBLISH_REQUEST};
struct ASTCommand{
    uint8_t priority;
    enum Action action;
    struct ASTNode* parameters;
    
    struct ASTNode* next_command;
    struct ASTNode* body;
};

enum CompareType{EQ, NE, LT, LTE, GT, GTE, ARR_AND_EQ, ARR_OR_EQ, ARR_NONE_EQ};
struct ASTCompare{
    enum CompareType type;
    struct ASTNode* left;
    struct ASTNode* right;
};

enum LogicalOperator{AND, OR, NOT};
struct ASTLogicalOperator{
    enum LogicalOperator type;
    struct ASTNode* left;
    struct ASTNode* right;
};


enum Identifiers{
    total_active_power, total_reactive_power, total_apparent_power,
    active_power, reactive_power, apparent_power,
    voltage, frequency, power_factor,
    module_id, module_tag_list, module_priority,
    time, switch_time 
};

struct ASTIdentifier{
    enum Identifiers identifier;
};

enum NodeType{command, compare, logical_operator, identifier, literal, parameter};
struct ASTNode{
    enum NodeType type;
    union {
        struct ASTCommand command;
        struct ASTCompare compare;
        struct ASTLogicalOperator logical_operator;
        struct ASTIdentifier identifier;
        double literal;
    } data;
};

/* Control Task Message Structure */
enum ControlMessageType{startRead, ModuleMetaData};

struct ControlQueueMessage{
    enum ControlMessageType type;
    void * data;
};

struct ModuleMetaData{
    std::string * module_id;
    std::vector<std::string> * tags;
    uint8_t * priority;

    uint64_t * switch_time;
    bool * state;

    uint8_t * mux_address;
    uint8_t * mux_io_multiplier;
};

#endif