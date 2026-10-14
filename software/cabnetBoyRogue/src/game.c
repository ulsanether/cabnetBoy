// ============================================================================
// game.c - cabnetBoy Rogue : turn based roguelike driven by the knob + slider
// ============================================================================
//
// Controls (all four analog/digital inputs of the cabnetBoy board are used):
//
//   RV09 knob  (PD4, ADC7)  character facing  - turn the knob like a compass
//   U5 slider  (PD6, ADC6)  POWER / MAGIC    - 1..5, read continuously
//   LEFT       select weapon / item          - DAG SWD AXE STF POT
//   ACT        attack / cast / use / take stairs
//   UP         step one tile forward (bumps attack too)
//   DOWN       wait one turn
//              (if the slider is dead, DOWN cycles the power instead)
//
// Turning the knob changes facing WITHOUT spending a turn, exactly like
// turning your head. Everything else spends a turn.
//
// Machine constraints this code respects:
//   * RV32EC: no FPU, no hardware multiply/divide -> integer only.
//   * 2 KB SRAM: the map is 2 bits per tile and the renderer streams one
//     128 byte OLED page instead of holding a 1 KB frame buffer.
//   * 16 KB flash: 8x8 font + 8 tiles + this file + drivers = ~6.4 KB.
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

// terrain codes, 2 bits each
#define TER_WALL   0
#define TER_FLOOR  1
#define TER_DOOR   2
#define TER_STAIR  3

// monster kinds
#define M_RAT     0
#define M_GOB     1
#define M_SNAKE   2
#define M_ORC     3

// item kinds lying on the floor
#define I_POTION  0
#define I_GOLD    1

// inventory slots
#define INV_DAG   0
#define INV_SWD   1
#define INV_AXE   2
#define INV_STF   3
#define INV_POT   4
#define INV_N     5

#define MP_MAX    10

typedef struct { uint8_t x, y, hp, kind; } mon_t;
typedef struct { uint8_t x, y, kind; } item_t;

static uint8_t map[(MAP_W * MAP_H) / 4];      // 128 bytes, 2 bits per tile
static uint8_t seen[(MAP_W * MAP_H) / 8];     // 64 bytes, explored
static uint8_t vis[(MAP_W * MAP_H) / 8];      // 64 bytes, visible now

static mon_t   mons[MAX_MON];
static item_t  items[MAX_ITEM];
static uint8_t n_mon, n_item;

static uint8_t px, py, facing;                // facing: 0=N 1=E 2=S 3=W
static uint8_t hp, hp_max, mp, depth, gold, potions;
static uint8_t sel;                           // selected inventory slot
static uint8_t power = 3;                     // 1..5, comes from the slider
static uint16_t turns;

static uint8_t cam_x, cam_y;
static char status_buf[VIEW_W + 1];
static char msg_buf[VIEW_W + 1];

static const char INV_NAME[INV_N][4] = { "DAG", "SWD", "AXE", "STF", "POT" };
static const int8_t DIRX[4] = { 0, 1, 0, -1 };
static const int8_t DIRY[4] = { -1, 0, 1, 0 };

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
      if (++tries > 30) {
        rooms[i].w = 4; rooms[i].h = 2;
        rooms[i].x = (uint8_t)(2 + i * 6);
        rooms[i].y = (uint8_t)(2 + (i & 1) * 5);
        break;
      }
    }
    carve_room(&rooms[i]);
  }

  for (i = 1; i < nrooms; i++) {
    uint8_t cx0 = (uint8_t)(rooms[i - 1].x + rooms[i - 1].w / 2);
    uint8_t cy0 = (uint8_t)(rooms[i - 1].y + rooms[i - 1].h / 2);
    uint8_t cx1 = (uint8_t)(rooms[i].x + rooms[i].w / 2);
    uint8_t cy1 = (uint8_t)(rooms[i].y + rooms[i].h / 2);
    hcorr(cx0, cx1, cy0);
    vcorr(cy0, cy1, cx1);
  }

  px = (uint8_t)(rooms[0].x + rooms[0].w / 2);
  py = (uint8_t)(rooms[0].y + rooms[0].h / 2);

  mset((uint8_t)(rooms[nrooms - 1].x + rooms[nrooms - 1].w / 2),
       (uint8_t)(rooms[nrooms - 1].y + rooms[nrooms - 1].h / 2), TER_STAIR);

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
  int8_t dx, dy, x, y;
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

// ===========================================================================
// Analog controls
// ===========================================================================

// ---- knob -> facing --------------------------------------------------------
// PD4 sits on the RV09 wiper: GND and VCC on the track ends, so the reading is
// a ratiometric 0..1023 that does not drift with the battery. Averaging four
// samples removes ADC jitter, which matters because a jittering knob would make
// the @ spin on a sector boundary.
static uint8_t knob_lo = 255, knob_hi = 0;

static uint8_t knob_read(void) {
  uint16_t s = 0;
  uint8_t i;
  for (i = 0; i < 4; i++) s += retro_knob();
  return (uint8_t)(s >> 2);
}

static uint8_t knob_sector(void) {
  uint8_t k = knob_read();
  uint8_t span;
  uint16_t q;

  if (k < knob_lo) knob_lo = k;
  if (k > knob_hi) knob_hi = k;
  span = (uint8_t)(knob_hi - knob_lo);

  if (span >= 90) q = (uint16_t)(k - knob_lo) * 4u / (span + 1u);  // self ranging
  else            q = (uint16_t)k * 4u / 256u;                     // assume full swing
  if (q > 3) q = 3;
  return (uint8_t)q;
}

// ---- slider -> power / magic strength --------------------------------------
// PD6 sits on the U5 B10K wiper. NOTE: on the current PCB revision U5 is wired
// wrongly (VCC is on the wiper instead of an end), so this input reads a
// constant until the two traces below the pot are cut and jumpered - see
// PCB_Design_Review.md. The game detects the pinning and falls back to DOWN.
static uint8_t slider_bad = 0;

static uint8_t slider_power(void) {
  uint8_t s = retro_slider();                    // 0..255
  if (s > 8 && s < 248) slider_bad = 0;          // it left the rail -> it works
  return (uint8_t)(1 + (((uint16_t)s * 5u) >> 8));   // -> 1..5
}

static void slider_probe(void) {
  uint8_t i, v, lo = 255, hi = 0;
  for (i = 0; i < 8; i++) {
    v = retro_slider();
    if (v < lo) lo = v;
    if (v > hi) hi = v;
    DLY_ms(3);
  }
  slider_bad = ((uint8_t)(hi - lo) < 6 && (lo > 248 || hi < 8)) ? 1 : 0;
}

// -------------------------------------------------------------- combat -----
static const char MON_CH[4] = { 'R', 'G', 'S', 'O' };
static const uint8_t MON_ATK[4] = { 1, 2, 2, 3 };

// melee damage for the selected weapon at the current power setting
static uint8_t melee_damage(void) {
  switch (sel) {
    case INV_DAG: return (uint8_t)(2 + (power >> 1));
    case INV_AXE: return (uint8_t)(4 + 2 * power);
    default:      return (uint8_t)(3 + power);        // sword
  }
}

static uint8_t spell_range(void)  { return (uint8_t)(1 + power); }
static uint8_t spell_damage(void) { return (uint8_t)(2 * power); }

static void kill_or_hurt(uint8_t mi, uint8_t dmg, const char *hit) {
  if (dmg >= mons[mi].hp) {
    mons[mi].hp = 0;
    retro_beep(140, 55);
    msg_set("YOU KILL IT");
  } else {
    mons[mi].hp = (uint8_t)(mons[mi].hp - dmg);
    retro_beep(300, 12);
    msg_set(hit);
  }
}

static void hurts_player(uint8_t amount, const char *what) {
  hp = (amount >= hp) ? 0 : (uint8_t)(hp - amount);
  retro_beep(110, 70);
  msg_set(what);
}

static void monsters_turn(void) {
  uint8_t i;
  for (i = 0; i < n_mon; i++) {
    int8_t dx, dy, sx, sy;
    uint8_t nx, ny, step;
    if (!mons[i].hp) continue;
    if (!bitget(vis, mons[i].x, mons[i].y)) continue;   // asleep until seen

    dx = (int8_t)((int8_t)px - (int8_t)mons[i].x);
    dy = (int8_t)((int8_t)py - (int8_t)mons[i].y);

    if (iabs8(dx) <= 1 && iabs8(dy) <= 1 && (dx || dy)) {
      hurts_player(MON_ATK[mons[i].kind], "IT HITS YOU");
      if (!hp) return;
      continue;
    }
    sx = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
    sy = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);
    if (!sx && !sy) continue;

    for (step = 0; step < 2; step++) {
      if (step == 0) {
        if (iabs8(dx) >= iabs8(dy)) { nx = (uint8_t)(mons[i].x + sx); ny = mons[i].y; }
        else                        { nx = mons[i].x; ny = (uint8_t)(mons[i].y + sy); }
      } else {
        if (iabs8(dx) >= iabs8(dy)) { nx = mons[i].x; ny = (uint8_t)(mons[i].y + sy); }
        else                        { nx = (uint8_t)(mons[i].x + sx); ny = mons[i].y; }
      }
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
  uint8_t nx = (uint8_t)(px + DIRX[facing]);
  uint8_t ny = (uint8_t)(py + DIRY[facing]);
  uint8_t mi;

  if (nx >= MAP_W || ny >= MAP_H) { retro_beep(90, 20); msg_set("ROCK AHEAD"); return; }
  mi = mon_at(nx, ny);
  if (mi != 0xFF) {
    if (sel == INV_AXE && power < 3) { msg_set("AXE TOO HEAVY"); retro_beep(90, 40); return; }
    kill_or_hurt(mi, melee_damage(), "YOU HIT IT");
    turns++;
    return;
  }
  if (!walkable(nx, ny)) { retro_beep(90, 20); msg_set("ROCK AHEAD"); return; }
  px = nx;
  py = ny;
  turns++;
  retro_beep(1000, 5);
}

// magic: a bolt that travels along the facing direction, range and damage both
// come from the slider. This is what makes the knob and the slider work together.
static void cast_spell(void) {
  uint8_t i, x, y;

  if (mp < power) { msg_set("NOT ENOUGH MP"); retro_beep(90, 40); turns++; return; }
  mp = (uint8_t)(mp - power);

  for (i = 1; i <= spell_range(); i++) {
    uint8_t mi;
    x = (uint8_t)(px + DIRX[facing] * (int8_t)i);
    y = (uint8_t)(py + DIRY[facing] * (int8_t)i);
    if (x >= MAP_W || y >= MAP_H) break;
    if (mget(x, y) == TER_WALL) break;
    mi = mon_at(x, y);
    if (mi != 0xFF) {
      retro_beep(1800, 25);
      kill_or_hurt(mi, spell_damage(), "THE BOLT HITS");
      turns++;
      return;
    }
  }
  retro_beep(500, 40);
  msg_set("THE BOLT FIZZLES");
  turns++;
}

static void swing(void) {
  uint8_t nx = (uint8_t)(px + DIRX[facing]);
  uint8_t ny = (uint8_t)(py + DIRY[facing]);
  uint8_t mi;

  if (nx >= MAP_W || ny >= MAP_H) { msg_set("SWING AT ROCK"); turns++; return; }
  mi = mon_at(nx, ny);
  if (mi == 0xFF) { msg_set("NOTHING THERE"); retro_beep(300, 8); turns++; return; }
  if (sel == INV_AXE && power < 3) { msg_set("AXE TOO HEAVY"); retro_beep(90, 40); return; }
  kill_or_hurt(mi, melee_damage(), "YOU HIT IT");
  turns++;
}

static void drink(void) {
  if (!potions) { msg_set("NO POTION LEFT"); retro_beep(90, 30); turns++; return; }
  potions--;
  hp = (uint8_t)((hp + 6 > hp_max) ? hp_max : hp + 6);
  mp = (uint8_t)((mp + 3 > MP_MAX) ? MP_MAX : mp + 3);
  retro_beep(1400, 20);
  msg_set("GLUG GLUG");
  turns++;
}

static void do_act(void) {
  uint8_t ii = item_at(px, py);

  if (ii != 0xFF) {                       // standing on something: pick it up
    if (items[ii].kind == I_POTION) {
      if (potions < 9) potions++;
      msg_set("POTION STASHED");
    } else {
      gold = (uint8_t)(gold + 5 + retro_rnd_n(10));
      msg_set("YOU GOT GOLD");
    }
    items[ii].kind = 0xFF;
    retro_beep(1400, 20);
    turns++;
    return;
  }
  if (mget(px, py) == TER_STAIR) {         // descend
    depth++;
    hp = (uint8_t)((hp + 3 > hp_max) ? hp_max : hp + 3);
    if (mp < MP_MAX) mp++;
    gen_level();
    fov_update();
    retro_beep(600, 40);
    retro_beep(900, 40);
    retro_beep(1300, 60);
    msg_num("FLOOR ", depth);
    turns++;
    return;
  }
  switch (sel) {
    case INV_STF: cast_spell(); break;
    case INV_POT: drink();      break;
    default:      swing();      break;
  }
}

static void cycle_select(void) {
  sel = (uint8_t)((sel + 1) % INV_N);
  retro_beep(1200, 6);
  msg_set(INV_NAME[sel]);
}

// -------------------------------------------------------------- drawing ----
static uint8_t cell_code(uint8_t x, uint8_t y) {
  uint8_t mi, ii, t;

  if (x == px && y == py) return (uint8_t)(G_PL_N + facing);
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

// HUD: "HP12 M10 P4 SWD" - hit points, mana, power setting, weapon
static void build_status(void) {
  uint8_t i = 0;
  status_buf[i++] = 'H';
  status_buf[i++] = 'P';
  status_buf[i++] = (char)('0' + (hp / 10) % 10);
  status_buf[i++] = (char)('0' + hp % 10);
  status_buf[i++] = ' ';
  status_buf[i++] = 'M';
  status_buf[i++] = (char)('0' + (mp / 10) % 10);
  status_buf[i++] = (char)('0' + mp % 10);
  status_buf[i++] = ' ';
  status_buf[i++] = 'P';
  status_buf[i++] = (char)('0' + power);
  status_buf[i++] = ' ';
  status_buf[i++] = INV_NAME[sel][0];
  status_buf[i++] = INV_NAME[sel][1];
  status_buf[i++] = INV_NAME[sel][2];
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
// Waits for something worth redrawing. Returns 0 if only the knob or the slider
// moved (a free action - the caller just redraws), otherwise the button mask.
static uint8_t wait_event(void) {
  uint8_t b;

  for (;;) {
    uint8_t f = knob_sector();
    uint8_t p;

    if (f != facing) {
      facing = f;
      retro_beep(1500, 4);                 // short tick: you can feel the sector
      return 0;
    }
    p = slider_power();
    if (!slider_bad && p != power) {
      power = p;                           // live read-out, no turn spent
      return 0;
    }
    b = retro_buttons();
    if (b) {
      DLY_ms(12);
      if (retro_buttons() == b) {
        while (retro_buttons()) DLY_ms(5);
        return b;
      }
    }
    DLY_ms(4);
  }
}

// ------------------------------------------------------------- new game ----
static void new_game(void) {
  hp_max = 12;
  hp = hp_max;
  mp = 8;
  depth = 1;
  gold = 0;
  potions = 2;
  sel = INV_SWD;
  turns = 0;
  gen_level();
  fov_update();
  knob_lo = 255;                       // let the knob self range from scratch
  knob_hi = 0;
  facing = knob_sector();
  msg_set("CABNETBOY ROGUE");
}

static void game_over(void) {
  retro_beep(400, 120);
  retro_beep(300, 120);
  retro_beep(200, 160);
  retro_beep(120, 260);
  msg_num("DEAD SCORE ", gold);
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

  slider_probe();
  new_game();
  if (slider_bad) msg_set("SLIDER DEAD: FIX U5");

  for (;;) {
    render();
    key = wait_event();
    if (!key) continue;                  // knob / slider only, just redraw

    switch (key) {
      case BTN_UP:   try_step();      break;
      case BTN_LEFT: cycle_select();  break;
      case BTN_ACT:  do_act();        break;
      case BTN_DOWN:
        if (slider_bad) {                // fallback: no slider, cycle power
          power = (uint8_t)((power % 5) + 1);
          retro_beep(1000, 6);
        } else {
          msg_set("YOU WAIT");           // wait one turn
          retro_beep(400, 8);
          turns++;
        }
        break;
      default: break;
    }

    if (turns && (turns % 4 == 0) && mp < MP_MAX) mp++;   // slow mana regen
    monsters_turn();
    fov_update();

    if (!hp) { game_over(); new_game(); }
  }
}
