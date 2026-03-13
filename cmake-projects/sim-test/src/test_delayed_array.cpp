#include "test_common.hpp"
#include <Smp/ISimpleArrayField.h>
#include <iostream>
#include <vector>

int main() {
    try {
        sim::Simulator simulator;
        LoadLibrary(simulator, "sample-delayed-array");

        Smp::Uuid delayedUuid = {0xAAA22222, 0x2222, 0x2222, 0x2222, 0x222222222222};
        Smp::IComponent* component = CreateAndInitModel(simulator, delayedUuid, "DelayedArray");

        Smp::ISimpleArrayField* inArray = dynamic_cast<Smp::ISimpleArrayField*>(component->GetField("InputArray"));
        Smp::ISimpleArrayField* outArray = dynamic_cast<Smp::ISimpleArrayField*>(component->GetField("OutputArray"));
        Smp::IEntryPoint* ep = dynamic_cast<Smp::IEntryPoint*>(component->GetChild("Execute"));

        if (!inArray || !outArray || !ep) {
            std::cerr << "Fields or entry point not found!" << std::endl;
            return 1;
        }

        size_t size = inArray->GetSize();
        std::vector<Smp::Int8> expected_mem1(size, 0);
        std::vector<Smp::Int8> expected_mem2(size, 0);

        for (int i = 0; i < 5; ++i) {
            std::vector<Smp::AnySimple> input_anys(size);
            std::vector<Smp::Int8> current_input(size);
            for (size_t j = 0; j < size; ++j) {
                current_input[j] = (Smp::Int8)(i * 10 + j);
                input_anys[j] = Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int8, current_input[j]);
            }
            
            inArray->SetValues(size, input_anys.data());
            
            ep->Execute();

            std::vector<Smp::AnySimple> output_anys(size);
            outArray->GetValues(size, output_anys.data());

            std::cout << "Iteration " << i << ": Testing 2-step delay." << std::endl;

            for (size_t j = 0; j < size; ++j) {
                Smp::Int8 outVal = (Smp::Int8)output_anys[j];
                if (outVal != expected_mem2[j]) {
                    std::cerr << "Test failed at iteration " << i << " element " << j 
                              << ". Expected " << (int)expected_mem2[j] << " but got " << (int)outVal << std::endl;
                    return 1;
                }
            }

            expected_mem2 = expected_mem1;
            expected_mem1 = current_input;
        }

        std::cout << "Lifecycle: Exiting..." << std::endl;
        simulator.Exit();
        std::cout << "DelayedArray Test Passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
