#!/bin/bash

# List of resoem executables to monitor
RESOEM_EXECUTABLES=(
    "test_broadcast_read"
    "test_enumeration"
    "test_coe_upload"
    "test_soe_framing"
    "test_topology"
    "test_dc_calc"
    "test_redundancy"
    "test_diagnostics"
    "simple_ng"
    "slaveinfo"
    "testmaster100"
    "eepromtool"
    "eni_test"
    "eoe_test"
    "firm_update"
)

# Get the directory of the script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BIN_DIR="$SCRIPT_DIR/build/bin"

# Check if script is run as root
if [[ $EUID -ne 0 ]]; then
   echo "Error: This script must be run as root."
   echo "Usage: sudo $0"
   exit 1
fi

# Verify dependencies
if ! command -v getcap &> /dev/null || ! command -v setcap &> /dev/null; then
    echo "Error: getcap or setcap not found. Please install libcap2-bin (apt install libcap2-bin)."
    exit 1
fi

echo "Starting resoem capability manager..."
echo "Monitoring directory: $BIN_DIR"

while true; do
    if [[ ! -d "$BIN_DIR" ]]; then
        sleep 1
        continue
    fi

    for exe in "${RESOEM_EXECUTABLES[@]}"; do
        EXE_PATH="$BIN_DIR/$exe"
        if [[ -f "$EXE_PATH" ]]; then
            # Verify it is an ELF executable (not a script or empty file)
            if ! file "$EXE_PATH" | grep -q "ELF"; then
                continue
            fi

            # Check if cap_net_raw=ep is already set.
            # NOTE: setcap 'cap_net_raw+ep' results in getcap output showing 'cap_net_raw=ep'
            # We use grep -F for fixed string match and -q for silence.
            if ! getcap "$EXE_PATH" 2>/dev/null | grep -q "cap_net_raw=ep"; then
                echo "[$(date +'%Y-%m-%d %H:%M:%S')] Setting cap_net_raw+ep for $exe"
                setcap 'cap_net_raw+ep' "$EXE_PATH" 2>/dev/null || echo "Failed to set capability for $exe"
            fi
        fi
    done
    sleep 1
done
