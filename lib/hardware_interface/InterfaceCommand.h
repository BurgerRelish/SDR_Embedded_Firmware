#pragma once

#ifndef INTERFACE_COMMAND_H
#define INTERFACE_COMMAND_H

#include <ps_stl.h>

class InterfaceCommand {
    uint16_t address;
    uint16_t command;

    InterfaceCommand(){}
    InterfaceCommand(uint16_t _address, uint16_t _command) : address(_address), command(_command) {}

    operator ps::string() {
        ps::ostringstream ret;
        ret.width(3);
        ret << "{" << address << "|" << command << "}";
        return ret.str();
    }
};

#endif