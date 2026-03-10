#include "quasar/scripting/PluginContract.hpp"
#include "quasar/scripting/ScriptableNamedObject.hpp"
#include <iostream>

namespace test_plugin {

class MyComponent : public quasar::scripting::ScriptableNamedObject {
public:
    static std::shared_ptr<MyComponent> create(const std::string& name) {
        struct Enabler : public MyComponent {
            Enabler(const std::string& n) : MyComponent(n) {}
        };
        auto obj = std::make_shared<Enabler>(name);
        obj->setSelf(obj);
        return obj;
    }

    void performAction() {
        std::cout << "MyComponent '" << getName() << "' is performing an action!\n";
    }

protected:
    MyComponent(const std::string& name) : quasar::scripting::ScriptableNamedObject(name) {}
};

} // namespace test_plugin

extern "C" {
    QUASAR_PLUGIN_EXPORT void registerPluginComponents(sol::state_view lua) {
        std::cout << "Test Plugin: Registering MyComponent...\n";

        auto myComponentExt = lua.new_usertype<test_plugin::MyComponent>("MyComponent",
            sol::base_classes, sol::bases<quasar::named::NamedObject, quasar::scripting::ScriptableNamedObject>()
        );
        
        myComponentExt["new"] = [](const std::string& name) {
            return test_plugin::MyComponent::create(name);
        };
        myComponentExt["performAction"] = &test_plugin::MyComponent::performAction;
        
        std::cout << "Test Plugin: Registration complete.\n";
    }
}
