#pragma once

#ifndef VARIABLE_LOOKUP_H
#define VARIABLE_LOOKUP_H
#include <Arduino.h>
#include <memory>
#include "../sdr_containers/SDRModule.h"
#include "../sdr_containers/SDRUnit.h"
#include "Language.h"


class VariableLookup {
    private:    
    void retrieveVar(const ps_string& var, double& val);
    void retrieveVar(const ps_string& var, bool& val);
    void retrieveVar(const ps_string& var, int& val);
    void retrieveVar(const ps_string& var, uint64_t& val);
    void retrieveVar(const ps_string& var, ps_string& val);
    double toDouble(ps_string& str);
    
    public:
    std::shared_ptr<SDRUnit> unit;
    std::shared_ptr<Module> module;

    VariableLookup(std::shared_ptr<SDRUnit> _unit, std::shared_ptr<Module> _module) : unit(_unit), module(_module) {}
    
    const VarType& getVarType(const Token& search_token);

    const double getDouble(Token& token);
    const bool getBool(Token& token);
    const int getInt(Token& token);
    const uint64_t getUint64(Token& token);
    const ps_string getString(Token& token);
};

#endif