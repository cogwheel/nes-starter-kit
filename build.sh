#!/usr/bin/env sh
set -e

# TODO: support other build systems besides makefile
cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=${BUILD_TYPE:-RelWithDebInfo}

make -C build -j $@
