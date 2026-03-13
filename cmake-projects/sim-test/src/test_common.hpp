#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

#include <Smp/IComponent.h>
#include <Smp/IModel.h>
#include <Smp/ISimulator.h>
#include <Smp/ISimpleField.h>
#include <Smp/IEntryPoint.h>
#include <sim/Simulator.hpp>
#include <iostream>
#include <string>
#include <stdexcept>

inline void LoadLibrary(sim::Simulator& simulator, const std::string& libName) {
    std::string libPath = "../../lib/lib" + libName + ".so";
    std::cout << "Loading library: " << libPath << std::endl;
    try {
        simulator.LoadLibrary(const_cast<char*>(libPath.c_str()));
    } catch (...) {
        libPath = "lib/lib" + libName + ".so";
        std::cout << "Retrying with: " << libPath << std::endl;
        try {
            simulator.LoadLibrary(const_cast<char*>(libPath.c_str()));
        } catch (...) {
            libPath = "lib" + libName + ".so";
            std::cout << "Final retry with: " << libPath << std::endl;
            simulator.LoadLibrary(const_cast<char*>(libPath.c_str()));
        }
    }
}

inline Smp::IComponent* CreateAndInitModel(sim::Simulator& simulator, Smp::Uuid uuid, const std::string& name) {
    Smp::IComponent* component = simulator.CreateInstance(uuid, name.c_str(), "Test Instance", nullptr);
    if (!component) throw std::runtime_error("Failed to create instance");

    Smp::IModel* model = dynamic_cast<Smp::IModel*>(component);
    if (!model) throw std::runtime_error("Component is not an IModel");
    simulator.AddModel(model);

    std::cout << "Lifecycle: Publishing..." << std::endl;
    simulator.Publish();
    std::cout << "Lifecycle: Configuring..." << std::endl;
    simulator.Configure();
    std::cout << "Lifecycle: Connecting..." << std::endl;
    simulator.Connect();

    return component;
}

#endif
