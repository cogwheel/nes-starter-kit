#include <ines.h>
#include <nesdoug.h>

#include "main_scene.hpp"
#include "prg_ptr.hpp"
#include "scene.hpp"

MAPPER_PRG_ROM_KB(256);
MAPPER_CHR_ROM_KB(128);
MAPPER_PRG_RAM_KB(8);
MAPPER_USE_VERTICAL_MIRRORING;

const cog::Scene scenes[] = {
  {
    .init = PRG_PTR(init_main_scene),
    .update = PRG_PTR(update_main_scene),
    .bg_palette = PRG_PTR(main_scene_bg_palette),
    .spr_palette = PRG_PTR(main_scene_spr_palette),
    // The remaining defaults are fine
  }
};

int main() {
  set_vram_buffer();

  cog::run_scenes(scenes, 1, 0);
}
