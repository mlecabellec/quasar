#include "ShellUtilities.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/ActiveEntity.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <list>

namespace quasar::scripting {

using namespace ftxui;

/**
 * @brief Renders a single node and its children recursively for the ls command.
 * @param node The current node.
 * @param output The ftxui Elements collection to populate.
 * @param depth Current recursion depth.
 * @compliance [CS-0010.38] Hard limit on recursion.
 */
void renderTreeNode(std::shared_ptr<named::NamedObject> node, Elements& output, int depth) {
    if (!node || depth > 128) {
        return; 
    }

    std::string indent = "";
    for (int i = 0; i < depth; ++i) {
        indent += "  ";
    }
    
    std::string prefix = (depth == 0) ? "" : "└─ ";
    
    output.push_back(hbox(Elements{
        text(indent + prefix),
        text(node->getName()) | color(depth == 0 ? Color::CyanLight : Color::Cyan) | (depth == 0 ? bold : nothing),
        text(" (") | dim,
        text(node->getType()) | color(Color::GrayLight),
        text(")") | dim
    }));

    std::list<std::shared_ptr<named::NamedObject>> children = node->getChildren();
    for (std::shared_ptr<named::NamedObject> const& child : children) {
        renderTreeNode(child, output, depth + 1);
    }
}

/**
 * @brief Implementation of the ls() Lua command.
 * @details Visualizes a sub-tree using ftxui components.
 */
void ls(sol::object target, sol::this_state L) {
    std::shared_ptr<named::NamedObject> no = extractNamedObject(target);
    if (!no) {
        if (target.get_type() == sol::type::lua_nil) {
             // Fallback to searching for 'root' in the global state
             sol::state_view lua(L);
             sol::object rootObj = lua["root"];
             no = extractNamedObject(rootObj);
        }
    }

    if (!no) {
        std::cout << "Error: Target is not a NamedObject. Usage: ls(obj) or ls(root)" << std::endl;
        return;
    }

    Elements entries;
    renderTreeNode(no, entries, 0);

    Element document = vbox(std::move(entries));
    Screen screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
    Render(screen, document);
    std::cout << screen.ToString() << std::endl;
}

/**
 * @brief Implementation of the help() Lua command.
 * @details Displays reflexive fields and methods in a formatted table.
 */
void help(sol::object target) {
    std::shared_ptr<named::NamedObject> no = extractNamedObject(target);
    if (!no) {
        std::cout << "Error: Target is not a NamedObject. Usage: help(obj)" << std::endl;
        return;
    }

    std::vector<std::vector<std::string>> rows;
    rows.push_back({"Property", "Value"});
    rows.push_back({"Name", no->getName()});
    rows.push_back({"Type", no->getType()});

    std::shared_ptr<named::ActiveEntity> ae = std::dynamic_pointer_cast<named::ActiveEntity>(no);
    if (ae) {
        std::string stateStr = "Unknown";
        switch (ae->getState()) {
            case named::EntityState::Uninitialized: stateStr = "Uninitialized"; break;
            case named::EntityState::Ready: stateStr = "Ready"; break;
            case named::EntityState::Running: stateStr = "Running"; break;
            case named::EntityState::Error: stateStr = "Error"; break;
        }
        rows.push_back({"State", stateStr});

        std::vector<std::string> fields = ae->listFields();
        std::string fieldsStr = "";
        for (std::string const& f : fields) {
            fieldsStr += f + " ";
        }
        rows.push_back({"Reflexive Fields", fieldsStr.empty() ? "(none)" : fieldsStr});

        std::vector<std::string> methods = ae->listMethods();
        std::string methodsStr = "";
        for (std::string const& m : methods) {
            methodsStr += m + "() ";
        }
        rows.push_back({"Reflexive Methods", methodsStr.empty() ? "(none)" : methodsStr});
    }

    Table table(rows);
    table.SelectAll().Separator(LIGHT);
    table.SelectRow(0).Decorate(bold);
    table.SelectColumn(0).Decorate(color(Color::YellowLight));

    Element document = table.Render();
    Screen screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
    Render(screen, document);
    std::cout << screen.ToString() << std::endl;
}

/**
 * @brief Registers the utilities to the Lua state.
 */
void bindShellUtilities(sol::state& lua) {
    lua.set_function("ls", ls);
    lua.set_function("help", help);
}

} // namespace quasar::scripting
