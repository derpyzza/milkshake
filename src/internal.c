#include "internal.h"
#include "glad/glad.h"
#include "milkshake/milkshake.h"
#include <GL/gl.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

struct _core G_core;

void ms_draw_elems(ms_vao vao, int prim_mode, int type, isize count, const int* start) {
	glBindVertexArray(vao.id);
	glDrawElements(prim_mode, count, type, start);
	G_core.stats.num_draw_calls++;
}

void ms_draw_arrays(ms_vao vao, int prim_mode, int index, int num_verts) {
	glBindVertexArray(vao.id);
	glDrawArrays(prim_mode, index, num_verts);
	G_core.stats.num_draw_calls++;
}

void ms_destroy_buffer(ms_buffer buffer) {
	glDeleteBuffers(1, &buffer.id);
}

void ms_destroy_vao(ms_vao vao) {
	glDeleteVertexArrays(1, &vao.id);
}

void ms_clear_colour(u32 hex) {
	vec4s col = ms_col_from_hex(hex);
	glClearColor(col.r, col.g, col.b, col.a);
}

ms_vao ms_create_vao(void) {
	ms_vao out;
	glGenVertexArrays(1, &out.id);
	return out;
}

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

static const isize _get_type_size(int type) {
	switch (type) {
		case GL_FLOAT:
			return sizeof(float);
		break;
		case GL_DOUBLE:
			return sizeof(double);
		break;

		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			return sizeof(short);
		break;

		case GL_INT:
		case GL_UNSIGNED_INT:
			return sizeof(int);
		break;

		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return sizeof(char);
		break;

		// yeah idk what to put here tbh
		default: return sizeof(float); break;
	}
}

ms_buffer ms_create_buffer(ms_buffertype type, ms_buffer_usage usage, isize size, const void * data) {
	ms_buffer out = {0};
	out.size = size;
	out.type = type;

	glGenBuffers(1, &out.id);
	glBindBuffer(type, out.id);
	glBufferData(type, size, data, usage);

	// unbind buffer, uhhhhh just because
	glBindBuffer(type, 0);
	return out;
}

void ms_vao_attach_vbo(ms_vao *vao, ms_buffer buffer, ms_vertex_layout layout) {
	glBindVertexArray(vao->id);
	glBindBuffer(buffer.type, buffer.id);

	dforeach(ms_vertex_attrib, attr, layout.attribs, layout.num_attribs) {
		glEnableVertexAttribArray(attr->index);

		glVertexAttribPointer(
		  attr->index,
		  attr->size,
		  attr->type,
		  attr->normalized,
		  layout.stride,
		  (const void*) attr->offset
		);
	}
}

void ms_vao_attach_ebo(ms_vao *vao, ms_buffer buffer) {
	glBindVertexArray(vao->id);
	glBindBuffer(buffer.type, buffer.id);
}
