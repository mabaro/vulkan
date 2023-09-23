#!/bin/sh
CMAKE_LOGLEVEL=--log-level=VERBOSE #WARNING ERROR STATUS VERBOSE TRACE
cmake -S . -B build ${CMAKE_LOGLEVEL}

CMAKE_PARALLEL_JOBS="--parallel 8"
#CMAKE_BUILD_VERBOSE=--verbose
cmake --build build --target $1 ${CMAKE_PARALLEL_JOBS} ${CMAKE_BUILD_VERBOSE}
