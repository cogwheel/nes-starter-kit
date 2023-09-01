; The `chr_rom_0` section represents the first 4KiB bank of CHR data that can be
; mapped into PPU memory by the MMC1 mapper
.section .chr_rom_0,"a"

    ; background.png contains the characters used for writing text in the
    ; example. It will be converted to `background.chr` by the build script.
    ; this line INCludes the BINary data directly into the `.chr_rom_0` section
    .incbin "background.chr"
