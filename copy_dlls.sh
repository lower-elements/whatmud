#!/bin/bash

# Check if the executable path is provided as an argument
if [ -z "$1" ]; then
    echo "Usage: $0 <path-to-executable>"
    exit 1
fi

# Path to the executable
EXECUTABLE="$1"

# Directory containing the executable
EXECUTABLE_DIR=$(dirname "${EXECUTABLE}")

# Run ldd command and extract DLLs
DLLS=$(ldd "${EXECUTABLE}" | grep -oP '(?<=\=\> ).*(?=\s\(0x)')
# cp dlls/*.dll "${EXECUTABLE_DIR}"
# Loop through each DLL
for DLL in ${DLLS}; do
    # Check if the DLL is not in a standard Windows installation
    if [[ ! "${DLL}" =~ ^/c/WINDOWS/ ]]; then
        # Copy the DLL next to the executable
        cp "${DLL}" "${EXECUTABLE_DIR}"
        echo "Copied ${DLL} to ${EXECUTABLE_DIR}"
    fi
done
