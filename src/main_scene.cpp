#include "main_scene.hpp"

#include <cstdio>

#include <mapper.h>
#include <nesdoug.h>
#include <neslib.h>

#include "common.hpp"
#include "explosion.hpp"
#include "scene.hpp"

// Make it so constants (rodata) and code (text) from this file are placed into
// 5th (index 4) PRG bank in the ROM.
//
// This is just an arbitrary placement to demonstrate PRG banking in action
#pragma clang section rodata = ".prg_rom_4.rodata" text = ".prg_rom_4.text"

BANK_DATA const char main_scene_bg_palette[] = {
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
};

BANK_DATA const char main_scene_spr_palette[] = {
    0x0f, 0x10, 0x26, 0x30, // cogwheel
    0x0f, 0x11, 0x2a, 0x16, // explosions
    0x0f, 0x10, 0x20, 0x30, // unused
    0x0f, 0x10, 0x20, 0x30, // unused
};

// Put implementation into an anonymous namespace instead of writing `static`
// everywhere
namespace {

using namespace cog;

// Counters to cycle through palette colors, changing every half second
char palette_color;
char counter;

// Swap sprite banks when the player presses Select
char sprite_bank;
char cog_x;
char cog_y;

constexpr char hello[] = "Hello, NES!";

void init_vars() {
  palette_color = 0;
  counter = 0;

  // Start with the first sprite bank
  sprite_bank = 1;

  // Cogwheel position
  cog_x = 15 * kPixelsPerTile;
  cog_y = 14 * kPixelsPerTile;
}

void draw_bg() {
  // Fill the background with space characters to clear the screen
  vram_adr(NAMETABLE_A);
  vram_fill(' ', kScreenSize);

  // Write a message
  vram_adr(NTADR_A(10, 10));
  vram_write(hello, sizeof(hello) - 1);
}

void swap_chr_on_select(InputState const &input) {
  // Set the MMC1 to use the chosen CHR bank for the upper half of the PPU
  // pattern table. Do this first thing after NMI finishes so that we are
  // still in VBLANK.
  set_chr_bank_1(sprite_bank);

  if (input.prev_pad0 & PAD_SELECT && !(input.pad0 & PAD_SELECT)) {
    // Select was released - swap CHR banks
    sprite_bank = sprite_bank == 1 ? 2 : 1;
  }
}

void update_cog_position(InputState const &input) {
  // Speed up when pressing B
  const char speed = input.pad0 & PAD_B ? 2 : 1;

  // Move the cogwheel in response to pad directions
  if (input.pad0 & PAD_UP) {
    cog_y -= speed;
  } else if (input.pad0 & PAD_DOWN) {
    cog_y += speed;
  }

  if (input.pad0 & PAD_LEFT) {
    cog_x -= speed;
  } else if (input.pad0 & PAD_RIGHT) {
    cog_x += speed;
  }
}

void update_explosions(InputState const &input) {
  if (input.pad0 & PAD_A) {
    // Create an explosion immediately when A is pressed, and then every 8
    // frames as long as A is held
    // `& 0x7` is equivalent to % `8`
    if (!(input.prev_pad0 & PAD_A) || !(get_frame_count() & 0x7)) {
      const char x = cog_x + (rand8() & 0xF);
      const char y = cog_y + 8 + (rand8() & 0xF);
      add_explosion(x, y);
    }
  }
}

void draw_cog() {
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
}

void update_palette_color() {
  // Change the color every half second (60 fps)
  if (++counter == 30) {
    counter = 0;
    if (++palette_color == 64)
      palette_color = 0;
    pal_col(3, palette_color);

    // Print the current palette color in hex
    char buffer[4];
    std::snprintf(buffer, sizeof(buffer), "$%02x",
                  static_cast<int>(palette_color));

    // Copy the text into the VRAM buffer. This will draw characters at the
    // given VRAM address during the next vertical blank period.
    multi_vram_buffer_horz(buffer, 3, NTADR_A(14, 12));
  }
}

} // end anonymous namespace

BANK_FUNC void init_main_scene() {
  init_vars();
  draw_bg();
}

BANK_FUNC cog::SceneResult
update_main_scene(cog::InputState const &input,
                  cog::TransitionState /* unused. TODO */) {
  swap_chr_on_select(input);
  update_cog_position(input);
  update_explosions(input);
  update_palette_color();

  // Draw sprites
  // Explosions are drawn first so they render on top of the cogwheel
  cog::animate_explosions();
  draw_cog();

  return cog::kContinueScene;
}
