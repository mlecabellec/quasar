#include "quasar/named/NamedService.hpp"
#include <iostream>

namespace quasar::named {

NamedService::NamedService(const std::string& name)
    : NamedObject(name) {}

NamedService::~NamedService() {
    // [CS-0010.44] Ensure the thread is stopped before destruction to avoid crashes.
    stop();
}

std::shared_ptr<NamedService> NamedService::create(const std::string& name, std::shared_ptr<NamedObject> parent) {
    // [CS-0010.10] Use of new or delete keywords is forbidden.
    struct make_shared_enabler : public NamedService {
        explicit make_shared_enabler(const std::string& n) : NamedService(n) {}
    };
    std::shared_ptr<NamedService> self = std::make_shared<make_shared_enabler>(name);
    self->setSelf(self);
    if (parent) {
        self->setParent(parent);
    }

    // [FE-0130.4.1] Create default NamedMethods for start and stop.
    // These methods call the C++ virtual methods start() and stop().
    // [CS-0010.6] Use weak_ptr to avoid circular reference in lambda captures.
    std::weak_ptr<NamedService> weakSelf = self;
    NamedMethod::create("start", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        if (std::shared_ptr<NamedService> s = weakSelf.lock()) {
            s->start();
        }
        return nullptr;
    }, self);

    NamedMethod::create("stop", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        if (std::shared_ptr<NamedService> s = weakSelf.lock()) {
            s->stop();
        }
        return nullptr;
    }, self);

    return self;
}

void NamedService::start() {
    if (m_running) return;

    // Call the optional "configure" hook before starting the thread.
    callHook("configure");

    m_running = true;
    m_thread = std::thread(&NamedService::serviceLoop, this);
}

void NamedService::stop() {
    if (!m_running) return;

    m_running = false;
    {
        std::lock_guard<std::mutex> lock(m_cvMutex);
        m_cv.notify_all();
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool NamedService::isRunning() const {
    return m_running;
}

void NamedService::setCycleTime(std::chrono::milliseconds cycleTime) {
    m_cycleTime = cycleTime;
}

std::string NamedService::getType() const {
    return "NamedService";
}

void NamedService::serviceLoop() {
    m_threadActive = true;

    // Call "onStart" hook if it exists.
    callHook("onStart");

    while (m_running) {
        // [CS-0010.44] Execute the main logic of the service.
        // Check for a "run" NamedMethod child and execute it.
        callHook("run");

        // Wait for next cycle or stop signal.
        std::unique_lock<std::mutex> lock(m_cvMutex);
        m_cv.wait_for(lock, m_cycleTime, [this] { return !m_running; });
    }

    // Call "onStop" hook if it exists.
    callHook("onStop");

    m_threadActive = false;
}

std::shared_ptr<NamedObject> NamedService::callHook(const std::string& methodName, std::shared_ptr<NamedObject> args) {
    // Find the child by name and check if it's a NamedMethod.
    std::shared_ptr<NamedObject> child = getChild(methodName);
    if (child) {
        // [CS-0010.44] Both C++ and Lua methods are supported.
        std::shared_ptr<NamedMethod> method = std::dynamic_pointer_cast<NamedMethod>(child);
        if (method) {
            try {
                return method->execute(args);
            } catch (const std::exception& e) {
                std::cerr << "NamedService [" << getName() << "] error executing hook '" << methodName << "': " << e.what() << std::endl;
            }
        }
    }
    return nullptr;
}

} // namespace quasar::named
