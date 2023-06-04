#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <Arduino.h>
#include <string>
#include <vector>

enum Command{DIRECT_ON, DIRECT_OFF, SCHEDULE_ON, SCHEDULE_OFF, STATUS_REQUEST, PUBLISH_REQUEST, EXEMPT};

/* Main App Task Message Structure */
enum AppMessageType{ReadingPacket, CommandPacket, ASTNode, ReInitAST, ReInitControl, ReInitComms};
struct AppQueueMessage{
    enum AppMessageType type;
    void * data;
};

struct ReadingPacket{
    struct ModuleMetaData* module;

    double voltage;
    double frequency;

    double active_power;
    double reactive_power;
    double apparent_power;
    double power_factor;

    double kwh_usage;
    uint64_t timestamp;
};

struct CommandPacket{
    enum Command action;
    uint8_t* priority;
    struct ASTNode* parameters;
};

struct ReadingStorePacket{
    struct ReadingStorePacket* next_reading;

    double voltage;
    double frequency;

    double active_power;
    double reactive_power;
    double apparent_power;
    double power_factor;

    double kwh_usage;
    uint64_t timestamp;
};

/* Comms Task Message Structure */
enum CommsMessageType{HTTPMessage, MQTTMessage, CLIENT};
struct CommsQueueMessage{
    enum CommsMessageType type;
    void * data;
};

enum HTTPMethod{GET, PUT, POST, DELETE};
struct CommsHTTPMessage{
    std::string* url;
    int* port;

    enum HTTPMethod method;
    std::string* header;
    std::string* body;
};

struct CommsMQTTMessage{
    std::string* topic;
    std::string* message;
};

/* AST Task Message Structure */
enum MessageAction{APPEND, REPLACE, CLEAR};
struct ASTQueueMessage{
    enum MessageAction action;
    uint8_t priority;
    std::string* data;
};


struct ASTCommand{
    uint8_t priority;
    enum Command action;
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

enum ArithmeticOperator{ADD, SUBTRACT, MULTIPLY, DIVIDE};
struct ASTArithmetic{
    enum ArithmeticOperator arithmetic_operator;
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

struct ASTParameter{
    struct ASTNode* next;
    struct ASTNode* value;
};

enum NodeType{COMMAND, COMPARE, LOGICAL, IDENTIFIER, LITERAL, PARAMETER, ARITHMETIC};
struct ASTNode{
    enum NodeType type;
    union {
        struct ASTCommand command;
        struct ASTCompare compare;
        struct ASTLogicalOperator logical_operator;
        struct ASTIdentifier identifier;
        struct ASTParameter parameter;
        struct ASTArithmetic arithmetic;
        double literal;
    } data;
};

/* Control Task Message Structure */
enum ControlMessageType{startRead, setState};
struct ControlQueueMessage{
    enum ControlMessageType type;
    void * data;
};

struct setState{
    struct ModuleMetaData* module;
    bool state;
};

struct ModuleMetaData{
    std::string * module_id;
    std::vector<std::string> * tags;
    uint8_t * priority;

    uint64_t switch_time;
    bool state;

    uint8_t mux_address;
    uint8_t mux_io_offset;

    struct ModuleMetaData* next_module;
    struct ReadingStorePacket* storage;
};

#endif