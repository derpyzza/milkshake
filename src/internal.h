#pragma once
#include "milkshake/milkshake.h"

//
// FIXME: improve the shader caching system, it's kinda jank rn
// TODO: wrap gl_ctx in a #ifdef guard that takes it out if the
// rendering backend being used is not an opengl one.
// im not sure what kind of alternative backends i'd like to add yet, but
// at the very least i'd like to add a software rendered backend.
// 

#define MAX_SHADERS 64

typedef struct ms_window {
  int width, height;
  bool focused, minimized;
  dstr name;

  SDL_Window* handle;
  SDL_GLContext * gl_ctx;
} ms_window;


typedef struct ms_shader {
  int id;
  char name[MS_NAME_LEN];
  char *vert, *frag;
  int locs[MAX_SHADER_LOCS];
} ms_shader;


typedef struct ms_texture {
	int id;
	int width, height, nrChannel;

	ms_sampler sampler;

	dstr path; // filepath ( optional )
} ms_texture;


struct _core {

  struct {
  	u8 last_pressed;
  	u8 last_released;
  	u8 prev_state [SDL_SCANCODE_COUNT];
  	u8 keystate   [SDL_SCANCODE_COUNT];
  } keyboard;

  struct {
  	ivec2 pos;
  	vec2 pos_delta;

  	u8 prev_state;
  	u8 btn_state;

  	f32 scroll; // positive for up, negative for down
  	f32 prev_scroll;
  } mouse;

  struct {} gamepad;

  d_arena arena;

  ms_shader * shader_buffer;
  ms_texture * texture_buffer;
} G_core;

static struct {
  char name[128];
  int value;
}shader_cache [MAX_SHADERS];
static int shader_cache_index = 0;

