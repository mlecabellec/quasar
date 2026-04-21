#include "ShellCompleter.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/named/NamedObject.hpp"
#include <algorithm>
#include <vector>

namespace quasar::scripting {

/**
 * @brief Constructs the completer.
 */
ShellCompleter::ShellCompleter(sol::state& lua) : m_lua(lua) {}

/**
 * @brief Helper to resolve a nested Lua object from a path string.
 * @param path The path like "root.Service.SubNode"
 * @return sol::object The resolved object or nil.
 */
sol::object resolveLuaPath(sol::state& lua, const std::string& path) {
    if (path.empty()) {
        return lua.globals();
    }

    std::stringstream ss(path);
    std::string segment;
    sol::object current = lua.globals();

    while (std::getline(ss, segment, '.')) {
        if (segment.empty()) continue;
        if (current.is<sol::table>()) {
            current = current.as<sol::table>()[segment];
        } else {
            return sol::lua_nil;
        }
    }
    return current;
}

/**
 * @brief Generates completion suggestions based on the current cursor context.
 */
replxx::Replxx::completions_t ShellCompleter::operator()(std::string const& input, int& contextLen) {
    std::string token = findCurrentToken(input, static_cast<int>(input.size()));
    
    replxx::Replxx::completions_t completions;
    size_t lastDelimiter = token.find_last_of(".:");

    if (lastDelimiter == std::string::npos) {
        // Simple global completion
        contextLen = static_cast<int>(token.size());
        for (std::pair<sol::object, sol::object> const& entry : m_lua.globals()) {
            if (entry.first.is<std::string>()) {
                std::string s = entry.first.as<std::string>();
                if (s.size() >= token.size() && s.compare(0, token.size(), token) == 0) {
                    completions.emplace_back(s);
                }
            }
        }
    } else {
        // Contextual completion: e.g., "root.Node."
        std::string prefix = token.substr(0, lastDelimiter);
        std::string suffix = token.substr(lastDelimiter + 1);
        contextLen = static_cast<int>(suffix.size());

        sol::object obj = resolveLuaPath(m_lua, prefix);
        if (obj.valid()) {
            // Check if it's a NamedObject proxy
            std::shared_ptr<named::NamedObject> no = extractNamedObject(obj);
            if (no) {
                std::list<std::shared_ptr<named::NamedObject>> children = no->getChildren();
                for (std::shared_ptr<named::NamedObject> const& child : children) {
                    std::string name = child->getName();
                    if (name.size() >= suffix.size() && name.compare(0, suffix.size(), suffix) == 0) {
                        completions.emplace_back(name);
                    }
                }
            }

            // Also check for table keys
            if (obj.is<sol::table>()) {
                sol::table t = obj.as<sol::table>();
                for (std::pair<sol::object, sol::object> const& entry : t) {
                    if (entry.first.is<std::string>()) {
                        std::string s = entry.first.as<std::string>();
                        if (s.size() >= suffix.size() && s.compare(0, suffix.size(), suffix) == 0) {
                            completions.emplace_back(s);
                        }
                    }
                }
            }
        }
    }

    // Sort and remove duplicates
    std::sort(completions.begin(), completions.end(), [](const replxx::Replxx::Completion& a, const replxx::Replxx::Completion& b) {
        return a.text() < b.text();
    });
    completions.erase(std::unique(completions.begin(), completions.end(), [](const replxx::Replxx::Completion& a, const replxx::Replxx::Completion& b) {
        return a.text() == b.text();
    }), completions.end());

    return completions;
}

/**
 * @brief Identifies the word-like token immediately preceding the cursor.
 */
std::string ShellCompleter::findCurrentToken(const std::string& input, int cursorIndex) const {
    if (input.empty() || cursorIndex <= 0) {
        return "";
    }

    int start = cursorIndex - 1;
    while (start >= 0) {
        char c = input[static_cast<size_t>(start)];
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == ':') {
            start--;
        } else {
            break;
        }
    }
    return input.substr(static_cast<size_t>(start + 1), static_cast<size_t>(cursorIndex - start - 1));
}

} // namespace quasar::scripting
