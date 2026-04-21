/**
 * @file ShellHighlighter.hpp
 * @brief Real-time Lua syntax highlighting for qlsh.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260421-001.2.3] Grammar and correctness verification.
 * - Fulfills [CS-0010.45] Doxygen documentation.
 */

#pragma once
#include <replxx.hxx>
#include <string>
#include <vector>
#include <unordered_set>

namespace quasar::scripting {

/**
 * @brief Provides real-time syntax highlighting for the Quasar Lua Shell.
 * @details Identifies Lua keywords, comments, and strings to apply ANSI colors.
 */
class ShellHighlighter {
public:
    /**
     * @brief Constructs the highlighter and initializes the keyword set.
     */
    ShellHighlighter();

    /**
     * @brief Callback for replxx to apply colors to the input line.
     * @param input The current input line.
     * @param colors Vector of colors to be populated.
     */
    void operator()(std::string const& input, replxx::Replxx::colors_t& colors);

private:
    std::unordered_set<std::string> m_keywords;
};

} // namespace quasar::scripting
