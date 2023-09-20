cmake_minimum_required(VERSION 3.18)

# Configure the project to use llvm-mos targeting the NES with the MMC1 mapper
set(LLVM_MOS_PLATFORM nes-mmc1)
find_package(llvm-mos-sdk REQUIRED)

# Set the project name and source types.
# The ROM image will be named ${CMAKE_PROJECT_NAME}.nes
project(nes-starter-kit C CXX ASM)

# Remove a bunch of default flags and set -flto for non-debug builds
include(set_build_flags.cmake)

# We _can_ have nice things!
set (CMAKE_CXX_FLAGS "-std=c++23")
set (CMAKE_C_FLAGS "-std=c23")

# Set compile flags for all objects
add_compile_options(
    -Os             # Use -Os for everything or generated code will be too large
    -g -gdwarf-4    # We want debug info generated for all builds; it doesn't go into the ROM
    -Wall -Wextra   # Compiler's got your back; -Werror is enabled for Release and MinSizeRel
)

# `add_compile_options` adds -MD, -MT, and -MF to the Ninja config but they're
# not supported by llvm assembler
set(CMAKE_DEPFILE_FLAGS_ASM)

# Set linker flags for all binaries
add_link_options(
    -g -gdwarf-4 -Wall -Wextra -Werror # Same goes for the linker
)

# This creates the `chr_data` object with the contents of CHR-ROM. See
# chr/* for more info.
add_subdirectory(chr)

# Gather all project source and header files to compile into the final ROM image
set(SRC ${CMAKE_SOURCE_DIR}/src)
file(GLOB_RECURSE SOURCES ${SRC}/*.c ${SRC}/*.cpp ${SRC}/*.s)
file(GLOB_RECURSE HEADERS ${SRC}/*.h ${SRC}/*.hpp ${SRC}/*.inc)

set (ROM ${CMAKE_PROJECT_NAME}.nes)
add_executable(${ROM}
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${ROM} PRIVATE
    ${SRC}
)

# Load NES-specific linker scripts
target_link_options(${ROM} PRIVATE

    # -Map generates a file with information about how the final binary's memory
    # is laid out. This will eventually be used to show the remaining space in
    # CHR-ROM, PRG-ROM, PRG-RAM, System RAM, and ZEROPAGE

    # --lto-whole-program-visibility helps ensure clang does as many
    # optimizations as possible
    -Wl,-Map,${CMAKE_PROJECT_NAME}.map,--lto-whole-program-visibility

    # `common.ld` is required by all NES targets. It sets up basic sections like
    # `prg_rom` and `chr_rom` among other bookkeeping
    -Tcommon.ld

    # `c-in-prg-ram.ld` makes it so that any variables that must be stored in
    # RAM will use the PRG-RAM (aka Work RAM, WRAM) chip on the cartridge
    # instead of the system RAM.
    #
    # This requires setting __prg_ram_size to a non-zero value in config.s
    # (TODO: check if it's expecting 8KiB increments?)
    -Tc-in-prg-ram-0.ld

    # `prg-rom-fixed` uses the default 32 KiB of PRG-ROM. TODO: PRG-ROM banking
    -Tprg-rom-fixed.ld
)

target_link_libraries(${ROM}
    # First-party, as described above
    chr_data

    # Second-party, included in llvm-mos
    neslib
    nesdoug
)

set(RUNNER ${CMAKE_SOURCE_DIR}/run_emulator.cmake)
set(ROMFILE ${CMAKE_BINARY_DIR}/${ROM})

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -P ${RUNNER} -- ${ROMFILE}
    COMMENT "Running ${ROMFILE}"
    DEPENDS ${ROM} ${RUNNER}
)

# TODO: add helper target for emitting assembly code for each module and/or the
# whole binary
