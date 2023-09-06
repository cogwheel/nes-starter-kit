# Cogwheel's NES Starter Kit

This is an example project using llvm-mos targeting the MMC1 mapper. It's very much work-in-progress. There will be more to come.

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

If you're familiar with CMake, have a quick glance at [`build.sh`](build.sh) and decide if you want to use it or not. Ninja and "Unix Makefiles" generators are known to work. Otherwise...

To build the project simply run `build.sh` ('nix) or `build.ps1` (Windows). This will configure CMake and compile the ROM to ./build/nes-starter-kit.nes.

The build script will pass arguments to `make` so you can do, e.g.:

```sh
./build.sh clean
```

You can also set the build type like:

```sh
BUILD_TYPE=Release ./build.sh
```

The default is RelWithDebInfo.

## Running

You can load the ROM manually from the `build/` directory.

There is also a `run` target which you can use with `make` or the build script:

```powershell
.\build.ps1 run
```

By default this will attempt to launch the ROM with your system's default app associated with `.nes` files. You can set the `EMULATOR` environment variable to use a specific program.

## Customizing

You can set the name of the project in [CMakeLists.txt](CMakeLists.txt). Look for the line that starts `project(nes-starter-kit...`

Any new files you add to `src/`, including any subdirectories, will be compiled. The project is currently set to support C++ and Assembly, but you can customize this by changing the `project` declaration mentioned above.

PNG files added to [`chr/`](chr/) will be automatically converted to .chr files during the build. See [`chr/make_chr.py`](chr/make_chr.py) for info on how to make compatible images. You will also need to add a new `.incbin` directive in [`chr/chr.s`](chr/chr.s) to incorporate them into the ROM.

### VS Code

If you copy `example_compile_flags.txt` to `compile_flags.txt` and change the llvm-mos path to suit your environment, then the `clangd` VS Code extension should give you full intellisense support.

## TODOs

I'd like this project to demonstrate all the core aspects of NES development. Here's what I've currently got planned:

### Near term

- [X] Sprite animation
- [X] Controller input
- [ ] CHR bank switching
- [ ] Scrolling

### Eventually

- [ ] Audio
- [ ] PRG bank switching
- [ ] Other build systems besides Makefiles
