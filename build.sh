#!/bin/bash
# Fast build script for pokeemerald
# Automatically uses all available CPU cores for parallel compilation

# Detect number of CPU cores
if command -v nproc &> /dev/null; then
    # Linux/WSL
    CORES=$(nproc)
elif command -v sysctl &> /dev/null; then
    # macOS
    CORES=$(sysctl -n hw.ncpu)
else
    # Fallback
    CORES=4
fi

echo "Building with $CORES parallel jobs..."
make -j$CORES "$@"
