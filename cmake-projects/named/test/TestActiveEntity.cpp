#include "quasar/named/ActiveEntity.hpp"
#include "quasar/named/NamedObject.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <iostream>

using namespace quasar::named;

/**
 * @class MockEventData
 * @brief A simple mock for event data.
 */
class MockEventData : public NamedObject {
public:
    /**
     * @brief Factory method.
     * @param name Object name.
     * @return Shared pointer.
     */
    static std::shared_ptr<MockEventData> create(const std::string& name) {
        // [CS-0010.10] Use of new forbidden. Use make_shared with helper.
        struct Helper : public MockEventData {
            explicit Helper(const std::string& n) : MockEventData(n) {}
        };
        std::shared_ptr<MockEventData> ptr = std::make_shared<Helper>(name);
        ptr->setSelf(ptr);
        return ptr;
    }
protected:
    /**
     * @brief Constructor.
     * @param name Object name.
     */
    explicit MockEventData(const std::string& name) : NamedObject(name) {}
};

/**
 * @class MockObserver
 * @brief Mock observer for testing ActiveEntity.
 */
class MockObserver : public IObserver {
public:
    /** @brief Number of notifications received. */
    int notificationCount = 0;
    /** @brief Last received event data. */
    std::shared_ptr<NamedObject> lastEventData;

    /**
     * @brief Notify implementation.
     * @param eventData The event data.
     */
    void notify(std::shared_ptr<NamedObject> eventData) override {
        notificationCount++;
        lastEventData = eventData;
    }
};

/**
 * @class MockTemperatureSensor
 * @brief Mock device implementing ActiveEntity.
 * @compliance [FE-0130] Testing ActiveEntity capabilities.
 */
class MockTemperatureSensor : public ActiveEntity {
public:
    /**
     * @brief Factory method.
     * @param name Object name.
     * @param parent Optional parent.
     * @return Shared pointer.
     */
    static std::shared_ptr<MockTemperatureSensor> create(const std::string& name, std::shared_ptr<NamedObject> parent = nullptr) {
        // [CS-0010.10] Use of new forbidden.
        struct Helper : public MockTemperatureSensor {
            explicit Helper(const std::string& n) : MockTemperatureSensor(n) {}
        };
        std::shared_ptr<MockTemperatureSensor> ptr = std::make_shared<Helper>(name);
        ptr->setSelf(ptr);
        if (parent) {
            ptr->setParent(parent);
        }
        ptr->initReflexivity();
        return ptr;
    }

    /** @brief Initialize implementation. */
    void initialize() override {
        setState(EntityState::Ready);
    }

    /** @brief Start implementation. */
    void start() override {
        setState(EntityState::Running);
        std::shared_ptr<MockEventData> event = MockEventData::create("sensorStarted");
        notifyObservers(event);
    }

    /** @brief Stop implementation. */
    void stop() override {
        setState(EntityState::Ready);
    }

    /** @brief Reset implementation. */
    void reset() override {
        setState(EntityState::Uninitialized);
    }

    /**
     * @brief Direct access for validation in tests.
     * @return The temperature node.
     */
    std::shared_ptr<NamedObject> getTemperatureNode() {
        return m_temperature;
    }

protected:
    /**
     * @brief Constructor.
     * @param name Object name.
     */
    explicit MockTemperatureSensor(const std::string& name) : ActiveEntity(name) {}

    /** @brief Initialize reflexive fields and methods. */
    void initReflexivity() {
        m_temperature = NamedObject::create("temperature", getSelf());
        REGISTER_FIELD("temperature", m_temperature);

        REGISTER_METHOD("calibrate", [this](std::shared_ptr<NamedObject> args) {
            return this->calibrate(args);
        });
    }

    /**
     * @brief Calibrate method.
     * @param args Arguments.
     * @return Result.
     */
    std::shared_ptr<NamedObject> calibrate(std::shared_ptr<NamedObject> args) {
        (void)args; // calibration ignores args for mockup
        m_calibrateCount++;
        return NamedObject::create("calibrationResult", getSelf());
    }

private:
    /** @brief Temperature field. */
    std::shared_ptr<NamedObject> m_temperature;
public:
    /** @brief Calibration count for verification. */
    int m_calibrateCount = 0;
};

TEST(TestActiveEntity, LifecycleTransitions) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<MockTemperatureSensor> sensor = MockTemperatureSensor::create("sensor1");
    
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
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<MockTemperatureSensor> sensor = MockTemperatureSensor::create("sensor1");
    std::shared_ptr<MockObserver> observer = std::make_shared<MockObserver>();
    
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
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<MockTemperatureSensor> sensor = MockTemperatureSensor::create("sensor1");
    
    std::vector<std::string> fields = sensor->listFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0], "temperature");
    
    std::shared_ptr<NamedObject> tempField = sensor->getField("temperature");
    ASSERT_NE(tempField, nullptr);
    EXPECT_EQ(tempField->getName(), "temperature");
    EXPECT_EQ(tempField, sensor->getTemperatureNode());
    
    EXPECT_EQ(sensor->getField("nonExistent"), nullptr);
}

TEST(TestActiveEntity, MethodReflexivity) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<MockTemperatureSensor> sensor = MockTemperatureSensor::create("sensor1");
    
    std::vector<std::string> methods = sensor->listMethods();
    ASSERT_EQ(methods.size(), 1);
    EXPECT_EQ(methods[0], "calibrate");
    
    std::shared_ptr<NamedObject> dummyArgs = NamedObject::create("args");
    
    EXPECT_EQ(sensor->m_calibrateCount, 0);
    
    // Execute dynamic method
    std::shared_ptr<NamedObject> res = sensor->execute("calibrate", dummyArgs);
    
    EXPECT_EQ(sensor->m_calibrateCount, 1);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->getName(), "calibrationResult");
    
    // Test non-existent method exception
    EXPECT_THROW(sensor->execute("unknownMethod", dummyArgs), std::runtime_error);
}

