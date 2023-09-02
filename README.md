# Cogwheel's NES Starter Kit

This is an example project using llvm-mos targeting the MMC1 mapper. It's very much work-in-progress. There will be more to come.

## Prerequisites

- llvm-mos
  - This needs to be findable by CMake. Currently I just put llvm-mos/bin in my path; need to find something more robust
- CMake
- Python 3
  - pillow

## Building

To build the project simply run:

```sh
./build.sh
```

This will configure CMake and compile the ROM to ./build/nes-starter-kit.nes.

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