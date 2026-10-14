// ============================================================================
// main.c - cabnetBoy Rogue
// ============================================================================
#include "retro.h"
#include "game.h"

int main(void) {
  retro_init();
  game_run();
  return 0;                 // never reached
}
