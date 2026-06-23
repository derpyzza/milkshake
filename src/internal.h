/* milkshake
 * 
 * Copyright (c) 2026 derpyzza
 * 
 * This file is released under the terms of the MIT license.
 * Read LICENSE for more information.
 *
 * no LLMS were used to write the code in this library.
*/ 

#pragma once
#include "milkshake/milkshake.h"
#include <SDL3/SDL_gamepad.h>

// these are just a bad habit at this point...
#define PANIC_SDL(CHECK, MSG) if (CHECK) { dlog_fatal(MSG "\nError: %s\n", SDL_GetError()); }
#define CRASH(CHECK,MSG) if (CHECK) { dlog_fatal(MSG"\nexiting now..."); exit(-1); }
#define CRASH_SDL(CHECK,MSG) if (CHECK) { dlog_fatal(MSG "\nError: %s\n", SDL_GetError()); exit(-1); }

#define MS_MAX_GAMEPADS 8

struct _core {
  struct {
  	int last_pressed;
  	int last_released;
  	u8 prev_state [MS_KEY_COUNT];
  	u8 keystate   [MS_KEY_COUNT];
  } keyboard;

  struct {
  	vec2s pos;
  	vec2s pos_delta;

  	u8 prev_state;
  	u8 btn_state;

  	f32 scroll_accum;
  	f32 scroll; // positive for up, negative for down
  	f32 prev_scroll;
  } mouse;

  struct {
    int id;

    u8 last_pressed;
    u8 last_released;
    u8 btn_state[SDL_GAMEPAD_BUTTON_COUNT];
    u8 prev_button_state[SDL_GAMEPAD_BUTTON_COUNT];
  } gamepad[MS_MAX_GAMEPADS];

  struct {
    int num_draw_calls;      // number of draw calls made this frame
    int prev_num_draw_calls; // number of draw calls made in the previous frame
  } stats;

  bool should_quit;
};

extern struct _core G_core;
