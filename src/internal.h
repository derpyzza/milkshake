#pragma once
#include "milkshake/milkshake.h"
#include <SDL3/SDL_gamepad.h>

//
// TODO: wrap gl_ctx in a #ifdef guard that takes it out if the
// rendering backend being used is not an opengl one.
// im not sure what kind of alternative backends i'd like to add yet, but
// at the very least i'd like to add a software rendered backend.
// 
 
// these are just a bad habit at this point...
#define PANIC_SDL(CHECK, MSG) if (CHECK) { dlog_fatal(MSG "\nError: %s\n", SDL_GetError()); }
#define CRASH(CHECK,MSG) if (CHECK) { dlog_fatal(MSG"\nexiting now..."); exit(-1); }
#define CRASH_SDL(CHECK,MSG) if (CHECK) { dlog_fatal(MSG "\nError: %s\n", SDL_GetError()); exit(-1); }

#define MAX_GAMEPADS 8

struct _core {
  struct {
  	u8 last_pressed;
  	u8 last_released;
  	u8 prev_state [SDL_SCANCODE_COUNT];
  	u8 keystate   [SDL_SCANCODE_COUNT];
  } keyboard;

  struct {
  	vec2s pos;
  	vec2s pos_delta;

  	u8 prev_state;
  	u8 btn_state;

  	f32 scroll; // positive for up, negative for down
  	f32 prev_scroll;
  } mouse;

  struct {
    int id;

    u8 last_pressed;
    u8 last_released;
    u8 btn_state[SDL_GAMEPAD_BUTTON_COUNT];
    u8 prev_button_state[SDL_GAMEPAD_BUTTON_COUNT];
  } gamepad[MAX_GAMEPADS];

  struct {
    int width, height;
    bool focused, minimized;
    char title[MS_NAME_LEN];

    SDL_Window* handle;
    SDL_GLContext * gl_ctx;
  } window;
};

extern struct _core G_core;
