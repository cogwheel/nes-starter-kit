cmake_minimum_required(VERSION 3.18)

# We want at least -Os for everything, even debug. Otherwise the emitted assembly
# can easily overflow code sections

# TODO: test which optimization level works best in release mode
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -flto")
set(CMAKE_CXX_FLAGS_DEBUG "-Os")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG -Os -flto")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -flto")
# Assembler complains when -DNDEBUG is here
set(CMAKE_ASM_FLAGS_RELEASE "-O3 -flto")
set(CMAKE_ASM_FLAGS_DEBUG "-Os")
set(CMAKE_ASM_FLAGS_MINSIZEREL "-Os -flto")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "-O3 -flto")

# Configure the project to use llvm-mos targeting the NES with the MMC1 mapper
set(LLVM_MOS_PLATFORM nes-mmc1)
find_package(llvm-mos-sdk REQUIRED)

# Set the project name and source types.
# The ROM image will be named ${CMAKE_PROJECT_NAME}.nes
project(nes-starter-kit CXX ASM)

add_compile_options(
    -g -gdwarf-4           # We want debug info generated for all builds
    -Wall -Wextra -Werror  # Compiler's got your back
    -std=c++23             # We _can_ have nice things!
)

add_link_options(
    -g -gdwarf-4 -Wall -Wextra -Werror # Same goes for the linker

    # -Map generates a file with information about how the final binary's memory
    # is laid out. This will eventually be used to show the remaining space in
    # CHR-ROM, PRG-ROM, PRG-RAM, System RAM, and ZEROPAGE

    # --lto-whole-program-visibility helps ensure clang does as many
    # optimizations as possible
    -Wl,-Map,${CMAKE_PROJECT_NAME}.map,--lto-whole-program-visibility
)

# This creates the `config` object containing symbols used by the linker to lay
# out the ROM file. See config/* for more info
add_subdirectory(config)

# This creates the `chr_data` object with the contents of CHR-ROM. See
# chr/* for more info.
add_subdirectory(chr)

# Gather all project source and header files to compile into the final ROM image
set(SRC ${CMAKE_SOURCE_DIR}/src)
file(GLOB_RECURSE SOURCES ${SRC}/*.cpp ${SRC}/*.s)
file(GLOB_RECURSE HEADERS ${SRC}/*.hpp ${SRC}/*.inc)

set (ROM ${CMAKE_PROJECT_NAME}.nes)
add_executable(${ROM}
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(${ROM}
    # First-party, as described above
    config
    chr_data

    # Second-party, included in llvm-mos
    neslib
    nesdoug
)

# TODO: add helper targets for:
#    - emitting assembly code for each module and/or the whole binary
#    - launching the built ROM on your emulator of choice
