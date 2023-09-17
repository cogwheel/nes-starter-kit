#pragma once

#include "scene.hpp"

BANK_DATA const char main_scene_bg_palette[];
BANK_DATA const char main_scene_spr_palette[];

BANK_FUNC void init_main_scene();
BANK_FUNC cog::SceneResult update_main_scene(cog::InputState const &, cog::TransitionState);
