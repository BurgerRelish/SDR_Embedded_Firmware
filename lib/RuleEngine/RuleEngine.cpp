#include "RuleEngine.h"
#include <time.h>
#include "Semantics.h"
#include <functional>

namespace re {

std::function<bool(ps::vector<ps::vector<Token>>&, re::VariableStorage*)> set_variable = [](ps::vector<ps::vector<Token>>& args, re::VariableStorage* vars){
    VariableType type = vars -> get_type(args.at(0).at(0).lexeme);
    ESP_LOGI("setVar", "Setting variable: %s", args.at(1).at(0).lexeme.c_str());

    switch (type) {
        case re::VAR_DOUBLE: {
            re::Expression expr(args.at(2), vars);
            vars -> set_var(args.at(0).at(0).lexeme, expr.result());
            break;
        }

        case re::VAR_BOOL: {
            re::Expression expr(args.at(2), vars);
            vars -> set_var(args.at(0).at(0).lexeme, expr.evaluate());
            break;
        }

        case re::VAR_INT: {
            re::Expression expr(args.at(2), vars);
            vars -> set_var(args.at(0).at(0).lexeme, (int)expr.result());
            break;
        }

        case re::VAR_UINT64_T: {
            re::Expression expr(args.at(2), vars);
            vars -> set_var(args.at(0).at(0).lexeme, (uint64_t)expr.result());
            break;
        }

        case re::VAR_STRING:
            vars -> set_var(args.at(0).at(0).lexeme, vars -> get_var<ps::string>(args.at(2).at(0).lexeme));
            break;
    }
    
    return true;
};



}