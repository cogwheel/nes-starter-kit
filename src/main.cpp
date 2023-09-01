#include <cstdio>

#include <nesdoug.h>
#include <neslib.h>

constexpr char kScreenWidth = 32;
constexpr char kScreenHeight = 30;
constexpr int kScreenSize = kScreenWidth * kScreenHeight;

constexpr char hello[] = "Hello, NES!";

constexpr char background_pal[] = {
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
};

int main() {
  // Disable the PPU so we can freely modify its state
  ppu_off();

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

  // Turn the PPU back on
  ppu_on_all();

  char palette_color = 0;
  char counter = 0;
  for (;;) {
    // Wait for the NMI routine to end so we can start working on the next frame
    ppu_wait_nmi();

    // Note: if you don't poll a controller during a frame, emulators will
    // report that as lag
    __attribute__((unused)) const char pad_state = pad_poll(0);

    if(counter == 0) {
      if (++palette_color == 64) palette_color = 0;

      // Tell `neslib` that we want to do a buffered background data transfer
      // this frame
      set_vram_buffer();
      char buffer[4];
      std::snprintf(buffer, sizeof(buffer), "$%02x", static_cast<int>(palette_color));
      multi_vram_buffer_horz(buffer, 3, NTADR_A(14, 12));
    }

    // TODO: interesting stuff
    pal_col(0, palette_color);

    if (++counter == 30) counter = 0;
  }
}