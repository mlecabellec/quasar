/**
 * @file TestQlsh.cpp
 * @brief Functional and stress tests for the Quasar Lua Shell (qlsh).
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260421-001] Interactive Lua Shell testing requirements.
 * - Adheres to [CS-0010.34] No auto keyword.
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <memory>
#include <string>
#include <array>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

namespace quasar::scripting {

/**
 * @class QlshTest
 * @brief Test fixture for spawning and interacting with the qlsh executable.
 */
class QlshTest : public ::testing::Test {
protected:
    /**
     * @brief Executes qlsh with the provided Lua input and captures output.
     * @param lua_input The commands to send to qlsh.
     * @param plugins List of plugin filenames to load.
     * @return Standard output and error from the qlsh process.
     */
    std::string run_qlsh(const std::string& lua_input, const std::vector<std::string>& plugins = {}) {
        // Write input to a temporary file to ensure reliable delivery to qlsh stdin
        std::string inputPath = "qlsh_input.tmp";
        std::ofstream tmp_input(inputPath);
        tmp_input << lua_input << "\n";
        tmp_input.close();

        // Robustly locate qlsh binary and lib directory
        std::string qlsh_path = "./bin/qlsh";
        std::string lib_path = "./lib/";

        if (!std::filesystem::exists(qlsh_path)) {
            qlsh_path = "./qlsh"; // If running from bin/
            lib_path = "../lib/";
        }
        if (!std::filesystem::exists(qlsh_path)) {
             qlsh_path = "../../../bin/qlsh"; // If running from build/cmake-projects/scripting/test
             lib_path = "../../../lib/";
        }
        if (!std::filesystem::exists(qlsh_path)) {
             qlsh_path = "../../bin/qlsh"; // Fallback
             lib_path = "../../lib/";
        }

        // Construct command line
        std::string command = qlsh_path;
        for (std::string const& p : plugins) {
            command += " --plugin " + lib_path + p;
        }
        command += " < " + inputPath + " 2>&1";

        std::array<char, 128> buffer;
        std::string result;
        
        // Execute qlsh in a sub-shell
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return "ERROR: popen failed";
        }

        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
            result += buffer.data();
        }
        
        pclose(pipe);
        std::remove(inputPath.c_str());
        return result;
    }
};

/**
 * @brief Verifies that basic Lua arithmetic and print work in qlsh.
 */
TEST_F(QlshTest, BasicLuaExecution) {
    std::string output = run_qlsh("print(1000 + 337)");
    EXPECT_NE(output.find("1337"), std::string::npos);
}

/**
 * @brief Verifies interaction with the Quasar NamedObject hierarchy.
 */
TEST_F(QlshTest, NamedObjectInteraction) {
    std::string script = "root = quasar.named.createObject('testRoot')\n"
                         "node = quasar.named.createLong('val', 777, root)\n"
                         "print('FOUND_VAL=' .. root:getChild('val'):asLong():value())";
    std::string output = run_qlsh(script);
    EXPECT_NE(output.find("FOUND_VAL=777"), std::string::npos);
}

/**
 * @brief Verifies that the ZMQ plugin can be loaded and used within qlsh.
 */
TEST_F(QlshTest, ZmqPluginInteraction) {
    std::string script = "ctx = quasar.zmq.Context.new()\n"
                         "print('ZMQ_CONTEXT_READY')\n";
    std::string output = run_qlsh(script, {"quasar_zmq.so"});
    EXPECT_NE(output.find("ZMQ_CONTEXT_READY"), std::string::npos);
}

/**
 * @brief Verifies the multi-line statement accumulator logic.
 */
TEST_F(QlshTest, MultiLineAccumulation) {
    std::string script = "function adder(a, b)\n"
                         "  return a + b\n"
                         "end\n"
                         "print('SUM=' .. adder(10, 20))";
    std::string output = run_qlsh(script);
    EXPECT_NE(output.find("SUM=30"), std::string::npos);
}

/**
 * @brief Verifies shell-specific diagnostic utilities (ls, help).
 */
TEST_F(QlshTest, ShellUtilities) {
    std::string script = "root = quasar.named.createObject('uiRoot')\n"
                         "ls(root)\n"
                         "help(root)";
    std::string output = run_qlsh(script);
    EXPECT_NE(output.find("uiRoot"), std::string::npos);
    EXPECT_NE(output.find("Property"), std::string::npos); // help() table header
    EXPECT_NE(output.find("Value"), std::string::npos);    // help() table header
}

/**
 * @brief Verifies background service execution within qlsh.
 */
TEST_F(QlshTest, NamedServiceInShell) {
    std::string script = "root = quasar.named.createObject('root')\n"
                         "svc = quasar.named.createService('mySvc', root)\n"
                         "counter = quasar.named.createLong('counter', 0, svc)\n"
                         "quasar.named.createLuaMethod('run', function(owner, args)\n"
                         "  local c_obj = owner:getChild('counter')\n"
                         "  if not c_obj then return end\n"
                         "  local c = c_obj:asLong()\n"
                         "  if not c then return end\n"
                         "  c:setValue(c:value() + 1)\n"
                         "end, svc)\n"
                         "svc:setCycleTime(10)\n"
                         "svc:start()\n"
                         "quasar.sleep(300)\n"
                         "print('COUNT=' .. counter:value())\n"
                         "quasar.sleep(100)\n"
                         "svc:stop()";
    std::string output = run_qlsh(script);
    if (output.find("COUNT=") == std::string::npos) {
        std::cerr << "--- SERVICE TEST OUTPUT START ---\n" << output << "\n--- SERVICE TEST OUTPUT END ---\n";
    }
    EXPECT_NE(output.find("COUNT="), std::string::npos);
    // We expect at least a few increments
    size_t pos = output.find("COUNT=");
    if (pos != std::string::npos) {
        std::string valStr = output.substr(pos + 6);
        // Find end of number
        size_t endPos = 0;
        while (endPos < valStr.size() && std::isdigit(valStr[endPos])) endPos++;
        if (endPos > 0) {
            int count = std::stoi(valStr.substr(0, endPos));
            EXPECT_GT(count, 0);
        }
    }
}

/**
 * @brief Verifies that variables are persisted between interactive lines.
 */
TEST_F(QlshTest, VariablePersistence) {
    // Send two separate statements. qlsh should process them one by one.
    std::string script = "myVar = 12345\n"
                         "print('MYVAR_VALUE=' .. myVar)";
    std::string output = run_qlsh(script);
    EXPECT_NE(output.find("MYVAR_VALUE=12345"), std::string::npos);

    // Test with local (now SHOULD persist due to liftLocals)
    std::string scriptLocal = "local myLocal = 'persistent_secret'\n"
                              "print('VAL=' .. tostring(myLocal))";
    std::string outputLocal = run_qlsh(scriptLocal);
    EXPECT_NE(outputLocal.find("VAL=persistent_secret"), std::string::npos);
}

/**
 * @brief Verifies that the exit() command works.
 */
TEST_F(QlshTest, ExitCommand) {
    std::string output = run_qlsh("exit()\nprint('SHOULD_NOT_SEE_THIS')");
    EXPECT_EQ(output.find("SHOULD_NOT_SEE_THIS"), std::string::npos);
}

/**
 * @brief Reproduces and verifies the fix for the user reported sequence failure.
 */
TEST_F(QlshTest, UserReportedSequenceFix) {
    std::string script = "local n = quasar.named\n"
                         "local buf = n.createBuffer('TheMatrix', 16)\n"
                         "print('BUF_SIZE=' .. buf:getSize())";
    
    std::string output = run_qlsh(script);
    
    // Now this should PASS due to liftLocals
    EXPECT_EQ(output.find("Lua Error"), std::string::npos);
    EXPECT_NE(output.find("BUF_SIZE=16"), std::string::npos);
}

/**
 * @brief Verifies that multiple global variables are persisted.
 */
TEST_F(QlshTest, MultipleGlobalPersistence) {
    std::string script = "x = 10\n"
                         "y = 20\n"
                         "print('SUM_XY=' .. x + y)";
    std::string output = run_qlsh(script);
    EXPECT_NE(output.find("SUM_XY=30"), std::string::npos);
}

/**
 * @brief Verifies the Evaluate-and-Print feature (implicit return).
 */
TEST_F(QlshTest, EvaluateAndPrint) {
    std::string script = "40 + 2";
    std::string output = run_qlsh(script);
    EXPECT_NE(output.find("42"), std::string::npos);
}

/**
 * @brief Verifies complex table management in qlsh.
 */
TEST_F(QlshTest, TableManagement) {
    std::string script = "myTable = { a = 1, b = 2 }\n"
                         "myTable.c = 3\n"
                         "print('TABLE_SUM=' .. myTable.a + myTable.b + myTable.c)";
    std::string output = run_qlsh(script);
    EXPECT_NE(output.find("TABLE_SUM=6"), std::string::npos);
}

/**
 * @brief Verifies that local variables within multiple methods do not interfere (Spec Compliance).
 */
TEST_F(QlshTest, LexicalScopingCompliance) {
    std::string script = 
        "local x = 1\n"
        "function getOuter() return x end\n"
        "function methodA()\n"
        "  local x = 2\n"
        "  return x\n"
        "end\n"
        "function methodB()\n"
        "  local x = 3\n"
        "  return x\n"
        "end\n"
        "print('OUTER=' .. getOuter())\n"
        "print('A=' .. methodA())\n"
        "print('B=' .. methodB())\n"
        "x = 10\n"
        "print('UPDATED_OUTER=' .. getOuter())";
    
    std::string output = run_qlsh(script);
    if (output.find("UPDATED_OUTER=10") == std::string::npos) {
        std::cerr << "--- SCOPING TEST OUTPUT START ---\n" << output << "\n--- SCOPING TEST OUTPUT END ---\n";
    }
    EXPECT_NE(output.find("OUTER=1"), std::string::npos);
    EXPECT_NE(output.find("A=2"), std::string::npos);
    EXPECT_NE(output.find("B=3"), std::string::npos);
    EXPECT_NE(output.find("UPDATED_OUTER=10"), std::string::npos);
}

/**
 * @brief Stress test: creating a massive tree and verifying structural integrity.
 */
TEST_F(QlshTest, HighVolumeStressTest) {
    // Create 1000 nodes and then verify count via Lua
    std::string script = "stressRoot = quasar.named.createObject('stress')\n"
                         "for i=1,1000 do quasar.named.createLong('item'..i, i, stressRoot) end\n"
                         "print('COUNT_RESULT=' .. #stressRoot:getChildren())\n";
    
    std::string output = run_qlsh(script);
    EXPECT_NE(output.find("COUNT_RESULT=1000"), std::string::npos);
}

} // namespace quasar::scripting
