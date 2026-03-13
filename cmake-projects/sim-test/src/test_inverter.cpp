#include "test_common.hpp"
#include <iostream>

int main() {
    try {
        sim::Simulator simulator;
        LoadLibrary(simulator, "sample-inverter");

        Smp::Uuid inverterUuid = {0x12345678, 0x1234, 0x5678, 0x1234, 0x567812345678};
        Smp::IComponent* component = CreateAndInitModel(simulator, inverterUuid, "Inverter");

        Smp::ISimpleField* input = dynamic_cast<Smp::ISimpleField*>(component->GetField("Input"));
        Smp::ISimpleField* output = dynamic_cast<Smp::ISimpleField*>(component->GetField("Output"));
        Smp::IEntryPoint* ep = dynamic_cast<Smp::IEntryPoint*>(component->GetChild("Execute"));

        if (!input || !output || !ep) {
            std::cerr << "Fields or entry point not found!" << std::endl;
            return 1;
        }

        for (int i = 0; i < 5; ++i) {
            bool testVal = (i % 2 == 0);
            input->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, testVal));
            ep->Execute();
            bool result = (bool)output->GetValue();
            std::cout << "Iteration " << i << ": Input=" << testVal << ", Output=" << result << std::endl;
            if (result == testVal) {
                std::cerr << "Test failed at iteration " << i << std::endl;
                return 1;
            }
        }

        std::cout << "Lifecycle: Exiting..." << std::endl;
        simulator.Exit();
        std::cout << "Inverter Test Passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
