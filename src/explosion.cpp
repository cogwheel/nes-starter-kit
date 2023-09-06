#include "explosion.hpp"

#include <neslib.h>

// Max number of explosions to display simultaneously
constexpr char kNumExplosions = 10;

// Number of animation frames in CHR data
constexpr char kExplosionChrFrames = 8;

// Tile id of the starting (leftmost) animation frame
constexpr char kExplosionStartTile = 0x30;

// Number of screen frames per explosion frame
constexpr char kExplosionScreenFrames = 4;

// Starting value of countdown timer. This will be decremented each game frame
// and used to index the animation frame.
constexpr char kExplosionTimer =
    kExplosionChrFrames * kExplosionScreenFrames - 1;

struct Explosion {
  char x;
  char y;
  char timer;
};

// Ring buffer for explosion animations
static Explosion explosions[kNumExplosions];

// `explosion_head` is the index of the first active explosion, if any
static int explosion_head = 0;

// `explosion_tail` is the next empty explosion location. If this is the same
// as `explosion_head`, it means there are no active explosions.
static int explosion_tail = 0;

void addExplosion(char x, char y) {
  // Put the new explosion at the tail location, and increment the tail
  explosions[explosion_tail++] = {x, y, kExplosionTimer};

  // Wrap tail to 0 if necessary
  if (explosion_tail == kNumExplosions)
    explosion_tail = 0;

  if (explosion_tail == explosion_head) {
    // Buffer is full; eject oldest explosion by incrementing head
    ++explosion_head;
    if (explosion_head == kNumExplosions)
      // wrap if necessary
      explosion_head = 0;
  }
}

static void animateExplosion(Explosion &explosion) {
  // move the explosion up over time
  const char sprite_y = explosion.y + explosion.timer - kExplosionTimer;

  // Select the animation frame.
  // Note: this could be simplified a bit if the animation frames were in
  // reverse order. This is left as an exercise for the reader
  // (read: I'm lazy --cogwheel)
  const char anim_offset =
      kExplosionChrFrames - 1 - (explosion.timer / kExplosionScreenFrames);

  // Explosion starts at tile ID 0x30. The attribute 1 chooses the second
  // sprite palette.
  oam_spr(explosion.x, sprite_y, kExplosionStartTile + anim_offset, 1);

  if (--explosion.timer == 0) {
    // Expired. Increment head
    if (++explosion_head == kNumExplosions)
      explosion_head = 0;
  }
}

void animateExplosions() {
  int head = explosion_head;

  // If the head of the circular buffer is past the tail then consume until the
  // end of the buffer
  if (head > explosion_tail) {
    for(; head < kNumExplosions; ++head) {
      animateExplosion(explosions[head]);
    }
    // Wrap to the beginning of the buffer
    head = 0;
  }

  // If the head of the cicular buffer is less than the tail, then consume
  // until reaching the tail
  for (; head < explosion_tail; ++head) {
    animateExplosion(explosions[head]);
  }
}
