#!/usr/bin/env sh
set -e

# TODO: support other build systems besides makefile
#   - Ninja: unsupported assembler arguments
#   - Visual Studio: is there a way to use 3rd-party LLVM toolkit?
cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=${BUILD_TYPE:-RelWithDebInfo}

make -C build -j $@
