#include "quasar/scripting/RegistryBindings_Internal.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/named/traversal/PredefinedRules.hpp"
#include "quasar/named/Serialization.hpp"

namespace quasar::scripting {

using namespace quasar::named;

void bindTraversalTypes(sol::state_view lua, std::shared_ptr<LuaService> service) {
    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    sol::table namedTable = quasarTable["named"].get_or_create<sol::table>();
    sol::table traversalTable = namedTable["traversal"].get_or_create<sol::table>();
    sol::table serializationTable = namedTable["serialization"].get_or_create<sol::table>();

    serializationTable["toJson"] = [](sol::object obj) {
        return quasar::named::serialization::toJson(extractNamedObject(obj));
    };
    serializationTable["fromJson"] = [](const std::string& json, sol::this_state L) {
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromJson(json);
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    };

    traversalTable["TransformContext"] = lua.new_usertype<quasar::named::traversal::TransformContext>("TransformContext", sol::no_constructor,
        "getNode", [](const quasar::named::traversal::TransformContext& ctx) { return LuaProxy<NamedObject>(ctx.getNode()); },
        "getDepth", &quasar::named::traversal::TransformContext::getDepth,
        "getPath", &quasar::named::traversal::TransformContext::getPath
    );

    traversalTable["Transformer"] = lua.new_usertype<quasar::named::traversal::Transformer>("Transformer",
        sol::constructors<quasar::named::traversal::Transformer()>(),
        "transform", [](quasar::named::traversal::Transformer& self, sol::object root, sol::this_state L) {
            std::vector<std::shared_ptr<NamedObject>> res = self.transform(extractNamedObject(root));
            std::vector<LuaProxy<NamedObject>> out;
            size_t id = getEngineId(L);
            for (auto const& ptr : res) {
                if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(id, ptr);
                out.emplace_back(ptr);
            }
            return out;
        }
    );

    sol::table predefinedTable = traversalTable["rules"].get_or_create<sol::table>();
    predefinedTable["sliceBuffer"] = [](const std::string& name, sol::table slices, CopyPolicy policy, int priority) {
        std::vector<quasar::named::traversal::SliceDefinition> v;
        for (size_t i = 1; i <= slices.size(); ++i) {
            sol::table s = slices[i];
            v.push_back({s.get<std::string>("name"), s.get<size_t>("offset"), s.get<size_t>("length")});
        }
        return quasar::named::traversal::PredefinedRules::sliceBuffer(name, v, policy, priority);
    };
}

} // namespace quasar::scripting
