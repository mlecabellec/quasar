#include "test_common.hpp"
#include <iostream>
#include <vector>

int main() {
    try {
        sim::Simulator simulator;
        LoadLibrary(simulator, "sample-delayed-simple");

        Smp::Uuid delayedUuid = {0xAAA11111, 0x1111, 0x1111, 0x1111, 0x111111111111};
        Smp::IComponent* component = CreateAndInitModel(simulator, delayedUuid, "DelayedSimple");

        Smp::ISimpleField* inInt32 = dynamic_cast<Smp::ISimpleField*>(component->GetField("Input_Int32"));
        Smp::ISimpleField* outInt32 = dynamic_cast<Smp::ISimpleField*>(component->GetField("Output_Int32"));
        Smp::ISimpleField* inFloat64 = dynamic_cast<Smp::ISimpleField*>(component->GetField("Input_Float64"));
        Smp::ISimpleField* outFloat64 = dynamic_cast<Smp::ISimpleField*>(component->GetField("Output_Float64"));
        Smp::IEntryPoint* ep = dynamic_cast<Smp::IEntryPoint*>(component->GetChild("Execute"));

        if (!inInt32 || !outInt32 || !inFloat64 || !outFloat64 || !ep) {
            std::cerr << "Fields or entry point not found!" << std::endl;
            return 1;
        }

        std::vector<int> inputs = {10, 20, 30, 40, 50, 60};
        std::vector<int> expected_outputs = {0, 0, 10, 20, 30, 40}; // 2-step delay

        for (size_t i = 0; i < inputs.size(); ++i) {
            inInt32->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32, inputs[i]));
            inFloat64->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Float64, (double)inputs[i]));
            
            ep->Execute();

            int outValInt = (int)outInt32->GetValue();
            double outValFloat = (double)outFloat64->GetValue();
            
            std::cout << "Iteration " << i << ": Input=" << inputs[i] << ", OutputInt=" << outValInt << ", OutputFloat=" << outValFloat << std::endl;

            if (outValInt != expected_outputs[i] || (int)outValFloat != expected_outputs[i]) {
                std::cerr << "Test failed at iteration " << i << ". Expected " << expected_outputs[i] << " but got " << outValInt << std::endl;
                return 1;
            }
        }

        std::cout << "Lifecycle: Exiting..." << std::endl;
        simulator.Exit();
        std::cout << "DelayedSimple Test Passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
