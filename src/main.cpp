#include <cstdio>
#include <mapper.h>
#include <nesdoug.h>
#include <neslib.h>

#include "explosion.hpp"

MAPPER_PRG_ROM_KB(32);
MAPPER_CHR_ROM_KB(128);
MAPPER_PRG_RAM_KB(8);
MAPPER_USE_VERTICAL_MIRRORING;


// The first byte of a nametable op is either the high byte of the VRAM address
// for a single-tile op, or the op-code for a multi-byte operation. Nametable
// addresses end at 0x2FFF, so multi-byte operations are indicated by values
// greater than 0x30

constexpr char NAMETABLE_OP_MULTI_START = 0x30;

enum nametable_opcode : char {
  single = NAMETABLE_OP_MULTI_START,
  copy_horz,
};

struct nametable_op_single_header {
  char address_hi;
  char address_lo;
  char data;
};

struct nametable_op_multi_header {
  nametable_opcode opcode;
  char address_hi;
  char address_lo;
  char size;
};

extern char VRAM_BUF[];
extern unsigned char VRAM_INDEX;

void one_nametable_buffer(unsigned address, char data) {
  auto &header = *reinterpret_cast<nametable_op_single_header *>(VRAM_BUF + VRAM_INDEX);
  header.address_hi = address >> 8;
  header.address_lo = address & 0xFF;
  header.data = data;
  VRAM_INDEX += sizeof(nametable_op_single_header);
  VRAM_BUF[VRAM_INDEX] = 0xFF;
}

void multi_vram_buffer_horz(unsigned address, unsigned char size, char *data) {
  auto dest = VRAM_BUF + VRAM_INDEX;
  auto &header = *reinterpret_cast<nametable_op_multi_header *>(dest);
  header.opcode = nametable_opcode::copy_horz;
  header.address_hi = address >> 8;
  header.address_lo = address & 0xFF;
  header.size = size;

  dest += sizeof(nametable_op_multi_header);
#pragma unroll 8
  for (unsigned char i = 0; i < size; ++i, ++VRAM_INDEX) {
    VRAM_BUF[VRAM_INDEX] = data[i];
  }
  VRAM_BUF[VRAM_INDEX] = 0xFF;
}

constexpr char kScreenWidth = 32;
constexpr char kScreenHeight = 30;
constexpr int kScreenSize = kScreenWidth * kScreenHeight;

constexpr char kPixelsPerTile = 8;

constexpr char hello[] = "Hello, NES!";

constexpr char background_pal[] = {
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
};

constexpr char sprite_pal[] = {
    0x0f, 0x10, 0x26, 0x30, // cogwheel
    0x0f, 0x11, 0x2a, 0x16, // explosions
    0x0f, 0x10, 0x20, 0x30, // unused
    0x0f, 0x10, 0x20, 0x30, // unused
};

void init_ppu() {
  // Disable the PPU so we can freely modify its state
  ppu_off();

  // Set up bufferd VRAM operations (see `multi_vram_buffer_horz` below)
  set_nametable_buffer();

  // Use lower half of PPU memory for background tiles
  bank_bg(0);

  // Set the background palette
  pal_bg(background_pal);

  // Fill the background with space characters to clear the screen
  vram_adr(NAMETABLE_A);
  vram_fill(' ', kScreenSize);

  // Write a message
  vram_adr(NTADR_A(10, 10));
  vram_write(hello, sizeof(hello) - 1);

  // Use the upper half of PPU memory for sprites
  bank_spr(1);

  // Set the sprite palette
  pal_spr(sprite_pal);

  // Turn the PPU back on
  ppu_on_all();
}

int main() {
  init_ppu();

  // Counters to cycle through palette colors, changing every half second
  char palette_color = 0;
  char counter = 0;

  // Start with the first sprite bank
  char sprite_bank = 1;

  // Cogwheel position
  char cog_x = 15 * kPixelsPerTile;
  char cog_y = 14 * kPixelsPerTile;

  // Store pad state across frames to check for changes
  char prev_pad_state = 0;

  for (;;) {
    // Wait for the NMI routine to end so we can start working on the next frame
    ppu_wait_nmi();

    // Set the MMC1 to use the chosen CHR bank for the upper half of the PPU
    // pattern table. Do this first thing after NMI finishes so that we are
    // still in VBLANK.
    set_chr_bank_1(sprite_bank);

    // The OAM (object attribute memory) is an area of RAM that contains data
    // about all the sprites that will be drawn next frame.
    oam_clear();

    // Note: if you don't poll a controller during a frame, emulators will
    // report that as lag
    const char pad_state = pad_poll(0);

    // Speed up when pressing B
    const char speed = pad_state & PAD_B ? 2 : 1;

    // Move the cogwheel in response to pad directions
    if (pad_state & PAD_UP) {
      cog_y -= speed;
    } else if (pad_state & PAD_DOWN) {
      cog_y += speed;
    }

    if (pad_state & PAD_LEFT) {
      cog_x -= speed;
    } else if (pad_state & PAD_RIGHT) {
      cog_x += speed;
    }

    if (pad_state & PAD_A) {
      // Create an explosion immediately when A is pressed, and then every 8
      // frames as long as A is held
      // `& 0x7` is equivalent to % `8`
      if (!(prev_pad_state & PAD_A) || !(get_frame_count() & 0x7)) {
        const char x = cog_x + (rand8() & 0xF);
        const char y = cog_y + 8 + (rand8() & 0xF);
        addExplosion(x, y);
      }
    }

    if (prev_pad_state & PAD_SELECT && !(pad_state & PAD_SELECT)) {
      // Select was released - swap CHR banks
      sprite_bank = sprite_bank == 1 ? 2 : 1;
    }

    prev_pad_state = pad_state;

    animateExplosions();

    // Adding Cogwheel after the explosions means the explosions will be prioritized
    for (char row = 0; row < 3; ++row) {
      for (char col = 0; col < 3; ++col) {
        // Convert row/col to pixels and add to cog position
        char const sprite_x = cog_x + (col << 3);
        char const sprite_y = cog_y + (row << 3);

        // There are 16 tiles per row; shift by 4
        char const tile = (row << 4) + col;
        oam_spr(sprite_x, sprite_y, tile, 0);
      }
    }

    // Change the color every half second (60 fps)
    if(++counter == 30) {
      counter = 0;
      if (++palette_color == 64) palette_color = 0;
      pal_col(3, palette_color);

      // Print the current palette color in hex
      char buffer[4];
      std::snprintf(buffer, sizeof(buffer), "$%02x", static_cast<int>(palette_color));

      // Copy the text into the VRAM buffer. This will draw characters at the
      // given VRAM address during the next vertical blank period.
      nametable_buffer_copy_horz(buffer, NTADR_A(14, 12), 3);
    }
  }
}
