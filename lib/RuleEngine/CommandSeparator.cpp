#include "CommandSeparator.h"
#include "esp32-hal-log.h"
#include <utility>

namespace re {

ps::vector<CommandData> CommandSeparator::separate(ps::string command) {
    ESP_LOGV("Separator", "started.");
    Lexer lexer;
    ps::queue<Token> tokens = lexer.tokenize(command);
    ps::vector<CommandData> ret;
    ps::queue<Token> current_command;

    while (!tokens.empty()) {
        /* Get the command */
        while (!tokens.empty() && tokens.front().lexeme != COMMAND_SEPARATOR) {
            ESP_LOGV("Get cmd", "Adding lexeme: %s", tokens.front().lexeme.c_str());
            current_command.push(tokens.front());
            tokens.pop();
        }

        tokens.pop(); // remove COMMAND_SEPARATOR

        uint8_t state = 0;
        ps::string name;
        ps::vector<ps::vector<ps::string>> args;
        ps::vector<ps::string> expr;
        ESP_LOGV("Parse", "Parsing command");
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

                case 2: // Get the token arguments
                    if (token.lexeme == ")") {
                        args.push_back(expr);
                        expr.clear();
                        state = 3;
                    } else if (token.lexeme == ARGUMENT_SEPARATOR) { // ,
                        args.push_back(expr);
                        expr.clear();
                    } else {
                        expr.push_back(token.lexeme);
                    }

                    break;
                case 3:
                    if (token.lexeme != COMMAND_SEPARATOR) ESP_LOGE("Syntax", "Invalid command syntax");
                break;
            }

            current_command.pop();
        }
        ESP_LOGV("", "Saving");
        auto pr = std::make_tuple(name, args);
        ret.push_back(pr);
    }
    ESP_LOGV("Done", "done.");
    return ret;
}
}