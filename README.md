# Cogwheel's NES Starter Kit

This is an example project using llvm-mos targeting the MMC1 mapper. It's very much work-in-progress. There will be more to come.

## Prerequisites

Note: this has only been tested on Ubuntu 22.04 (bash) and Windows 11 (powershell). YMMV.

- [llvm-mos SDK](https://github.com/llvm-mos/llvm-mos-sdk#download)
  - This needs to be findable by CMake. You can set CMAKE_PREFIX_PATH to `path/to/llvm-mos` or you can add `path/to/llvm-mos/bin` to your PATH
- CMake
- GNU Make (or equivalent)
- Python 3
  - pillow - for converting PNG to CHR

## Building

To build the project simply run `build.sh` (unix) or `build.ps1` (Windows). This will configure CMake and compile the ROM to ./build/nes-starter-kit.nes.

The build script will pass arguments to `make` so you can do, e.g.:

```sh
./build.sh clean
```

You can also set the build type like:

```sh
BUILD_TYPE=Release ./build.sh
```

The default is RelWithDebInfo.

Of course, you can always just issue the `cmake` and `make` commands yourself. :)

## TODOs

I'd like this project to demonstrate all the core aspects of NES development. Here's what I've currently got planned:

### Near term

- [ ] Sprite animation
- [ ] Controller input
- [ ] CHR bank switching
- [ ] Scrolling

### Eventually

- [ ] Audio
- [ ] PRG bank switching
- [ ] Other build systems besides Makefiles
