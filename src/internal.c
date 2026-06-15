/* milkshake
 * 
 * Copyright (c) 2026 derpyzza
 * 
 * This file is released under the terms of the MIT license.
 * Read LICENSE for more information.
 *
 * no LLMS were used to write the code in this library.
*/ 

#include "glad/glad.h"
#include "internal.h"
#include "milkshake/milkshake.h"
#include <GL/gl.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

struct _core G_core;

ms_window ms_init_window(int width, int height, const char* title, int flags) {
#	ifdef __linux__ 
	// TODO: compiler flag for forcing wayland
	SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
#	endif
	dlog_init(DLOG_DEBUG);

	ms_window out = {0};

	out.width = width;
	out.height = height;
	out.title = (char*)title;

	u32 sdl_flags =
	  	SDL_INIT_VIDEO
	  | SDL_INIT_AUDIO
	  | SDL_INIT_EVENTS
	  | SDL_INIT_GAMEPAD
	;
	CRASH_SDL(SDL_Init(sdl_flags) < 0, "Could not init SDL");

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 16 );

	u32 window_flags =
		  SDL_WINDOW_OPENGL
		| SDL_WINDOW_RESIZABLE
		| SDL_WINDOW_HIGH_PIXEL_DENSITY
		;
	out.handle = SDL_CreateWindow(
		out.title,
		out.width,
		out.height,
		window_flags
	);

	CRASH_SDL(out.handle == NULL, "Window is NULL");

	out.gl_ctx = SDL_GL_CreateContext(out.handle);
	if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) printf("Failed to load gl"), exit(-1);
	
	if( (flags & MS_WindowFlag_MsaaX4) == MS_WindowFlag_MsaaX4 ) {
	  glEnable(GL_MULTISAMPLE);
	}
	SDL_GL_MakeCurrent(out.handle, out.gl_ctx);
	// SDL_SetWindowRelativeMouseMode(out.handle, true);
	// vsync. 0 is off, 1 is on
	SDL_GL_SetSwapInterval(IS_FLAG_SET(flags, MS_WindowFlag_EnableVsync));	

	return out;
}


void ms_update(void) {
	G_core.stats.prev_num_draw_calls = G_core.stats.num_draw_calls;
	G_core.stats.num_draw_calls = 0;
	
	SDL_Event ev;
	while(SDL_PollEvent(&ev)) {
		switch(ev.type) {

			case SDL_EVENT_MOUSE_WHEEL: {
				f32 scroll = ev.wheel.y;
				if(ev.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) scroll *= -1;
				G_core.mouse.scroll_accum += scroll;
				break;
			}
		}
	}
	
  int len;
  u8* keys = (u8*)SDL_GetKeyboardState(&len);
  memcpy(G_core.keyboard.prev_state, G_core.keyboard.keystate, len);
  memcpy(G_core.keyboard.keystate, keys, len);

  G_core.mouse.prev_scroll = G_core.mouse.scroll;
  G_core.mouse.scroll = G_core.mouse.scroll_accum;
  G_core.mouse.scroll_accum = 0;
  G_core.mouse.prev_state = G_core.mouse.btn_state;
  vec2s prev_pos = G_core.mouse.pos;

  float mx, my;
  G_core.mouse.btn_state = SDL_GetRelativeMouseState(&mx, &my);

  G_core.mouse.pos.x += mx;
  G_core.mouse.pos.y += my;

  G_core.mouse.pos_delta = glms_vec2_sub(G_core.mouse.pos, prev_pos);
}
