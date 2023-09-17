#pragma once

/*
The scene system provides a largely declarative and data-oriented way to set up
and transition between different game states (e.g. from the main game to the
pause screen). For example, you can set which CHR banks to load, the starting
scroll position, etc.

The scene engine handles the basic game loop: waiting for NMI, clearing the OAM
buffer, and polling input. Then it calls the Scene's `update` function which
returns a result indicating what to do next (stay in this scene, transition,
etc.).

Each scene (or rather each module that defines the init/update functions for a
scene) is responsible for managing its own state. Since the scene is treated as
a singleton, it is free to use static objects for storing the state. Data that
needs to be initialized during `init` can be a file-level static. Other data
(e.g. counters, RNGs, and such) can be `static` in the `update` function.
*/

#include "prg_ptr.hpp"

namespace cog {

// SceneResult allows scenes to tell the scene engine what to do next. If
// `has_next` is true, then a transition will begin from the current scene to
// `next_scene`.
//
// This could be expanded to include the type of transition (fade to black,
// fade to white, some kind of schmancy demo effect, etc.)
struct SceneResult {
  char next_scene : 7;
  bool has_next : 1;
};

// Helper for returning a SceneResult with a next_scene
inline SceneResult next_scene(char scene_id) {
  return SceneResult{.next_scene = scene_id, .has_next = true};
}

constexpr SceneResult kContinueScene{.next_scene = 0, .has_next = false};

// TransitionState is a generic flag to tell the scene whether it's in the
// middle of a transition. This lets the scene take some subset of normal
// update/draw operations (e.g. rendering but not updating enemies while fading
// out to pause)
//
// TODO: support different transitions than fade-to-black
enum class TransitionState {
  kNone, // Not in the middle of any transition
  kIn,   // Transitioning into the current scene
  kOut,  // Transitioning out of the current scene
  kDone, // Final update of the current transition
};

// The current and previous pad states are passed to the scene update function
struct InputState {
  char pad0;
  char prev_pad0;

  char pad1;
  char prev_pad1;
};

// Data-driven description of a scene. This allows the game to more easily reuse
// components across different scenes compared to, say, an inheritance
// hierarchy.
struct Scene {
  // Init is called to initialize the scene before it appears. This function
  // should do any remaining PPU setup not handled by the Scene engine, draw
  // static backgrounds, and initialize any scene-specific data. The PPU is off
  // during this function.
  //
  // TODO: maybe split this into Init and Enter, where init only runs the
  // first time the scene is entered. This can be accomplished by setting or
  // checking a `static` flag in Init.
  using Init = void (*)();
  PrgPtr<Init> init;

  // Update should do any state updates and drawing relevant to the given pad
  // and transition states.
  using Update = SceneResult (*)(const InputState &input_state,
                                 TransitionState transition_state);
  PrgPtr<Update> update;

  // 16-byte palettes to use for the background and sprites
  PrgPtr<const char> bg_palette;
  PrgPtr<const char> spr_palette;

  // Set which banks of CHR ROM to use for the lower (0) and upper (1) halves of
  // the PPU pattern tables. For MMC1 these can be in the range [0, 31]
  //
  // TODO: make sure all 31 banks are usable in the current config.
  char chr_bank_0 = 0;
  char chr_bank_1 = 1;

  // Set which halves of the PPU pattern tables (0 or 1) to use for background
  // or sprites
  //
  // TODO: pack into single byte?
  char bg_bank = 0;
  char spr_bank = 1;

  // Initial scroll value
  unsigned int scroll_x = 0;
  unsigned int scroll_y = 0;
};

[[gnu::noreturn]] void run_scenes(Scene const scenes[], char num_scenes,
                                  char first_scene);

} // end namespace cog
