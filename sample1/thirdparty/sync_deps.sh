#!/bin/bash
THIRDPARTY_DIR=$(pwd)

GLSLC=true
DXC=false
IMGUI=true
SDL2=true

if [ "$SDL2" = true ]; then
    git clone -b SDL2 https://github.com/libsdl-org/SDL.git $THIRDPARTY_DIR/SDL2
        TRACEPARAM=--trace-source="CMakeLists.txt"
        GENERATOR=-GNinja
        EXTRAS=-DSDL_SHARED:String=OFF
        cmake -S . -B build --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE ${EXTRAS} ${GENERATOR}
        cmake --build build --config Release --target dxc .
    popd
fi


if [ "$DXC" = true ]; then
    tar xvf dxc-artifacts.tar.gz
    # cp dxc-artifacts/bin/dxc ../tools/linux/bin
    # cp dxc-artifacts/lib/*.so ../tools/linux/bin
    # ### change search path for DXC dynamic libraries
    # chrpath -r. ../tools/linux/bin/dxc
    ############
    # git clone --recurse-submodules -b master https://github.com/google/DirectXShaderCompiler.git $THIRDPARTY_DIR/dxc
    # pushd "dxc"
    #     TRACEPARAM=--trace-source="CMakeLists.txt"
    #     # EXTRAS=$(cat ../utils/cmake-predefined-config-params)
    #     EXTRAS=-C cmake/caches/PredefinedParams.cmake
    #     GENERATOR=-GNinja
    #     cmake -S . -B build --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE ${EXTRAS} ${GENERATOR}
    #     cmake --build build --config Release --target dxc .
    # popd
fi


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