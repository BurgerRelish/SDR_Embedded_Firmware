#pragma once

#ifndef ARRAY_SEPARATOR_H
#define ARRAY_SEPARATOR_H

#include "ps_vector.h"
#include "ps_string.h"
#include "Language.h"
#include "Lexer.h"

class ArraySeparator {
    private:
    ps_queue<Token> tokenize_array(const ps_string& string);

    public:
    /**
     * @brief Separate an array token into its lexemes.
     * @returns A queue of strings representing the array elements.
    */
    ps_vector<ps_string> separate(const Token& token);
};

#endif // ARRAY_SEPARATOR_H