#include "ShellHighlighter.hpp"
#include <cctype>

namespace quasar::scripting {

/**
 * @brief Initializes the Lua keyword set with standard reserved words.
 */
ShellHighlighter::ShellHighlighter() : m_keywords({
    "and", "break", "do", "else", "elseif", "end", "false", "for",
    "function", "if", "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while", "goto"
}) {}

/**
 * @brief Lexes the input string and applies colors based on token types.
 */
void ShellHighlighter::operator()(std::string const& input, replxx::Replxx::colors_t& colors) {
    size_t i = 0;
    while (i < input.size()) {
        char c = input[i];
        
        // Skip whitespace
        if (std::isspace(c)) {
            colors[i++] = replxx::Replxx::Color::DEFAULT;
            continue;
        }

        // Handle Lua Comments (--...)
        if (c == '-' && i + 1 < input.size() && input[i+1] == '-') {
            while (i < input.size()) {
                colors[i++] = replxx::Replxx::Color::GRAY;
            }
            break;
        }

        // Handle Strings ("..." or '...')
        if (c == '"' || c == '\'') {
            char quote = c;
            colors[i++] = replxx::Replxx::Color::BRIGHTGREEN;
            while (i < input.size()) {
                colors[i] = replxx::Replxx::Color::BRIGHTGREEN;
                // Basic escape support
                if (input[i] == quote && (i == 0 || input[i-1] != '\\')) {
                    i++; 
                    break; 
                }
                i++;
            }
            continue;
        }

        // Handle Identifiers and Keywords
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t start = i;
            while (i < input.size() && (std::isalnum(static_cast<unsigned char>(input[i])) || input[i] == '_')) {
                i++;
            }
            std::string word = input.substr(start, i - start);
            
            replxx::Replxx::Color color = replxx::Replxx::Color::DEFAULT;
            if (m_keywords.find(word) != m_keywords.end()) {
                color = replxx::Replxx::Color::BRIGHTMAGENTA;
            } else if (std::isupper(static_cast<unsigned char>(word[0]))) {
                // Highlighting hint for Quasar objects which typically start with UpperCase
                color = replxx::Replxx::Color::BRIGHTCYAN;
            }

            for (size_t k = start; k < i; ++k) {
                colors[k] = color;
            }
            continue;
        }

        // Handle Numbers
        if (std::isdigit(static_cast<unsigned char>(c))) {
             while (i < input.size() && (std::isalnum(static_cast<unsigned char>(input[i])) || input[i] == '.')) {
                 colors[i++] = replxx::Replxx::Color::BRIGHTBLUE;
             }
             continue;
        }

        // Default operator/punctuation coloring
        colors[i++] = replxx::Replxx::Color::DEFAULT;
    }
}

} // namespace quasar::scripting
