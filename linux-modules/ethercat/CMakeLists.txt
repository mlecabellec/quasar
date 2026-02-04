cmake_minimum_required(VERSION 3.10)
project(ethercat_module NONE)

set(ETHERCAT_VERSION "1.6.8")
set(DKMS_MODULE_NAME "ethercat")
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# --- Local Build Target ---

# Helper target to configure the project
add_custom_command(
    OUTPUT ${SOURCE_DIR}/Makefile
    COMMAND cd ${SOURCE_DIR} && ./bootstrap
    COMMAND cd ${SOURCE_DIR} && ./configure --enable-generic --disable-8139too --enable-cycles
    WORKING_DIRECTORY ${SOURCE_DIR}
    COMMENT "Configuring EtherCAT module..."
)

# Target to build the modules locally
add_custom_target(ethercat_module
    COMMAND make modules
    WORKING_DIRECTORY ${SOURCE_DIR}
    COMMENT "Building EtherCAT kernel modules..."
    DEPENDS ${SOURCE_DIR}/Makefile
)

# --- DKMS Install Target ---

# Generate dkms.conf
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/dkms.conf.in
    ${CMAKE_CURRENT_SOURCE_DIR}/dkms.conf
    @ONLY
)

# Target to install via DKMS
# Note: This requires root privileges.
add_custom_target(ethercat_dkms
    # 1. Copy source to /usr/src
    COMMAND ${CMAKE_COMMAND} -E echo "Installing EtherCAT source to /usr/src/${DKMS_MODULE_NAME}-${ETHERCAT_VERSION}..."
    COMMAND sudo cp -r ${SOURCE_DIR} /usr/src/${DKMS_MODULE_NAME}-${ETHERCAT_VERSION}
    
    # 2. Add to DKMS
    COMMAND ${CMAKE_COMMAND} -E echo "Adding to DKMS..."
    COMMAND sudo dkms add -m ${DKMS_MODULE_NAME} -v ${ETHERCAT_VERSION} || true
    
    # 3. Build via DKMS
    COMMAND ${CMAKE_COMMAND} -E echo "Building via DKMS..."
    COMMAND sudo dkms build -m ${DKMS_MODULE_NAME} -v ${ETHERCAT_VERSION}
    
    # 4. Install via DKMS
    COMMAND ${CMAKE_COMMAND} -E echo "Installing via DKMS..."
    COMMAND sudo dkms install -m ${DKMS_MODULE_NAME} -v ${ETHERCAT_VERSION}
    
    COMMENT "Installing EtherCAT module via DKMS (requires sudo)"
    VERBATIM
)
