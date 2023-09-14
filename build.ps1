# TODO: Visual Studio - is there a way to use 3rd-party LLVM toolkit?

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE="$($env:BUILD_TYPE ?? "RelWithDebInfo")" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja -C build @args
