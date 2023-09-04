#!/usr/bin/env sh
set -e

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE:-RelWithDebInfo}
ninja -C build $@
