#include "quasar/scripting/ScriptConsole.hpp"
#include <iostream>
#include <vector>

namespace quasar::scripting {

void ScriptConsole::run(const std::string& prompt, std::function<void(const std::string&)> executor) {
    std::string line;
    std::cout << "Quasar Scripting Console. Type 'exit' to quit." << std::endl;
    
    while (true) {
        std::cout << prompt << " " << std::flush;
        if (!std::getline(std::cin, line) || line == "exit") {
            break;
        }
        
        if (line.empty()) continue;
        
        try {
            executor(line);
        } catch (const std::exception& e) {
            std::cerr << "REPL Error: " << e.what() << std::endl;
        }
    }
}

} // namespace quasar::scripting
