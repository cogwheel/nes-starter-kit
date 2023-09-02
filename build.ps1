cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="$($env:BUILD_TYPE ?? "RelWithDebInfo")"
make -C build @args
