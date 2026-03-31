/**
 * @file Expression.cpp
 * @brief Implementation of the Lua expression bridge.
 */

#include "quasar/logic/Expression.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include <stdexcept>

namespace quasar::logic {

Expression::Expression(sol::state& lua, const std::string& source) : m_lua(&lua) {
    // Compile to bytecode
    sol::load_result loadResult = m_lua->load("return (" + source + ")");
    if (!loadResult.valid()) {
        sol::error err = loadResult;
        throw std::runtime_error("Failed to compile Lua expression: " + std::string(err.what()));
    }

    sol::protected_function func = loadResult;
    sol::bytecode bc = func.dump();
    m_bytecode.assign(reinterpret_cast<const std::byte*>(bc.data()), 
                      reinterpret_cast<const std::byte*>(bc.data()) + bc.size());
}

bool Expression::evaluate(const std::shared_ptr<quasar::named::NamedObject>& contextRoot) const {
    if (isEmpty()) {
        return true;
    }

    // Prepare 'ctx' table
    sol::table ctx = m_lua->create_table();
    if (contextRoot) {
        mapTreeToLua(contextRoot, ctx);
    }
    (*m_lua)["ctx"] = ctx;

    // Load and execute bytecode
    sol::load_result loadResult = m_lua->load(std::string_view(reinterpret_cast<const char*>(m_bytecode.data()), m_bytecode.size()));
    if (!loadResult.valid()) {
        return false;
    }

    sol::protected_function func = loadResult;
    sol::protected_function_result result = func();
    
    if (!result.valid()) {
        return false;
    }

    return result.get<bool>();
}

void Expression::mapTreeToLua(const std::shared_ptr<quasar::named::NamedObject>& node, sol::table& table) {
    if (!node) return;

    // Iterate over children
    std::list<std::shared_ptr<quasar::named::NamedObject>> children = node->getChildren();
    for (std::list<std::shared_ptr<quasar::named::NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
        std::shared_ptr<quasar::named::NamedObject>& child = *it;
        const std::string& name = child->getName();

        // Handle primitives using raw dynamic_cast for template detection
        quasar::named::NamedObject* raw = child.get();

        if (const quasar::named::NamedInteger<int64_t>* int64Obj = dynamic_cast<const quasar::named::NamedInteger<int64_t>*>(raw)) {
            table[name] = int64Obj->value();
        } else if (const quasar::named::NamedInteger<int32_t>* int32Obj = dynamic_cast<const quasar::named::NamedInteger<int32_t>*>(raw)) {
            table[name] = int32Obj->value();
        } else if (const quasar::named::NamedInteger<int>* intObj = dynamic_cast<const quasar::named::NamedInteger<int>*>(raw)) {
            table[name] = intObj->value();
        } else if (const quasar::named::NamedBoolean* boolObj = dynamic_cast<const quasar::named::NamedBoolean*>(raw)) {
            table[name] = boolObj->booleanValue();
        } else if (const quasar::named::NamedString* strObj = dynamic_cast<const quasar::named::NamedString*>(raw)) {
            table[name] = strObj->toString();
        } else if (const quasar::named::NamedFloatingPoint<double>* doubleObj = dynamic_cast<const quasar::named::NamedFloatingPoint<double>*>(raw)) {
            table[name] = doubleObj->value();
        } else if (const quasar::named::NamedFloatingPoint<float>* floatObj = dynamic_cast<const quasar::named::NamedFloatingPoint<float>*>(raw)) {
            table[name] = floatObj->value();
        } else {
            // It's a branch, recurse
            sol::table subTable = table.create_with();
            mapTreeToLua(child, subTable);
            table[name] = subTable;
        }
    }
}

} // namespace quasar::logic
