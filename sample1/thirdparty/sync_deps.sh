#!/bin/sh
THIRDPARTY_DIR=$(pwd)

GLSLC=true
IMGUI=true

if [ "$GLSLC" = true ]; then
    git clone https://github.com/google/shaderc $THIRDPARTY_DIR/shaderc
    cd $THIRDPARTY_DIR/shaderc
    cd utils
    # ./git-sync-deps
    cd ..
    TRACEPARAM=--trace-source="CMakeLists.txt"
    cmake -S . -B build --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DSHADERC_SKIP_TESTS:BOOL=TRUE -DSHADERC_SKIP_INSTALL:BOOL=TRUE -DSHADERC_SKIP_EXAMPLES:BOOL=TRUE -DSHADERC_SKIP_COPYRIGHT_CHECK:BOOL=TRUE -G "Unix Makefiles"
    cmake --build build --config Release --target glslc .
    cp build/glslc/glslc ../tools/linux/bin/
fi

if [ "$IMGUI" = true ]; then
    git clone https://github.com/ocornut/imgui.git $THIRDPARTY_DIR/imgui
fi