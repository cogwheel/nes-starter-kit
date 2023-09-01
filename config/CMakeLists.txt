# Assemble `config.s` into an object so the linker has access to the
# configuration symbols
add_library(config OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/config.s)

# Export linker options to bring in NES-specific configuration scripts
target_link_options(config PUBLIC
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

    ## `prg-rom-32.ld` sets up 32 KiB of PRG-ROM without banking. The m
    ## of ROM is made of two swappable 8-KiB banks, and the upper half is made up
    ## of a 16-KiB fixed bank.
    ## 
    ## This requires setting __prg_rom_size > 32 KiB (TODO: check if it's
    ## expecting 8KiB increments)
    -Tprg-rom-32.ld
)
