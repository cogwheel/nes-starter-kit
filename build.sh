#!/usr/bin/env sh
set -e

cmake -B build -DCMAKE_BUILD_TYPE=${BUILD_TYPE:-RelWithDebInfo}
make -C build -j $@
