#include "internal.h"
#include "glad/glad.h"
#include "milkshake/milkshake.h"
#include <GL/gl.h>


void ms_init_window(int width, int height, const char* title, int flags) {
#	ifdef __linux__ 
	// TODO: compiler flag for forcing wayland
	SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
#	endif

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
	SDL_GL_MakeCurrent(G_core.window.handle, *G_core.window.gl_ctx);

	SDL_CaptureMouse(1);
	SDL_SetWindowRelativeMouseMode(G_core.window.handle, true);
	// SDL_WarpMouseInWindow(G_Window.handle, (float)window->width/2, (float)window->height/2);
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


  // maybe something like this would be better?
  // ms_mesh m = ms_new_mesh();
  // ms_mesh_add_buffer(&m, MS_BufferVertex, verts, 24, MS_VERTLAYOUT_POSCOL);
  // ms_mesh_add_buffer(&m, MS_BufferIndex, indices, 36);
  //
  // i need to be able to account for the following scenarios:
  //
  // interleaved and non-interleaved vertex buffer data
  //   - ms_mesh_add_buffer(&m, MS_BufferVertex, verts, 24, MS_VERTLAYOUT_POS);
  //   - ms_mesh_add_buffer(&m, MS_BufferVertex, verts, 24, MS_VERTLAYOUT_COL);
  //   - ms_mesh_add_buffer(&m, MS_BufferVertex, verts, 24, MS_VERTLAYOUT_UV);
  //
  // void ms_mesh_add_buffer(ms_mesh*, ms_buffer_type, void *, isize, layout) {
  //    
  // }
  // static vs dynamic vertex buffers
  // indexed vs non-indexed buffers


ms_mesh ms_create_mesh(const void* verts, isize num_verts, const float * indices, isize num_indices, ms_vertex_layout layout) {
	ms_mesh out = { 0 };
	out.num_verts = num_verts;
	out.num_indices = num_indices;

	glGenVertexArrays(1, &out.vao);
	glBindVertexArray(out.vao);

	glGenBuffers(1, &out.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, out.vbo);

	isize size = 0;
	if(layout.num_attribs == 1) {
		size = layout.attribs[0].size * _get_type_size(layout.attribs[0].type) * out.num_verts;
	} else {
		for(int i = 0; i < layout.num_attribs; i++) {
			ms_vertex_attrib attr = layout.attribs[i];
			size += attr.size * _get_type_size(attr.type) * out.num_verts;
		}
	}

	for(int i = 0; i < layout.num_attribs; i++) {
		ms_vertex_attrib attr = layout.attribs[i];
	  glBufferData(
	    GL_ARRAY_BUFFER,
	    out.num_verts * layout.attribs[0].size * sizeof(float),
	    verts,
	    GL_STATIC_DRAW
		);

		glEnableVertexAttribArray(attr.index);
		glVertexAttribPointer(
		  attr.index,
		  attr.size,
		  attr.type,
		  attr.normalized,
		  attr.stride,
		  (const void*) attr.offset
		);
	}
	
}
