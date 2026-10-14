// ============================================================================
// game.c - cabnetBoy Rogue : a turn based, grid based, single player roguelike
// ============================================================================
//
// Design notes for a 16 KB / 2 KB machine with no FPU and no hardware divide:
//
//  * Integer only. No float anywhere (the old cabnetBoy firmware used float
//    for the ball physics - that pulls in the soft-float library).
//  * The level is stored as 2 bits per tile (32x16 = 128 bytes) instead of
//    one byte per tile. Explored / visible maps are 1 bit per tile (64 B).
//  * Nothing is double buffered: gfx streams one 8-pixel page at a time, so
//    the renderer costs 128 bytes of RAM instead of a 1 KB frame buffer.
//  * Everything is turn based, so the CPU can spend 25 ms redrawing after
//    every key press and still feel instant.
//
// Controls (UP / DOWN / LEFT buttons + ACT):
//     UP     walk one tile forward (or attack whatever is in front)
//     LEFT   turn counter clockwise
//     DOWN   turn clockwise
//     ACT    pick up / take stairs / wait one turn
//   The RV09 knob also turns the player: twist right = turn clockwise.
// ============================================================================
#include "game.h"
#include "retro.h"
#include "gfx.h"
#include "tiles.h"

// ---------------------------------------------------------------- map ------
#define MAP_W     32
#define MAP_H     16
#define FOV_R     7
#define MAX_MON   10
#define MAX_ITEM  6

// terrain codes stored in the map, 2 bits each
// (distinct from the graphic tile indices G_* in tiles.h)
#define TER_WALL   0
#define TER_FLOOR  1
#define TER_DOOR   2
#define TER_STAIR  3

// monster kinds
#define M_RAT     0
#define M_GOB     1
#define M_SNAKE   2
#define M_ORC     3

// item kinds
#define I_POTION  0
#define I_GOLD    1

typedef struct { uint8_t x, y, hp, kind; } mon_t;
typedef struct { uint8_t x, y, kind; } item_t;

static uint8_t map[(MAP_W * MAP_H) / 4];      // 128 bytes, 2 bits per tile
static uint8_t seen[(MAP_W * MAP_H) / 8];     // 64 bytes, explored
static uint8_t vis[(MAP_W * MAP_H) / 8];      // 64 bytes, visible now

static mon_t   mons[MAX_MON];
static item_t  items[MAX_ITEM];
static uint8_t n_mon, n_item;

static uint8_t px, py, facing;                // facing: 0=N 1=E 2=S 3=W
static uint8_t hp, hp_max, depth, gold, alive;
static uint16_t turns;

static uint8_t cam_x, cam_y;
static char status_buf[VIEW_W + 1];
static char msg_buf[VIEW_W + 1];

// ------------------------------------------------------------ 2 bpp map ----
static uint8_t mget(uint8_t x, uint8_t y) {
  uint16_t i = (uint16_t)y * MAP_W + x;
  return (map[i >> 2] >> ((i & 3) * 2)) & 3;
}

static void mset(uint8_t x, uint8_t y, uint8_t v) {
  uint16_t i = (uint16_t)y * MAP_W + x;
  uint8_t s = (uint8_t)((i & 3) * 2);
  map[i >> 2] = (uint8_t)((map[i >> 2] & ~(3u << s)) | ((uint32_t)(v & 3) << s));
}

// ----------------------------------------------------------- 1 bpp flags ---
static uint8_t bitget(const uint8_t *bm, uint8_t x, uint8_t y) {
  uint16_t i = (uint16_t)y * MAP_W + x;
  return (bm[i >> 3] >> (i & 7)) & 1;
}

static void bitset(uint8_t *bm, uint8_t x, uint8_t y) {
  uint16_t i = (uint16_t)y * MAP_W + x;
  bm[i >> 3] |= (uint8_t)(1u << (i & 7));
}

static void bitclear_all(uint8_t *bm) {
  uint8_t i;
  for (i = 0; i < sizeof(seen); i++) bm[i] = 0;
}

// ------------------------------------------------------------- helpers -----
static int8_t iabs8(int8_t v) { return (v < 0) ? (int8_t)(-v) : v; }

static uint8_t walkable(uint8_t x, uint8_t y) {
  uint8_t t = mget(x, y);
  return (t == TER_FLOOR || t == TER_DOOR || t == TER_STAIR);
}

static uint8_t mon_at(uint8_t x, uint8_t y) {
  uint8_t i;
  for (i = 0; i < n_mon; i++)
    if (mons[i].hp && mons[i].x == x && mons[i].y == y) return i;
  return 0xFF;
}

static uint8_t item_at(uint8_t x, uint8_t y) {
  uint8_t i;
  for (i = 0; i < n_item; i++)
    if (items[i].kind != 0xFF && items[i].x == x && items[i].y == y) return i;
  return 0xFF;
}

// ------------------------------------------------------------- messages ----
static void msg_set(const char *s) {
  uint8_t i = 0;
  while (i < VIEW_W) {
    char c = s[i];
    if (!c) break;
    if (c >= 'a' && c <= 'z') c -= 32;
    msg_buf[i++] = c;
  }
  while (i < VIEW_W) msg_buf[i++] = ' ';
  msg_buf[VIEW_W] = 0;
}

static void msg_num(const char *prefix, uint16_t v) {
  char tmp[VIEW_W + 1];
  uint8_t i = 0, n = 0;
  char digits[5];
  while (prefix[i] && i < VIEW_W) { tmp[i] = prefix[i]; i++; }
  if (v == 0) digits[n++] = '0';
  while (v && n < 5) { digits[n++] = (char)('0' + (v % 10)); v /= 10; }
  while (n && i < VIEW_W) tmp[i++] = digits[--n];
  while (i < VIEW_W) tmp[i++] = ' ';
  tmp[VIEW_W] = 0;
  msg_set(tmp);
}

// ------------------------------------------------------------ level gen ----
typedef struct { uint8_t x, y, w, h; } room_t;
static room_t rooms[5];

static void carve_room(const room_t *r) {
  uint8_t x, y;
  for (y = r->y; y < r->y + r->h; y++)
    for (x = r->x; x < r->x + r->w; x++) mset(x, y, TER_FLOOR);
}

static uint8_t room_free(const room_t *r) {
  int8_t x, y;
  for (y = (int8_t)r->y - 1; y <= (int8_t)(r->y + r->h); y++)
    for (x = (int8_t)r->x - 1; x <= (int8_t)(r->x + r->w); x++) {
      if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) continue;
      if (mget((uint8_t)x, (uint8_t)y) != TER_WALL) return 0;
    }
  return 1;
}

static void hcorr(uint8_t x0, uint8_t x1, uint8_t y) {
  uint8_t x;
  if (x0 > x1) { x = x0; x0 = x1; x1 = x; }
  for (x = x0; x <= x1; x++) mset(x, y, TER_FLOOR);
}

static void vcorr(uint8_t y0, uint8_t y1, uint8_t x) {
  uint8_t y;
  if (y0 > y1) { y = y0; y0 = y1; y1 = y; }
  for (y = y0; y <= y1; y++) mset(x, y, TER_FLOOR);
}

static void gen_level(void) {
  uint8_t i, tries, x, y, nrooms = 4;

  for (i = 0; i < sizeof(map); i++) map[i] = 0;      // everything is wall
  bitclear_all(seen);
  bitclear_all(vis);
  n_mon = 0;
  n_item = 0;

  for (i = 0; i < nrooms; i++) {
    tries = 0;
    for (;;) {
      rooms[i].w = (uint8_t)(4 + retro_rnd_n(4));       // 4..7
      rooms[i].h = (uint8_t)(2 + retro_rnd_n(3));       // 2..4
      rooms[i].x = (uint8_t)(1 + retro_rnd_n(MAP_W - rooms[i].w - 2));
      rooms[i].y = (uint8_t)(1 + retro_rnd_n(MAP_H - rooms[i].h - 2));
      if (room_free(&rooms[i])) break;
      if (++tries > 30) { rooms[i].w = 4; rooms[i].h = 2;
                          rooms[i].x = 2 + i * 6; rooms[i].y = 2 + (i & 1) * 5;
                          break; }
    }
    carve_room(&rooms[i]);
  }

  for (i = 1; i < nrooms; i++) {
    uint8_t cx0 = rooms[i - 1].x + rooms[i - 1].w / 2;
    uint8_t cy0 = rooms[i - 1].y + rooms[i - 1].h / 2;
    uint8_t cx1 = rooms[i].x + rooms[i].w / 2;
    uint8_t cy1 = rooms[i].y + rooms[i].h / 2;
    hcorr(cx0, cx1, cy0);
    vcorr(cy0, cy1, cx1);
  }

  // player starts in the middle of room 0
  px = (uint8_t)(rooms[0].x + rooms[0].w / 2);
  py = (uint8_t)(rooms[0].y + rooms[0].h / 2);
  facing = 1;

  // stairs in the last room
  mset((uint8_t)(rooms[nrooms - 1].x + rooms[nrooms - 1].w / 2),
       (uint8_t)(rooms[nrooms - 1].y + rooms[nrooms - 1].h / 2), TER_STAIR);

  // monsters: one to three per room except the starting room
  for (i = 1; i < nrooms && n_mon < MAX_MON; i++) {
    uint8_t cnt = (uint8_t)(1 + retro_rnd_n(2 + depth / 3));
    while (cnt-- && n_mon < MAX_MON) {
      x = (uint8_t)(rooms[i].x + retro_rnd_n(rooms[i].w));
      y = (uint8_t)(rooms[i].y + retro_rnd_n(rooms[i].h));
      if (!walkable(x, y) || (x == px && y == py)) continue;
      if (mon_at(x, y) != 0xFF) continue;
      mons[n_mon].x = x;
      mons[n_mon].y = y;
      mons[n_mon].kind = (uint8_t)(retro_rnd_n(2) + ((depth > 2) ? 1 : 0)
                                   + ((depth > 5) ? 1 : 0));
      if (mons[n_mon].kind > M_ORC) mons[n_mon].kind = M_ORC;
      mons[n_mon].hp = (uint8_t)(3 + mons[n_mon].kind * 2);
      n_mon++;
    }
  }

  // a couple of items
  for (i = 0; i < 3 && n_item < MAX_ITEM; i++) {
    uint8_t r = (uint8_t)(1 + retro_rnd_n(3));
    x = (uint8_t)(rooms[r].x + retro_rnd_n(rooms[r].w));
    y = (uint8_t)(rooms[r].y + retro_rnd_n(rooms[r].h));
    if (!walkable(x, y) || item_at(x, y) != 0xFF) continue;
    items[n_item].x = x;
    items[n_item].y = y;
    items[n_item].kind = (retro_rnd_n(3) == 0) ? I_GOLD : I_POTION;
    n_item++;
  }
}

// ------------------------------------------------------------------ LOS ----
static uint8_t los(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
  int8_t ax = (int8_t)x0, ay = (int8_t)y0;
  int8_t bx = (int8_t)x1, by = (int8_t)y1;
  int8_t dx = iabs8((int8_t)(bx - ax));
  int8_t dy = (int8_t)(-iabs8((int8_t)(by - ay)));
  int8_t sx = (ax < bx) ? 1 : -1;
  int8_t sy = (ay < by) ? 1 : -1;
  int8_t err = (int8_t)(dx + dy);
  int8_t e2;

  for (;;) {
    if (ax == bx && ay == by) return 1;
    e2 = (int8_t)(err << 1);
    if (e2 >= dy) { err = (int8_t)(err + dy); ax = (int8_t)(ax + sx); }
    if (e2 <= dx) { err = (int8_t)(err + dx); ay = (int8_t)(ay + sy); }
    if (ax == bx && ay == by) return 1;
    if (ax < 0 || ay < 0 || ax >= MAP_W || ay >= MAP_H) return 0;
    if (mget((uint8_t)ax, (uint8_t)ay) == TER_WALL) return 0;
  }
}

static void fov_update(void) {
  int8_t dx, dy;
  int8_t x, y;
  bitclear_all(vis);
  bitset(vis, px, py);
  bitset(seen, px, py);
  for (dy = -FOV_R; dy <= FOV_R; dy++)
    for (dx = -FOV_R; dx <= FOV_R; dx++) {
      if (dx * dx + dy * dy > FOV_R * FOV_R) continue;
      x = (int8_t)px + dx;
      y = (int8_t)py + dy;
      if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) continue;
      if (los(px, py, (uint8_t)x, (uint8_t)y)) {
        bitset(vis, (uint8_t)x, (uint8_t)y);
        bitset(seen, (uint8_t)x, (uint8_t)y);
      }
    }
}

// -------------------------------------------------------------- combat -----
static const char MON_CH[4] = { 'R', 'G', 'S', 'O' };
static const uint8_t MON_ATK[4] = { 1, 2, 2, 3 };

static void player_attack(uint8_t mi) {
  uint8_t dmg = (uint8_t)(1 + retro_rnd_n(2) + depth / 3);
  if (dmg >= mons[mi].hp) {
    mons[mi].hp = 0;
    retro_beep(140, 55);
    msg_set("YOU KILL IT");
  } else {
    mons[mi].hp = (uint8_t)(mons[mi].hp - dmg);
    retro_beep(300, 12);
    msg_set("YOU HIT IT");
  }
}

static void hurts_player(uint8_t amount, const char *what) {
  if (amount >= hp) hp = 0; else hp = (uint8_t)(hp - amount);
  retro_beep(110, 70);
  msg_set(what);
}

static void monsters_turn(void) {
  uint8_t i;
  for (i = 0; i < n_mon; i++) {
    int8_t dx, dy, sx, sy;
    uint8_t nx, ny;
    if (!mons[i].hp) continue;
    if (!bitget(vis, mons[i].x, mons[i].y)) continue;   // only awake while seen

    dx = (int8_t)((int8_t)px - (int8_t)mons[i].x);
    dy = (int8_t)((int8_t)py - (int8_t)mons[i].y);

    if (iabs8(dx) <= 1 && iabs8(dy) <= 1 && (dx || dy)) {
      hurts_player(MON_ATK[mons[i].kind], "IT HITS YOU");
      if (!hp) return;
      continue;
    }
    sx = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
    sy = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);

    for (uint8_t step = 0; step < 2; step++) {
      if (step == 0) {
        if (iabs8(dx) >= iabs8(dy)) { nx = (uint8_t)(mons[i].x + sx); ny = mons[i].y; }
        else                        { nx = mons[i].x; ny = (uint8_t)(mons[i].y + sy); }
      } else {
        if (iabs8(dx) >= iabs8(dy)) { nx = mons[i].x; ny = (uint8_t)(mons[i].y + sy); }
        else                        { nx = (uint8_t)(mons[i].x + sx); ny = mons[i].y; }
      }
      if (!sx && !sy) break;
      if (nx == px && ny == py) break;
      if (!walkable(nx, ny)) continue;
      if (mon_at(nx, ny) != 0xFF) continue;
      mons[i].x = nx;
      mons[i].y = ny;
      break;
    }
  }
}

// ----------------------------------------------------------- player act ----
static void try_step(void) {
  static const int8_t DX[4] = { 0, 1, 0, -1 };
  static const int8_t DY[4] = { -1, 0, 1, 0 };
  uint8_t nx = (uint8_t)(px + DX[facing]);
  uint8_t ny = (uint8_t)(py + DY[facing]);
  uint8_t mi;

  if (nx >= MAP_W || ny >= MAP_H) { retro_beep(90, 20); return; }
  mi = mon_at(nx, ny);
  if (mi != 0xFF) { player_attack(mi); turns++; return; }
  if (!walkable(nx, ny)) { retro_beep(90, 20); return; }
  px = nx;
  py = ny;
  turns++;
  retro_beep(1000, 5);
}

static void do_act(void) {
  uint8_t ii = item_at(px, py);
  if (ii != 0xFF) {
    if (items[ii].kind == I_POTION) {
      hp = (uint8_t)((hp + 6 > hp_max) ? hp_max : hp + 6);
      msg_set("GLUG GLUG");
    } else {
      gold = (uint8_t)(gold + 5 + retro_rnd_n(10));
      msg_set("YOU GOT GOLD");
    }
    items[ii].kind = 0xFF;
    retro_beep(1400, 20);
    turns++;
    return;
  }
  if (mget(px, py) == TER_STAIR) {
    depth++;
    hp = (uint8_t)((hp + 3 > hp_max) ? hp_max : hp + 3);
    gen_level();
    fov_update();
    retro_beep(600, 40);
    retro_beep(900, 40);
    retro_beep(1300, 60);
    msg_num("FLOOR ", depth);
    turns++;
    return;
  }
  msg_set("YOU WAIT");
  retro_beep(400, 8);
  turns++;
}

// -------------------------------------------------------------- drawing ----
static uint8_t cell_code(uint8_t x, uint8_t y) {
  uint8_t mi, ii, t;

  if (x == px && y == py) return G_PLAYER;
  if (!bitget(seen, x, y)) return G_VOID;

  if (bitget(vis, x, y)) {
    mi = mon_at(x, y);
    if (mi != 0xFF) return (uint8_t)MON_CH[mons[mi].kind];
    ii = item_at(x, y);
    if (ii != 0xFF) return (items[ii].kind == I_GOLD) ? '$' : '!';
  }

  t = mget(x, y);
  switch (t) {
    case TER_WALL:  return G_WALL;
    case TER_DOOR:  return '+';
    case TER_STAIR: return '>';
    default:        return G_FLOOR;
  }
}

static void build_status(void) {
  uint8_t i = 0;
  status_buf[i++] = 'H';
  status_buf[i++] = 'P';
  status_buf[i++] = (char)('0' + (hp / 10) % 10);
  status_buf[i++] = (char)('0' + hp % 10);
  status_buf[i++] = '/';
  status_buf[i++] = (char)('0' + (hp_max / 10) % 10);
  status_buf[i++] = (char)('0' + hp_max % 10);
  status_buf[i++] = ' ';
  status_buf[i++] = 'L';
  status_buf[i++] = (char)('0' + (depth / 10) % 10);
  status_buf[i++] = (char)('0' + depth % 10);
  status_buf[i++] = ' ';
  status_buf[i++] = 'G';
  status_buf[i++] = (char)('0' + (gold / 100) % 10);
  status_buf[i++] = (char)('0' + (gold / 10) % 10);
  status_buf[i++] = (char)('0' + gold % 10);
  status_buf[i] = 0;
}

static void render(void) {
  uint8_t page, col;

  cam_x = (px > VIEW_W / 2) ? (uint8_t)(px - VIEW_W / 2) : 0;
  if (cam_x > MAP_W - VIEW_W) cam_x = MAP_W - VIEW_W;
  cam_y = (py > VIEW_H / 2) ? (uint8_t)(py - VIEW_H / 2) : 0;
  if (cam_y > MAP_H - VIEW_H) cam_y = MAP_H - VIEW_H;

  for (page = 0; page < VIEW_H; page++) {
    gfx_begin(page);
    for (col = 0; col < VIEW_W; col++)
      gfx_cell(col, cell_code((uint8_t)(cam_x + col), (uint8_t)(cam_y + page)));
    gfx_send();
  }
  build_status();
  gfx_begin(STATUS_PAGE);
  gfx_text(0, status_buf);
  gfx_send();

  gfx_begin(MESSAGE_PAGE);
  gfx_text(0, msg_buf);
  gfx_send();
}

// ----------------------------------------------------------------- input ---
// Blocking. Buttons are debounced; the RV09 knob is also read so it can be
// used to turn the player.
static uint8_t wait_key(void) {
  uint8_t last_knob = retro_knob();
  uint8_t b;

  for (;;) {
    b = retro_buttons();
    if (b) {
      DLY_ms(12);
      if (retro_buttons() == b) {
        while (retro_buttons()) DLY_ms(5);
        return b;
      }
    }
    {
      uint8_t k = retro_knob();
      int8_t d = (int8_t)(k - last_knob);
      if (d > 40 || d < -40) {
        last_knob = k;
        return (d > 0) ? BTN_DOWN : BTN_LEFT;   // twist = turn
      }
    }
    DLY_ms(4);
  }
}

// ------------------------------------------------------------- new game ----
static void new_game(void) {
  hp_max = 12;
  hp = hp_max;
  depth = 1;
  gold = 0;
  turns = 0;
  alive = 1;
  gen_level();
  fov_update();
  msg_set("CABNETBOY ROGUE");
}

static void game_over(void) {
  retro_beep(400, 120);
  retro_beep(300, 120);
  retro_beep(200, 160);
  retro_beep(120, 260);
  msg_num("YOU DIED SCORE ", gold);
  hp = 0;
  render();
  for (;;) {
    if (retro_buttons() & BTN_ACT) break;
    DLY_ms(20);
  }
  while (retro_buttons()) DLY_ms(10);
}

// ------------------------------------------------------------- main loop ---
void game_run(void) {
  uint8_t key;

  new_game();
  for (;;) {
    render();
    key = wait_key();

    switch (key) {
      case BTN_UP:   try_step();                 break;
      case BTN_LEFT: facing = (uint8_t)((facing + 3) & 3); turns++; retro_beep(700, 6); break;
      case BTN_DOWN: facing = (uint8_t)((facing + 1) & 3); turns++; retro_beep(700, 6); break;
      case BTN_ACT:  do_act();                   break;
      default: break;
    }

    monsters_turn();
    fov_update();

    if (!hp) { game_over(); new_game(); }
  }
}

