#pragma once

#ifndef ARRAY_SEPARATOR_H
#define ARRAY_SEPARATOR_H

#include <ps_stl.h>

#include "Language.h"
#include "Lexer.h"

class ArraySeparator {
    private:
    ps::queue<Token> tokenize_array(const ps::string& string);

    public:
    /**
     * @brief Separate an array token into its lexemes.
     * @returns A queue of strings representing the array elements.
    */
    ps::vector<ps::string> separate(const Token& token);
};

#endif // ARRAY_SEPARATOR_H