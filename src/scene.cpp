#include "scene.hpp"

#include <mapper.h>
#include <neslib.h>

#include "common.hpp"

namespace cog {

constexpr char kNumFadeFrames = 6;

void start_scene(Scene const &scene) {
  // Disable the PPU so we can freely modify its state
  ppu_off();

  // Clear out the sprite data (object attribute memory) buffer
  oam_clear();

  // Set palettes
  //
  // Uses scoped banking since `pal_bg` and `pal_spr` copy the data into a
  // buffer
  {
    auto const bg_palette = scene.bg_palette.push_bank();
    pal_bg(bg_palette);
  }

  {
    auto const spr_palette = scene.spr_palette.push_bank();
    pal_spr(spr_palette);
  }

  // Set CHR banks
  //
  // Cogwheel's note: it is unfortunate that "bank" means two different things
  // here. The names "bank_0" and "bank_1" refer to the two haves of PPU
  // nametable memory that are used for sprites and backgrounds. But the data
  // contained in "chr_bank_0" and "chr_bank_1" is a bank number referring to an
  // area in the actual ROM. So `set_chr_bankN` remaps PPU addresses to ROM
  // addresses, but `bank_bg/spr` tell the PPU which set of PPU addresses to use
  // for backgrounds or sprites.
  set_chr_bank_0(scene.chr_bank_0);
  set_chr_bank_1(scene.chr_bank_1);

  // Set PPU nametable banks
  bank_bg(scene.bg_bank);
  bank_spr(scene.spr_bank);

  // Set scroll
  scroll(scene.scroll_x, scene.scroll_y);

  // Initialize the scene
  banked_call(scene.init);

  ppu_on_all();
}

[[nodiscard]] static SceneResult
do_update(Scene const &scene, TransitionState const transition_state) {
  static InputState input{};

  // Note: if you don't poll a controller during a frame, emulators will
  // report that as a lag frame
  input.prev_pad0 = input.pad0;
  input.prev_pad1 = input.pad1;
  input.pad0 = pad_poll(0);
  input.pad1 = pad_poll(1);

  return banked_call(scene.update, input, transition_state);
}

static void wait_frame() {
  ppu_wait_nmi();
  oam_clear();
}

static void fade_brightness(Scene const &scene, char const from,
                            char const to, signed char step,
                            TransitionState const transition_state) {
  for (char brightness = from; brightness != to; brightness += step) {
    for (char timer = 0; timer < kNumFadeFrames; ++timer) {
      wait_frame();
      pal_bright(brightness);
      // Ignore any new scene requests during transition
      UNUSED(do_update(scene, transition_state));
    }
  }
  wait_frame();
  pal_bright(to);
  UNUSED(do_update(scene, TransitionState::kDone));
}

static void fade_in(Scene const &scene) {
  // Assumes brightness is already 0.
  fade_brightness(scene, 1, 4, 1, TransitionState::kIn);
}

static void fade_out(Scene const &scene) {
  // Assumes brightness is already 4;
  fade_brightness(scene, 3, 0, -1, TransitionState::kOut);

  // Need to wait one more frame in order for final palette change to take
  // effect before turning off the PPU
  wait_frame();
}

// Start the scene engine, never look back
[[gnu::noreturn]] void run_scenes(Scene const scenes[], char const num_scenes,
                                  char const first_scene) {
  UNUSED(num_scenes); // TODO: add a way to panic if scene ID is out of bounds

  pal_bright(0);

  // Initialize banks so that "prev" values are sensical. Also allows optimizer
  // to have a known starting point for elision
  set_prg_bank(0);

  char const last_scene = std::min(num_scenes - 1, 0);

  Scene const *scene = nullptr;
  SceneResult result = next_scene(std::min(first_scene, last_scene));
  for (;;) {
    scene = &scenes[result.next_scene];

    start_scene(*scene);
    fade_in(*scene);
    do {
      wait_frame();
      result = do_update(*scene, TransitionState::kNone);
    } while (!result.has_next);
    fade_out(*scene);
  }
}

} // end namespace cog
