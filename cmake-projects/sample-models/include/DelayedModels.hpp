#ifndef DELAYED_MODELS_HPP
#define DELAYED_MODELS_HPP

#include "common/SmpModel.hpp"
#include <Smp/IEntryPoint.h>
#include <vector>

namespace sample {

/**
 * @brief Model with one input and one output per simple type (except String8).
 * @details Implements 2 steps of lateness for all values.
 */
class DelayedSimpleModel : public SmpModel {
public:
    DelayedSimpleModel(Smp::String8 name, Smp::String8 description, Smp::IObject* parent);
    virtual ~DelayedSimpleModel() noexcept override = default;

    void Publish(Smp::IPublication* receiver) override;
    const Smp::Uuid& GetUuid() const override;
    void Execute();

private:
    struct SimpleValues {
        Smp::Bool b;
        Smp::Char8 c8;
        Smp::Int8 i8;
        Smp::Int16 i16;
        Smp::Int32 i32;
        Smp::Int64 i64;
        Smp::UInt8 u8;
        Smp::UInt16 u16;
        Smp::UInt32 u32;
        Smp::UInt64 u64;
        Smp::Float32 f32;
        Smp::Float64 f64;
        Smp::Duration dur;
        Smp::DateTime dt;
    };

    SimpleValues _input;
    SimpleValues _mem1;
    SimpleValues _mem2;
    SimpleValues _output;

    class ExecuteEntryPoint : public virtual Smp::IEntryPoint {
    public:
        ExecuteEntryPoint(DelayedSimpleModel* model) : _model(model) {}
        Smp::String8 GetName() const override { return "Execute"; }
        Smp::String8 GetDescription() const override { return "Execute DelayedSimpleModel"; }
        Smp::IObject* GetParent() const override { return _model; }
        Smp::IObject* GetChild(Smp::String8 name) const override { return nullptr; }
        void Execute() const override { _model->Execute(); }
    private:
        DelayedSimpleModel* _model;
    };

    ExecuteEntryPoint _executeEntryPoint;
    static const Smp::Uuid _uuid;
};

/**
 * @brief Model with an Smp::IArrayField of Smp::Int8.
 * @details Implements 2 steps of lateness for the entire array.
 */
class DelayedArrayModel : public SmpModel {
public:
    DelayedArrayModel(Smp::String8 name, Smp::String8 description, Smp::IObject* parent);
    virtual ~DelayedArrayModel() noexcept override = default;

    void Publish(Smp::IPublication* receiver) override;
    const Smp::Uuid& GetUuid() const override;
    void Execute();

private:
    static constexpr size_t ARRAY_SIZE = 10;
    Smp::Int8 _input[ARRAY_SIZE];
    Smp::Int8 _mem1[ARRAY_SIZE];
    Smp::Int8 _mem2[ARRAY_SIZE];
    Smp::Int8 _output[ARRAY_SIZE];

    class ExecuteEntryPoint : public virtual Smp::IEntryPoint {
    public:
        ExecuteEntryPoint(DelayedArrayModel* model) : _model(model) {}
        Smp::String8 GetName() const override { return "Execute"; }
        Smp::String8 GetDescription() const override { return "Execute DelayedArrayModel"; }
        Smp::IObject* GetParent() const override { return _model; }
        Smp::IObject* GetChild(Smp::String8 name) const override { return nullptr; }
        void Execute() const override { _model->Execute(); }
    private:
        DelayedArrayModel* _model;
    };

    ExecuteEntryPoint _executeEntryPoint;
    static const Smp::Uuid _uuid;
};

} // namespace sample

#endif // DELAYED_MODELS_HPP
