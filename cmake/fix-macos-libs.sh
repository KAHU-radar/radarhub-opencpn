#!/bin/bash

# Adjust absolute paths to wxWidgets libraries to match their
# location ot the target MacOS system where they are installed
# as part of OpenCPN

readonly RUNTIME_PATH="@executable_path/../Frameworks/"

echo "Fixing maxos library paths in $(pwd)..."
find . -name '*.dylib' | while read plugin; do
    echo "    Fixing library $plugin..."
    for lib in $(otool -L "$plugin" | awk ' /wx/ {print $1}'); do
        libdir=${lib%/*}
        if [ "$libdir" = "$lib" ]; then
            continue
        elif [ "$libdir" != "$RUNTIME_PATH" ]; then
            runtime_lib=$(echo $lib | sed "s|$libdir|$RUNTIME_PATH|")
            install_name_tool -change "$lib" "$runtime_lib" "$plugin"
        fi
    done
    echo "fix-macos-libs.sh: Revised library paths:"
    otool -L "$plugin" | grep wx
done
