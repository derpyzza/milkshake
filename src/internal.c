#include "internal.h"
#include "glad/glad.h"
#include "milkshake/milkshake.h"
#include <GL/gl.h>
#include <SDL3/SDL_video.h>

struct _core G_core;

void ms_clear_colour(u32 hex) {
	vec4s col = ms_col_from_hex(hex);
	glClearColor(col.r, col.g, col.b, col.a);
}

ms_vao ms_create_vao(void) {
	ms_vao out;
	glGenVertexArrays(1, &out.id);
	return out;
}

void ms_end_drawing(void) {
	SDL_GL_SwapWindow(G_core.window.handle);
}

void ms_cleanup(void) {
	glFinish();
	SDL_GL_DestroyContext(G_core.window.gl_ctx);
	SDL_DestroyWindow(G_core.window.handle);
}

void ms_init_window(int width, int height, const char* title, int flags) {
#	ifdef __linux__ 
	// TODO: compiler flag for forcing wayland
	SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
#	endif
	dlog_init(DLOG_DEBUG);

	G_core.window.width = width;
	G_core.window.height = height;
	G_core.window.title = (char*)title;

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
	G_core.window.handle = SDL_CreateWindow(
		G_core.window.title,
		G_core.window.width,
		G_core.window.height,
		window_flags
	);

	CRASH_SDL(G_core.window.handle == NULL, "Window is NULL");

	G_core.window.gl_ctx = SDL_GL_CreateContext(G_core.window.handle);
	if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) printf("Failed to load gl"), exit(-1);
	
	if( (flags & MS_WindowFlag_MsaaX4) == MS_WindowFlag_MsaaX4 ) {
	  glEnable(GL_MULTISAMPLE);
	}
	SDL_GL_MakeCurrent(G_core.window.handle, G_core.window.gl_ctx);
	// SDL_SetWindowRelativeMouseMode(G_core.window.handle, true);
	// vsync. 0 is off, 1 is on
	SDL_GL_SetSwapInterval(IS_FLAG_SET(flags, MS_WindowFlag_EnableVsync));	
}


void ms_update(void) {
  SDL_PumpEvents();
  int len;
  u8* keys = (u8*)SDL_GetKeyboardState(&len);
  memcpy(G_core.keyboard.prev_state, G_core.keyboard.keystate, len);
  memcpy(G_core.keyboard.keystate, keys, len);

  G_core.mouse.scroll = 0;
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
		case GL_FLOAT: return sizeof(float); break;
		case GL_INT: return sizeof(int); break;
		case GL_BYTE: return sizeof(char); break;
		// ... and so on
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
		  attr->stride,
		  (const void*) attr->offset
		);
	}
}

void ms_vao_attach_ebo(ms_vao *vao, ms_buffer buffer) {
	glBindVertexArray(vao->id);
	glBindBuffer(buffer.type, buffer.id);
}
