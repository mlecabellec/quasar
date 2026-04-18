/**
 * @file Expression.cpp
 * @brief Implementation of the Lua expression bridge with Proxy support.
 */

#include "quasar/logic/Expression.hpp"
#include "quasar/logic/EvaluationPool.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include <stdexcept>

namespace quasar::logic {

/**
 * @class LogicProxy
 * @brief Provides safe access from Lua to the NamedObject tree.
 */
class LogicProxy {
public:
    explicit LogicProxy(quasar::named::NamedObject* node, size_t workerId) : m_node(node), m_workerId(workerId) {}

    sol::object index(const std::string& name, sol::this_state s) {
        if (!m_node) return sol::nil;
        
        quasar::named::NamedObject* child = nullptr;
        
        try {
            // [CS-0010.44] Explicitly iterate children to find by name.
            std::list<std::shared_ptr<quasar::named::NamedObject>> children = m_node->getChildren();
            for (std::list<std::shared_ptr<quasar::named::NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
                if (*it && (*it)->getName() == name) {
                    child = (*it).get();
                    break;
                }
            }
        } catch (const std::exception&) {
            return sol::nil;
        }
        
        if (!child) return sol::nil;

        // Leaf detection and automatic conversion to primitives for fast expressions.
        if (const quasar::named::NamedInteger<int64_t>* i64 = dynamic_cast<const quasar::named::NamedInteger<int64_t>*>(child)) return sol::make_object(s, i64->value());
        if (const quasar::named::NamedInteger<int32_t>* i32 = dynamic_cast<const quasar::named::NamedInteger<int32_t>*>(child)) return sol::make_object(s, i32->value());
        if (const quasar::named::NamedInteger<int>* i = dynamic_cast<const quasar::named::NamedInteger<int>*>(child)) return sol::make_object(s, i->value());
        if (const quasar::named::NamedBoolean* b = dynamic_cast<const quasar::named::NamedBoolean*>(child)) return sol::make_object(s, b->booleanValue());
        if (const quasar::named::NamedString* str = dynamic_cast<const quasar::named::NamedString*>(child)) return sol::make_object(s, str->toString());
        if (const quasar::named::NamedFloatingPoint<double>* d = dynamic_cast<const quasar::named::NamedFloatingPoint<double>*>(child)) return sol::make_object(s, d->value());
        if (const quasar::named::NamedFloatingPoint<float>* f = dynamic_cast<const quasar::named::NamedFloatingPoint<float>*>(child)) return sol::make_object(s, f->value());

        // Return a proxy for the sub-branch.
        return sol::make_object(s, LogicProxy(child, m_workerId));
    }

private:
    quasar::named::NamedObject* m_node;
    size_t m_workerId;
};

Expression::Expression(sol::state& lua, const std::string& source) : m_lua(&lua) {
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

    EvaluationStatus status = EvaluationPool::getInstance().evaluate(m_bytecode, contextRoot);
    return status == EvaluationStatus::Success;
}

void Expression::bindContext(sol::state& lua, const std::shared_ptr<quasar::named::NamedObject>& contextRoot) {
    if (!contextRoot) return;

    // Fetch worker ID from global if present (used by EvaluationPool).
    size_t workerId = 0;
    sol::object idObj = lua["__logic_worker_id"];
    if (idObj.is<size_t>()) {
        workerId = idObj.as<size_t>();
    }

    // [CS-0010.44] ctx is the standard entry point for expression evaluation.
    lua["ctx"] = LogicProxy(contextRoot.get(), workerId);
}

void registerLogicTypes(sol::state& lua) {
    // [CS-0010.5] Register once to ensure thread-safe metadata and stable metatables.
    lua.new_usertype<LogicProxy>("LogicProxy",
        sol::meta_function::index, &LogicProxy::index
    );
}

} // namespace quasar::logic
