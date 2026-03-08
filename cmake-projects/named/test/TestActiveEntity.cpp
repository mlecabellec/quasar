#include "quasar/named/ActiveEntity.hpp"
#include "quasar/named/NamedObject.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <iostream>

using namespace quasar::named;

// A simple mock for event data
class MockEventData : public NamedObject {
public:
    static std::shared_ptr<MockEventData> create(const std::string& name) {
        auto ptr = std::shared_ptr<MockEventData>(new MockEventData(name));
        ptr->setSelf(ptr);
        return ptr;
    }
protected:
    MockEventData(const std::string& name) : NamedObject(name) {}
};

// Mock observer
class MockObserver : public IObserver {
public:
    int notificationCount = 0;
    std::shared_ptr<NamedObject> lastEventData;

    void notify(std::shared_ptr<NamedObject> eventData) override {
        notificationCount++;
        lastEventData = eventData;
    }
};

// Mock device implementing ActiveEntity
class MockTemperatureSensor : public ActiveEntity {
public:
    static std::shared_ptr<MockTemperatureSensor> create(const std::string& name, std::shared_ptr<NamedObject> parent = nullptr) {
        auto ptr = std::shared_ptr<MockTemperatureSensor>(new MockTemperatureSensor(name));
        ptr->setSelf(ptr);
        if (parent) {
            ptr->setParent(parent);
        }
        ptr->initReflexivity();
        return ptr;
    }

    void initialize() override {
        setState(EntityState::Ready);
    }

    void start() override {
        setState(EntityState::Running);
        auto event = MockEventData::create("sensorStarted");
        notifyObservers(event);
    }

    void stop() override {
        setState(EntityState::Ready);
    }

    void reset() override {
        setState(EntityState::Uninitialized);
    }

    // Direct access for validation in tests
    std::shared_ptr<NamedObject> getTemperatureNode() {
        return m_temperature;
    }

protected:
    MockTemperatureSensor(const std::string& name) : ActiveEntity(name) {}

    void initReflexivity() {
        m_temperature = NamedObject::create("temperature", getSelf());
        REGISTER_FIELD("temperature", m_temperature);

        REGISTER_METHOD("calibrate", [this](std::shared_ptr<NamedObject> args) {
            return this->calibrate(args);
        });
    }

    std::shared_ptr<NamedObject> calibrate(std::shared_ptr<NamedObject> args) {
        (void)args; // calibration ignores args for mockup
        m_calibrateCount++;
        return NamedObject::create("calibrationResult", getSelf());
    }

private:
    std::shared_ptr<NamedObject> m_temperature;
public:
    int m_calibrateCount = 0;
};

TEST(TestActiveEntity, LifecycleTransitions) {
    auto sensor = MockTemperatureSensor::create("sensor1");
    
    EXPECT_EQ(sensor->getState(), EntityState::Uninitialized);
    
    sensor->initialize();
    EXPECT_EQ(sensor->getState(), EntityState::Ready);
    
    sensor->start();
    EXPECT_EQ(sensor->getState(), EntityState::Running);
    
    sensor->stop();
    EXPECT_EQ(sensor->getState(), EntityState::Ready);
    
    sensor->reset();
    EXPECT_EQ(sensor->getState(), EntityState::Uninitialized);
}

TEST(TestActiveEntity, ObserverPattern) {
    auto sensor = MockTemperatureSensor::create("sensor1");
    auto observer = std::make_shared<MockObserver>();
    
    sensor->subscribe(observer);
    
    EXPECT_EQ(observer->notificationCount, 0);
    
    sensor->initialize();
    sensor->start(); // This triggers notifyObservers("sensorStarted")
    
    EXPECT_EQ(observer->notificationCount, 1);
    ASSERT_NE(observer->lastEventData, nullptr);
    EXPECT_EQ(observer->lastEventData->getName(), "sensorStarted");

    sensor->unsubscribe(observer);
    sensor->start(); // Triggers again but observer should be removed
    
    EXPECT_EQ(observer->notificationCount, 1); // No increment
}

TEST(TestActiveEntity, FieldReflexivity) {
    auto sensor = MockTemperatureSensor::create("sensor1");
    
    auto fields = sensor->listFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0], "temperature");
    
    auto tempField = sensor->getField("temperature");
    ASSERT_NE(tempField, nullptr);
    EXPECT_EQ(tempField->getName(), "temperature");
    EXPECT_EQ(tempField, sensor->getTemperatureNode());
    
    EXPECT_EQ(sensor->getField("nonExistent"), nullptr);
}

TEST(TestActiveEntity, MethodReflexivity) {
    auto sensor = MockTemperatureSensor::create("sensor1");
    
    auto methods = sensor->listMethods();
    ASSERT_EQ(methods.size(), 1);
    EXPECT_EQ(methods[0], "calibrate");
    
    auto dummyArgs = NamedObject::create("args");
    
    EXPECT_EQ(sensor->m_calibrateCount, 0);
    
    // Execute dynamic method
    auto res = sensor->execute("calibrate", dummyArgs);
    
    EXPECT_EQ(sensor->m_calibrateCount, 1);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->getName(), "calibrationResult");
    
    // Test non-existent method exception
    EXPECT_THROW(sensor->execute("unknownMethod", dummyArgs), std::runtime_error);
}
