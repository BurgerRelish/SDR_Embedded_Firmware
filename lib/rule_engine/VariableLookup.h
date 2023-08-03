#pragma once

#ifndef VARIABLE_LOOKUP_H
#define VARIABLE_LOOKUP_H
#include <Arduino.h>
#include "../sdr_containers/SDRModule.h"
#include "../sdr_containers/SDRUnit.h"
#include "Language.h"


class VariableLookup {
    private:
    SDRUnit* unit = nullptr;
    Module* module = nullptr;
    
    void retrieveVar(const ps_string& var, double& val);
    void retrieveVar(const ps_string& var, bool& val);
    void retrieveVar(const ps_string& var, int& val);
    void retrieveVar(const ps_string& var, uint64_t& val);
    void retrieveVar(const ps_string& var, ps_string& val);
    double toDouble(ps_string& str);
    
    public:
    VariableLookup(SDRUnit* _unit, Module* _module) : unit(_unit), module(_module) {}
    VariableLookup(SDRUnit* _unit) : unit(_unit), module(nullptr) {}

    const VarType& getVarType(const Token& search_token);

    const double getDouble(Token& token);
    const bool getBool(Token& token);
    const int getInt(Token& token);
    const uint64_t getUint64(Token& token);
    const ps_string getString(Token& token);
};

#endif