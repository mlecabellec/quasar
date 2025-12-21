# Quasar


# Project description

Quasar is a project of global industrial automation solution.

Quasar shall:
- Provide required middleware (including kernel modules, userland libraries, and userland applications) allowing to interact with common industrial hardware and standard industrial protocols.
- Provide API oriented interface for application development.
- Provide required environnements and integration for hardware in the loop simulation (HILS). Provided environments shall allow to simulate common industrial hardware and standard industrial protocols. It shall be possible to run applications and automations seemlessly in siulated or real environment.
- Provide API and environements allowing rapid develoment of automations and applications.
- Have a robust design compatible with DOE-178A class C.


Quasar should:
- Have a secure design compatible with IEC 62443-4-2.
- Have a reliable design compatible with IEC 61508-3.
- Have a maintainable design compatible with IEC 61508-3.

# Purpose of this file

This file is intended to collect and index all the development notes and information about the Quasar project. It is also intended to be used as a reference for the coding agents.


# General structure

- `doc/` : This directory
- 'doc/architecture.md' : Architecture overview
- 'doc/development.md' : Development notes
- 'doc/TODO.md' : TODO list. The TODO list is intended to be used as a reference for the coding agents regarding short term tasks. 
- 'doc/README.md' : This file
- 'doc/architecture/' : Architecture details. This directory is intended to contain architecture details for the Quasar project. It will also contain architectural constraints that shall be adhered to by the coding agents.
- 'doc/features/' : Feature details. This directory is intended to contain feature details for the Quasar project. It will also contain feature that shall be implemented by the coding agents.
- 'doc/testDefs/' : Test definitions. This directory is intended to contain test definitions for the Quasar project. It will also contain test cases that shall be implemented by the coding agents. Generated code shall pass all tests.

# Code structure

This repository will contain multiple projects and modules with multiple technologies.

'cmake-projects/' : This directory contains the CMake projects. Projects could be recursively nested, each project will have its own CMakeLists.txt file.

'maven-projects/' : This directory contains the Maven projects. Projects could be recursively nested, each project will have its own pom.xml file. Maven projects will also have a CMakeLists.txt allowing them to be built with CMake.

'linux-modules/' : This directory contains the Linux kernel modules. Projects could be recursively nested, each project will have its own Makefile.

'ext-projects/' : This directory contains the external projects. Projects could be recursively nested, each project will have its own build system but, for each project, a CMakeLists.txt will be provided allowing it to be built with CMake. Each external project shall have a LICENSE file corresponding to the license of the project and a README.md file with information about its origin (including a link to the original repository).

