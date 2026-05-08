#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "resoem/EthercatMasterService.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

using namespace ftxui;
using namespace resoem;
using namespace quasar::named;

int main(int argc, char* argv[]) {
    // 1. Initialize Service
    std::shared_ptr<NamedObject> root = NamedObject::create("root");
    std::shared_ptr<EthercatMasterService> service = EthercatMasterService::create("EcatMaster", root);
    service->setInterface("lo");
    service->start();

    ScreenInteractive screen = ScreenInteractive::TerminalOutput();

    // 2. State & UI Components
    int selected_slave = 0;
    std::vector<std::string> slaves_list_cache = {"<No Slaves Found>"};
    std::mutex data_mutex;

    // 3. Logic to fetch tree data
    std::function<void()> update_data = [&]() {
        std::lock_guard<std::mutex> lock(data_mutex);
        std::vector<std::string> list;
        std::shared_ptr<NamedObject> slavesNode = service->getChild("slaves");
        if (slavesNode != nullptr) {
            std::list<std::shared_ptr<NamedObject>> children = slavesNode->getChildren();
            for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
                std::shared_ptr<NamedObject> child = *it;
                list.push_back(child->getName());
            }
        }
        if (list.empty() == true) list.push_back("<No Slaves Found>");
        slaves_list_cache = std::move(list);
    };

    // 4. Component Definitions
    Component menu = Menu(&slaves_list_cache, &selected_slave);

    Component renderer = Renderer(menu, [&] {
        std::lock_guard<std::mutex> lock(data_mutex);
        
        // Detailed info for selected slave
        Elements details;
        std::shared_ptr<NamedObject> slavesNode = service->getChild("slaves");
        if (slavesNode != nullptr) {
            std::list<std::shared_ptr<NamedObject>> children = slavesNode->getChildren();
            if (selected_slave < static_cast<int>(children.size())) {
                std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin();
                std::advance(it, selected_slave);
                std::shared_ptr<NamedObject> slave = *it;
                details.push_back(text("Name: " + slave->getName()));
                std::shared_ptr<NamedObject> diag = slave->getChild("diagnostics");
                if (diag != nullptr) {
                     std::shared_ptr<NamedObject> st = diag->getChild("al_status");
                     if (st != nullptr) {
                         std::shared_ptr<NamedInteger<uint16_t>> stInt = std::dynamic_pointer_cast<NamedInteger<uint16_t>>(st);
                         if (stInt != nullptr) {
                            details.push_back(text("Status: 0x" + std::to_string(stInt->value())));
                         }
                     }
                }
            }
        }

        return hbox(
            window(text(" Physical Bus "), menu->Render()) | flex,
            window(text(" Slave Details "), vbox(std::move(details))) | flex
        );
    });

    // 5. Global Actions
    renderer |= CatchEvent([&](Event event) {
        if (event == Event::Character('q')) {
            screen.Exit();
            return true;
        }
        if (event == Event::Character('r')) {
            // Find and execute the method manually since NamedService doesn't expose execute()
            std::shared_ptr<NamedObject> methodObj = service->getChild("refreshStatus");
            std::shared_ptr<NamedMethod> method = std::dynamic_pointer_cast<NamedMethod>(methodObj);
            if (method != nullptr) {
                method->execute(nullptr);
            }
            return true;
        }
        return false;
    });

    // 6. Refresh loop (separate thread)
    std::atomic<bool> refresh_active{true};
    std::thread refresh_thread([&] {
        while (refresh_active == true) {
            update_data();
            screen.PostEvent(Event::Custom);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    screen.Loop(renderer);

    refresh_active = false;
    refresh_thread.join();
    service->stop();

    return 0;
}
