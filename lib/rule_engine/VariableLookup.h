#pragma once

#ifndef VARIABLE_LOOKUP_H
#define VARIABLE_LOOKUP_H
#include <Arduino.h>
#include <memory>
#include "../sdr_containers/SDRModule.h"
#include "../sdr_containers/SDRUnit.h"
#include "Language.h"
#include "../ps_stl/ps_stl.h"



class VariableLookup {
    private:    
    void retrieveVar(const ps::string& var, double& val);
    void retrieveVar(const ps::string& var, bool& val);
    void retrieveVar(const ps::string& var, int& val);
    void retrieveVar(const ps::string& var, uint64_t& val);
    void retrieveVar(const ps::string& var, ps::string& val);
    double toDouble(ps::string& str);
    
    public:
    std::shared_ptr<SDRUnit> unit;
    std::shared_ptr<Module> module;

    VariableLookup(std::shared_ptr<SDRUnit> _unit, std::shared_ptr<Module> _module) : unit(_unit), module(_module) {}
    
    const VarType& getVarType(const Token& search_token);

    const double getDouble(Token& token);
    const bool getBool(Token& token);
    const int getInt(Token& token);
    const uint64_t getUint64(Token& token);
    const ps::string getString(Token& token);
};

#endif