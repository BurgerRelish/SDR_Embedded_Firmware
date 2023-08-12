#include "CommandSeparator.h"
#include "esp_log.h"


namespace re {

ps::vector<std::pair<ps::string, ps::vector<Token>>> CommandSeparator::separate(ps::string& command) {
    Lexer lexer;
    auto tokens = lexer.tokenize(command);
    ps::vector<std::pair<ps::string, ps::vector<Token>>> ret;
    ps::queue<Token> current_command;

    while (!tokens.empty()) {
        /* Get the command */
        while (!tokens.empty() && tokens.front().lexeme != COMMAND_SEPARATOR) {
            current_command.push(tokens.front());
            tokens.pop();
        }

        uint8_t state = 0;
        ps::string name;
        ps::vector<Token> args;

        while (!current_command.empty()) {
            auto& token = current_command.front();

            /* Separate command into its name and argument tokens. Drop all brackets, commas, stop when the COMMAND_SEPARATOR has been reached. */
            switch (state) {
                case 0: // Get name
                    if (token.type == TokenType::IDENTIFIER) {
                        name = token.lexeme;
                        state = 1;
                    }
                    break;

                case 1: // Wait for opening bracket.
                    if (token.lexeme == "(") {
                        state = 2;
                    }
                    break;

                case 2:
                    if (token.lexeme == ")") {
                        state = 3;
                    } else if (token.lexeme != ARGUMENT_SEPARATOR) { // ,
                        args.push_back(token);
                    }
                    break;
                case 3:
                    if (token.lexeme != COMMAND_SEPARATOR) ESP_LOGE("Syntax", "Invalid command syntax");
                break;
            }

            current_command.pop();
        }

        ret.push_back(std::make_pair(name, args));
    }

    return ret;
}
}