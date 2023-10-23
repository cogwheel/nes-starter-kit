# Cogwheel's NES Starter Kit

This is an example project using llvm-mos targeting the MMC1 mapper. It's very much work-in-progress. There will be more to come.

![nes-starter-kit](https://github.com/cogwheel/nes-starter-kit/assets/2105215/e4b32ece-9fce-4696-8c72-0c00c4c274be)

## Prerequisites

Note: I've only tested this on Ubuntu 22.04 (bash) and Windows 11 (powershell). YMMV.

- [llvm-mos SDK](https://github.com/llvm-mos/llvm-mos-sdk#download)
  - This needs to be findable by CMake. You can set CMAKE_PREFIX_PATH to `path/to/llvm-mos` or you can add `path/to/llvm-mos/bin` to your PATH

The rest of these should be available in your PATH:

- [CMake](https://cmake.org/download/)
- [Ninja](https://ninja-build.org/)
- [Python 3](https://www.python.org/downloads/)
  - [Pillow](https://pillow.readthedocs.io/en/stable/) - for converting PNG to CHR

## Building

If you're familiar with CMake, have a quick glance at [`build.sh`](build.sh) and decide whether you want to use it. Ninja and "Unix Makefiles" generators are known to work. Otherwise...

To build the project, simply run `build.sh` ('nix) or `build.ps1` (Windows). This will configure CMake and compile the ROM to ./build/nes-starter-kit.nes.

The build script will pass arguments to `ninja` so you can do, e.g.:

```sh
./build.sh clean
```

You can also set the build type like:

```sh
BUILD_TYPE=Release ./build.sh
```

The default is RelWithDebInfo.

### Q&D CMake intro

For maximum productivity you'll want to get familiar with the way CMake works. Here's a quick overview grafted [a conversation in llvm-mos discord](https://discord.com/channels/1058149494107148399/1058150812691476593/1147638891894034603):

CMake lets you generate build-system scripts (visual studio projects, makefiles, Ninja build scripts, etc.) appropriate for whatever system you're developing on. Then you use those tools to actually do the build. This project uses `Ninja` by default.

CMakeLists.txt files are (mostly) declarative configuration for various targets (binaries, libraries, and such), their relationships, dependencies, and the way they get built. When you first run cmake, all of the results of processing CMakeLists.txt get baked into the build directory.

Running cmake again in this directory will only update anything that has changed since it originally ran (new source code files, changes made to CMakeLists.txt itself, etc). A lot of the core configuration (build type, SDK location, etc.) remains fixed. For example, passing a different `CMAKE_BUILD_TYPE` will have no effect. If you want to change the configuration, you need to remove the CMakeCache.txt and CMakeFiles dirs from inside build/ dir, remove the build/ dir itself, or make a new config-specific build dir.

The usual workflow is to make a `build` directory, `cd` into it, then do `cmake [options] ..`. This uses the CMakeLists.txt of the parent directory to do the configuration, and the results of the configuration are saved to the current `build` dircetory. Then you can issue your normal build system commands from there (e.g. `ninja run`).

To configure for a different build type, you pass -DCMAKE_BUILD_TYPE=[something]. `something` can be one of `Debug`, `Release`, `RelWithDebInfo`, and `MinSizeRel`. This project's `.gitignore` filters out any directory that starts with `build`. So you can easily have several build directories each with a different configuration.

## Running

You can load the ROM manually from the `build/` directory.

There is also a `run` target which you can use with `ninja` or the build script:

```powershell
.\build.ps1 run
```

By default this will attempt to launch the ROM with your system's default app associated with `.nes` files. You can set the `EMULATOR` environment variable to use a specific program.

## Customizing

You can set the name of the project in [CMakeLists.txt](CMakeLists.txt). Look for the line that starts `project(nes-starter-kit...`

Any new files you add to `src/`, including any subdirectories, will be compiled. The project is currently set to support C++ and Assembly, but you can customize this by changing the `project` declaration mentioned above.

PNG files added to [`chr/`](chr/) will be automatically converted to .chr files during the build. See [`chr/make_chr.py`](chr/make_chr.py) for info on how to make compatible images. You will also need to add a new `.incbin` directive in [`chr/chr.s`](chr/chr.s) to incorporate them into the ROM.

### VS Code

llvm-mos includes a custom clangd server that knows how to handle llvm-mos-specific extensions, system include directories, etc. To set this up, rename `.vscode/example_settings.json` to `.vscode/settings.json` or follow the [instructions on the llvm-mos wiki](https://llvm-mos.org/wiki/Clangd).

Note: the build scripts above already include `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.

## TODOs

I'd like this project to demonstrate all the core aspects of NES development. Here's what I've currently got planned:

### Near term

- [X] Sprite animation
- [X] Controller input
- [X] CHR bank switching
- [ ] Scrolling

### Eventually

- [ ] Audio
- [ ] PRG bank switching
- [ ] Generate Visual Studio projects with cmake
