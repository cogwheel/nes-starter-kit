; Configuration symbols from https://llvm-mos.org/wiki/NES_targets#Symbol_Configuration

.global __prg_rom_size, __chr_rom_size, __prg_ram_size
.global __mirroring, __four_screen

; Kilobytes
__prg_rom_size = 256
__chr_rom_size = 128
__prg_ram_size = 8

; Flags
__mirroring = 0   ; vertical, but unused for now