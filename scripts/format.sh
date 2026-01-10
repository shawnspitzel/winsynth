#!/bin/bash
echo "Formatting C++ source files..."
find . -type f \( -name "*.cpp" -o -name "*.h" \) \
    ! -path "./build/*" \
    ! -path "./build-*/*" \
    ! -path "./Debug/*" \
    ! -path "./Release/*" \
    ! -path "./x64/*" \
    ! -path "./external/*" \
    -exec clang-format -i {} \;
echo "Files formatted:"
find . -type f \( -name "*.cpp" -o -name "*.h" \) \
    ! -path "./build/*" \
    ! -path "./build-*/*" \
    ! -path "./Debug/*" \
    ! -path "./Release/*" \
    ! -path "./x64/*" \
    ! -path "./external/*"
