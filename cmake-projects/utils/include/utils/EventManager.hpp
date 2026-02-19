#pragma once

#include <Smp/IEntryPoint.h>
#include <Smp/Services/IEventManager.h>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace utils {

class EventManager : public Smp::Services::IEventManager {
public:
  EventManager();
  virtual ~EventManager() noexcept = default;

  Smp::Services::EventId QueryEventId(Smp::String8 eventName) override;

  void Subscribe(Smp::Services::EventId event,
                 const Smp::IEntryPoint *entryPoint) override;
  void Unsubscribe(Smp::Services::EventId event,
                   const Smp::IEntryPoint *entryPoint) override;
  void Emit(Smp::Services::EventId event,
            Smp::Bool synchronous = true) override;

private:
  Smp::Services::EventId _nextEventId = 20; // Start after pre-defined events
  std::map<std::string, Smp::Services::EventId> _eventIds;
  std::map<Smp::Services::EventId, std::string> _eventNames;
  std::map<Smp::Services::EventId, std::vector<const Smp::IEntryPoint *>>
      _subscriptions;

  mutable std::mutex _mutex;

  void RegisterPredefinedEvents();
};

} // namespace utils
