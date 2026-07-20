/* milkshake
 * small opengl framework for gamedev
 * 
 * Copyright (c) 2026 derpyzza
 * 
 * This file is released under the terms of the MIT license.
 * Read LICENSE for more information.
 *
 * no LLMS were used to write the code in this library.
*/ 

#pragma once
#include <SDL3/SDL.h>
#include <libderp/derp.h>
#include <cglm/struct.h>
#include "glad/glad.h"
#include <GL/gl.h>


// NOTE:
// 	> maybe instead of having fixed length name strings i could instead have a
//    library-wide arena for strings?
// 
// 
// TODO:
//  > implement support for textures that aren't just 2D textures
// 	> implement a basic text renderer that uses the spritebatch functions
// 	> implement instanced drawing functions
// 	> implement gamepad functions
// 	> wrap G_core.gl_ctx in a #ifdef guard that takes it out if the
// 		rendering backend being used is not an opengl one.
// 		im not sure what kind of alternative backends i'd like to add yet, but
// 		at the very least i'd like to add a software rendered backend.
// 	> introduce an ms_shader_stage type that's analogous to the opengl shader
// 		type ( as opposed to the shader program type — ms_shader ), so that users
// 		can cache compiled shaders themselves instead of making the program
// 		repeatedly recompile previously compiled shaders.
// 	> allow shader programs ( ms_shader ) to have more than two shaders
// 		attached to them. using some sort of shader descriptor similar to the
// 		vertex layout descriptor struct. this will be used to implement reusable
// 		shader code / shader libraries that you can shader across different
// 		programs.
// 		NOTE: this behaviour is supported on desktop but not on web and mobile.
// 
// 
// FIXME:
// 	> ms_create_shader needs some sort of error throwing functionality.
//    currently it logs an error but it needs to return some sort of
//    error value that can be checked too.
// 	> some parts of the library use the opengl 4.5 DSA api,
//    ( in particular, the ms_texture.c functions ). this is undesirable.
// 		in the future i'd like to add support for multiple opengl versions,
//    particularly: core3.3, core4.x, GLES3, webgl2, and the library
//    should be structured in a way that supports all those apis. however,
//    for now, i should pick an api version and stick to it, and i don't
//    want to use core4.5 for the current version of the library.
// 		i should probably just use core3.3 as the default desktop version with core3.0 as
// 		the default web/android version
// 	> wrap sampler values into an enum
// 	> wrap texture types into an enum
// 		

// {{ DEFINES
#define MS_VERSION "0.0.0"
#define MS_NAME_LEN 128     /* string length for name fields in structs to avoid having to allocate strings everywhere */
#define MAX_SHADER_LOCS 32  /* maximum uniform locations a shader can have */
#define MAX_SHADERS 64

#define MS_BLACK      0x161616ff
#define MS_WHITE      0xe9e4d9ff
#define MS_GREY       0x585550ff
#define MS_RED        0xf3565dff
#define MS_PINK       0xdd7ab3ff
#define MS_PURPLE     0x9756dfff
#define MS_BLUE       0x6181f0ff
#define MS_CYAN       0x5ddac5ff
#define MS_GREEN      0xa6e25aff
#define MS_YELLOW     0xe6d957ff
#define MS_ORANGE     0xf89646ff

#define MS_DARK_WHITE  0xa4a097ff
#define MS_DARK_GREY   0x3d3b37ff     
#define MS_DARK_RED    0xae2f43ff
#define MS_DARK_PINK   0xba569cff
#define MS_DARK_PURPLE 0x513391ff    
#define MS_DARK_BLUE   0x4249c4ff   
#define MS_DARK_CYAN   0x3999a7ff
#define MS_DARK_GREEN  0x5ba936ff    
#define MS_DARK_YELLOW 0xc2a53eff    
#define MS_DARK_ORANGE 0xb15c2aff    

#define IS_FLAG_SET(n, f) ((n) & (f)) == (f)
#define WRAP(f, x, y) if(f < x){ f = y; } else if(f>y){f = x;} 
// }} DEFINES

// {{ ENUMS

#ifdef __cplusplus
	extern "C" {
#endif

#include "input_enums.h"

enum ms_window_flags {
	MS_WindowFlag_Resizable    = 1,
	MS_WindowFlag_Centered     = 1<<1,
	MS_WindowFlag_MsaaX4       = 1<<2,
	MS_WindowFlag_EnableVsync  = 1<<3,

	MS_WINDOWFLAG_COUNT
};

typedef enum ms_uniform_type{
	MS_Uniform_Float,
	MS_Uniform_Float2,
	MS_Uniform_Float3,
	MS_Uniform_Float4,

	MS_Uniform_Int,
	MS_Uniform_Int2,
	MS_Uniform_Int3,
	MS_Uniform_Int4,

	MS_Uniform_Uint,
	MS_Uniform_Uint2,
	MS_Uniform_Uint3,
	MS_Uniform_Uint4,

	MS_Uniform_Sampler2D,

	MS_UNIFORM_COUNT
} ms_uniform_type;

enum ms_vertex_format_type {
	MS_VertexFormat_Float,
	MS_VertexFormat_Float2,
	MS_VertexFormat_Float3,
	MS_VertexFormat_Float4,

	MS_VertexFormat_Int,
	MS_VertexFormat_Int2,
	MS_VertexFormat_Int3,
	MS_VertexFormat_Int4,

	MS_VertexFormat_Uint,
	MS_VertexFormat_Uint2,
	MS_VertexFormat_Uint3,
	MS_VertexFormat_Uint4,

	MS_VertexFormat_Sampler2D,

	MS_VERTEXFORMAT_COUNT
};
 

// these are modelled off of the default vertex attributes
// that gltf supports ( as they're the most common attributes you'll use )
typedef enum ms_vert_attr_kinds {
  MS_VertKind_Invalid,
  // vec3 float
  MS_VertKind_Position,
  // vec3 float
  MS_VertKind_Normal,
  // vec4 float
  MS_VertKind_Tangent,
  // vec2 float
  MS_VertKind_TexCoord_N,
  // vec4 float or uint32_t modelled as a vec4 ubyte
  MS_VertKind_Colour_N,
  // vec4 int
  MS_VertKind_Weights_N,
  // vec4 float
  MS_VertKind_Joints_N,

  MS_VertKind_Custom_N,

  MS_VERTKIND_COUNT
} ms_vert_attr_kinds;

typedef enum ms_cull_mode {
  MS_CullMode_None,
  MS_CullMode_Back,
  MS_CullMode_Front,
} ms_cull_mode;

typedef enum ms_blend_mode {
  MS_BlendMode_Opaque,
  MS_BlendMode_Alpha,
  MS_BlendMode_Mask,
} ms_blend_mode;

typedef enum ms_draw_primitives {
  MS_PrimMode_Points        = GL_POINTS,
  MS_PrimMode_Lines         = GL_LINES,
  MS_PrimMode_Triangle      = GL_TRIANGLES,
  MS_PrimMode_TriangleStrip = GL_TRIANGLE_STRIP,
  MS_PrimMode_TriangleFan   = GL_TRIANGLE_FAN,

  MS_PRIMMODE_COUNT,
} ms_draw_primitives;

typedef enum ms_poly_mode {
  MS_PolyMode_Fill,
  MS_PolyMode_Wire
} ms_poly_mode;

typedef enum ms_buffertype {
	MS_BufferType_Invalid,
	MS_BufferType_Vertex        = GL_ARRAY_BUFFER,
	MS_BufferType_Index         = GL_ELEMENT_ARRAY_BUFFER,
	MS_BufferType_ShaderStorage = GL_SHADER_STORAGE_BUFFER,
	MS_BufferType_Uniform       = GL_UNIFORM_BUFFER,
} ms_buffertype;

typedef enum ms_buffer_usage {
	MS_BufferUsage_StreamDraw = GL_STREAM_DRAW,
	MS_BufferUsage_StreamRead = GL_STREAM_READ,
	MS_BufferUsage_StreamCopy = GL_STREAM_COPY,

	MS_BufferUsage_StaticDraw = GL_STATIC_DRAW,
	MS_BufferUsage_StaticRead = GL_STATIC_READ,
	MS_BufferUsage_StaticCopy = GL_STATIC_COPY,

	MS_BufferUsage_DynamicDraw = GL_DYNAMIC_DRAW,
	MS_BufferUsage_DynamicRead = GL_DYNAMIC_READ,
	MS_BufferUsage_DynamicCopy = GL_DYNAMIC_COPY,
} ms_buffer_usage;

enum ms_attachment {
	MS_ATTACHMENT_COLOR_N = GL_COLOR_ATTACHMENT0,
	MS_ATTACHMENT_DEPTH = GL_DEPTH_ATTACHMENT,
	MS_ATTACHMENT_STENCIL = GL_STENCIL_ATTACHMENT,
	MS_ATTACHMENT_DEPTH_STENCIL = GL_DEPTH_STENCIL_ATTACHMENT
};

// }} ENUMS

// {{ TYPES

typedef struct {
  int width, height;
  bool focused, minimized;
  char* title;

  SDL_Window* handle;
  SDL_GLContext gl_ctx;
} ms_window;

// typesafe wrappers cuz no one likes naked ints
typedef struct {
	u32 id;
} ms_shader;

typedef struct {
	u32 id;
} ms_uniform;

typedef struct ms_vao {
	u32 id;
} ms_vao;

typedef struct ms_framebuffer {
	u32 id;
} ms_framebuffer;

typedef struct ms_renderbuffer {
	u32 id;
} ms_renderbuffer;

typedef struct {
	int wrap_t, wrap_s;
	int min_filter, max_filter;
} ms_sampler;

typedef struct ms_texture {
	u32 id;
	int width, height, nrChannel;

	// NOTE:
	// do i even need to store this in the texture object?
	ms_sampler sampler;

	char path[MS_NAME_LEN]; // filepath ( optional )
} ms_texture;

// pipeline state object, similar to the kinds of pipeline objects found in modern graphics apis like Vulkan, or sokol_gfx
// NOTE:
// Should i get rid of this?
// along with all the pipeline state structs? i don't really use them much rn
typedef struct ms_pipeline {
  ms_shader * shader;
  ms_cull_mode cullmode;
  ms_blend_mode blend_mode;
  ms_draw_primitives primitives;
  ms_poly_mode poly_mode;
} ms_pipeline;


typedef struct ms_vertex_attrib {
	int index;
	int size;
	int type;
	bool normalized;
	isize offset;

	// NOTE:
	// this field is totally optional and is a convenience feature
	// it exists so that the default gltf loader can automatically
	// populate a vertex_layout struct correctly.
	// 
	// if you're manually specifying vertex layouts or loading models
	// by hand, then feel free to not use this field if you don't want
	// to
	ms_vert_attr_kinds attr_kind;
} ms_vertex_attrib;


typedef struct ms_vertex_layout {
	isize num_attribs;
	int stride; // shared across the entire vbo
		          // if 0, then it's a
	ms_vertex_attrib * attribs;
} ms_vertex_layout;


typedef struct ms_buffer {
	u32 id;
	ms_buffertype type;
	isize size;
} ms_buffer;

typedef struct ms_mesh {
	ms_vao vao;
	ms_buffer *vbo; // may either be a single buffer or an array of buffers
	ms_buffer *ebo; // may be null

	size_t num_buffers;
	
	size_t num_verts, num_indices;
} ms_mesh;

typedef struct ms_camera {	
	vec3s   pos;
	versors rot;
	vec3s   target;

	vec3s front, forward, right;

	f32 near, far;
	f32 fov, aspect;

	mat4s view, proj;
} ms_camera;

static const ms_vertex_layout MS_VERTLAYOUT_POSCOL = {
	.num_attribs = 2,
	.stride = 7 * sizeof(float),
	.attribs = (ms_vertex_attrib[]){
		{
			.index = 0,
			.size = 3,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 0,
			.attr_kind = MS_VertKind_Position,
		},
		{
			.index = 1,
			.size = 4,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 3 * sizeof(float),
			.attr_kind = MS_VertKind_Colour_N,
		}
	}
};

static const ms_vertex_layout MS_VERTLAYOUT_POSUV = {
	.num_attribs = 2,
	.stride = 5 * sizeof(float),
	.attribs = (ms_vertex_attrib[]){
		{
			.index = 0,
			.size = 3,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 0,
			.attr_kind = MS_VertKind_Position,
		},
		{
			.index = 1,
			.size = 2,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 3 * sizeof(float),
			.attr_kind = MS_VertKind_TexCoord_N,
		}
	}
};

static const ms_vertex_layout MS_VERTLAYOUT_POSUVCOL = {
	.num_attribs = 2,
	.stride = 9 * sizeof(float),
	.attribs = (ms_vertex_attrib[]){
		{
			.index = 0,
			.size = 3,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 0,
			.attr_kind = MS_VertKind_Position,
		},
		{
			.index = 1,
			.size = 2,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 3 * sizeof(float),
			.attr_kind = MS_VertKind_TexCoord_N,
		},
		{
			.index = 2,
			.size = 4,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 5 * sizeof(float),
			.attr_kind = MS_VertKind_Colour_N,
		}
	}
};

static const ms_vertex_layout MS_VERTLAYOUT_LIT = {
	.num_attribs = 2,
	.stride = 12 * sizeof(float),
	.attribs = (ms_vertex_attrib[]){
		// position, vec3f
		{
			.index = 0,
			.size = 3,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 0,
			.attr_kind = MS_VertKind_Position,
		},
		// normal, vec3f
		{
			.index = 1,
			.size = 3,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 3 * sizeof(float),
			.attr_kind = MS_VertKind_Normal,
		},
		// uv, vec2f
		{
			.index = 2,
			.size = 2,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 6 * sizeof(float),
			.attr_kind = MS_VertKind_TexCoord_N,
		},
		// colour, vec4f
		{
			.index = 3,
			.size = 4,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 8 * sizeof(float),
			.attr_kind = MS_VertKind_Colour_N,
		}
	}
};

static const ms_vertex_layout MS_VERTLAYOUT_SKINNED = {
	.num_attribs = 2,
	.stride = 16 * sizeof(float) + 4 * sizeof(int),
	.attribs = (ms_vertex_attrib[]){
		// position, vec3f
		{
			.index = 0,
			.size = 3,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 0,
			.attr_kind = MS_VertKind_Position,
		},
		// normal, vec3f
		{
			.index = 1,
			.size = 3,
			.type = GL_FLOAT,
			.normalized = true,
			.offset = 3 * sizeof(float),
			.attr_kind = MS_VertKind_Normal,
		},
		// uv, vec2f
		{
			.index = 2,
			.size = 2,
			.type = GL_FLOAT,
			.normalized = false,
			.offset = 6 * sizeof(float),
			.attr_kind = MS_VertKind_TexCoord_N,
		},
		// colour, vec4f
		{
			.index = 3,
			.size = 4,
			.type = GL_FLOAT,
			.normalized = true,
			.offset = 8 * sizeof(float),
			.attr_kind = MS_VertKind_Colour_N,
		},
		// joints, vec4i
		{
			.index = 4,
			.size = 4,
			.type = GL_INT,
			.normalized = false,
			.offset = 12 * sizeof(float),
			.attr_kind = MS_VertKind_Joints_N,
		},
		// weights, vec4f
		{
			.index = 5,
			.size = 4,
			.type = GL_FLOAT,
			.normalized = true,
			.offset = 12 * sizeof(float) + 4 * sizeof(int),
			.attr_kind = MS_VertKind_Weights_N,
		},
	}
};



// }} TYPES

// {{ FUNCTION PROTOTYPES

// NOTE:
// all the colour functions deal with normalized colours
// and will automatically normalize their inputs and outputs
static inline vec4s ms_col_from_hex(u32 hex){
  return (vec4s) {{
	  ((hex >> 24) & 0xFF) / 255.0f,
    ((hex >> 16) & 0xFF) / 255.0f,
    ((hex >> 8)  & 0xFF) / 255.0f,
     (hex        & 0xFF) / 255.0f 
  }};
}

#define ms_hex_to_bytes(hex) \
	{ \
		hex >> 24 & 0xFF, \
		hex >> 16 & 0xFF, \
		hex >>  8 & 0xFF, \
		hex       & 0xFF, \
	}

static inline u32 ms_col_to_hex(vec4s c) {
	c = glms_vec4_normalize(c);
  return ((uint)c.x << 24) & ((uint)c.y << 16) & ((uint)c.z << 8) & (uint)c.w;
}

static inline vec4s ms_col(u8 r, u8 g, u8 b, u8 a) {
	return (vec4s) {{
		(float)r/255.0f, (float)g/255/0.f, (float)b/255/0.f, (float)a/255/0.f
	}};
}

static inline vec4s ms_col_f(f32 r, f32 g, f32 b, f32 a) {
	if( r > 1 ) r /= 255.0f;
	if( g > 1 ) g /= 255.0f;
	if( b > 1 ) b /= 255.0f;
	if( a > 1 ) a /= 255.0f;
	return (vec4s){{r,g,b,a}};
}

typedef fnptr(void, handle_events_cb, SDL_Event);

// create and initialize a window
ms_window ms_init_window( int width, int height, const char* title, int flags );
// update internal library state, like input state etc.
void ms_update(void);
// returns true if an application quit event is recieved ( ex. when the
// close button is pressed on a window )
bool ms_should_quit(void);
// set a custom callback to be able to handle SDL events yourself.
void ms_set_event_callback(handle_events_cb cb);

// convenience function to be able to use hex codes for the
// background clear colour.
void ms_clear_colour(u32 hex);
// returns number of drawcalls made this frame.
// NOTE: make sure to run this function either before you call
// ms_update, or right after all your drawing's done, as ms_update
// clears the drawcall count for the next frame.
int ms_num_drawcalls(void);

// === SHADERS ===
// reads shader code from provided file paths and compiles and links the shader program.
ms_shader ms_create_shader( const char * vert_file, const char * frag_file );
// creates and compiles a shader program with the given source strings.
ms_shader ms_create_shader_from_source( dstr vert_source, dstr frag_source );
void ms_destroy_shader (ms_shader shader);
void ms_bind_shader    (ms_shader shader);
// get uniform location by it's name
ms_uniform ms_get_uniform(ms_shader, const char* name);

// the following are all type-safe wrappers over the ms_shader_set_value functions
// i've selected just the most common data types for the type-safe wrappers because
// i don't want to manually write wrappers over all the other types since they don't
// get used that often anyway.
void ms_shader_set_int_v   (ms_shader, ms_uniform, int*, isize count);
void ms_shader_set_float_v (ms_shader, ms_uniform, float*, isize count);
void ms_shader_set_ivec4   (ms_shader, ms_uniform, ivec4s*);
void ms_shader_set_ivec4_v (ms_shader, ms_uniform, ivec4s*, isize count);
void ms_shader_set_vec4    (ms_shader, ms_uniform, vec4s*);
void ms_shader_set_vec4_v  (ms_shader, ms_uniform, vec4s*, isize count);

void ms_shader_set_value   ( ms_shader, ms_uniform loc, const void * data, ms_uniform_type val_type );
void ms_shader_set_value_v ( ms_shader, ms_uniform loc, const void * data, isize count, enum ms_uniform_type val_type );

void ms_shader_set_mat4    (ms_shader, ms_uniform, mat4s*, bool);
void ms_shader_set_mat4_v  (ms_shader, ms_uniform, mat4s*, isize count, bool);

// === TEXTURES ===
// these functions take in an optional pointer to a sampler
// passing in NULL uses a default sampler value instead.
ms_texture ms_create_texture( int w, int h, int infmt, int outfmt, const void* data, const ms_sampler * sampler );
ms_texture ms_load_texture( const char* path, const ms_sampler * sampler );
ms_texture ms_load_texture_from_memory( const u8* data, int data_len, const ms_sampler * sampler );
void ms_bind_texture(ms_texture texture);
// UNIMPLEMENTED:
void ms_bind_texture_slot(ms_texture texture, uint slot);
void ms_destroy_texture(ms_texture tex);

// === VERTEX ARRAYS AND BUFFERS ===
ms_vao ms_create_vao(void);
void ms_destroy_vao(ms_vao vao);

void ms_bind_vao(ms_vao vao);
void ms_vao_attach_vbo(ms_vao vao, ms_buffer buffer, ms_vertex_layout layout);
void ms_vao_attach_ebo(ms_vao vao, ms_buffer buffer);

// NOTE:
// if usage is any of the DYNAMIC_* types then data may be NULL
ms_buffer ms_create_buffer(ms_buffertype type, ms_buffer_usage usage, isize size, const void * data);
void ms_destroy_buffer(ms_buffer buffer);

// === FRAMEBUFFERS ===
ms_framebuffer ms_create_fbo (void);
void           ms_destroy_fbo(ms_framebuffer);
void           ms_bind_fbo   (ms_framebuffer);
// binds 0 (the default screen)
void           ms_unbind_fbo (void);

// `num` is only used in case the attachment is MS_ATTACHMENT_COLOR_N, otherwise it's ignored
void ms_fbo_attach_texture (ms_framebuffer, ms_texture, enum ms_attachment slot, int num, int mip_level);
// attaches a render buffer to a framebuffer
// `num` is only used in case the attachment is MS_ATTACHMENT_COLOR_N, otherwise it's ignored
void ms_fbo_attach_rbo     (ms_framebuffer, ms_renderbuffer, enum ms_attachment slot, int num);
// validates the framebuffer.
bool           ms_fbo_is_complete    (ms_framebuffer);

// === RENDERBUFFERS ===
ms_renderbuffer ms_create_rbo (void);
void            ms_destroy_rbo(ms_renderbuffer);


// TODO:
// implement the instanced versions of these two functions.
// NOTE:
// is there some way i can get rid of or abstract away the `prim_mode` and `type`
// parameters?
void ms_draw_elems(ms_vao vao, int prim_mode, int type, isize count, const int* start);
void ms_draw_arrays(ms_vao vao, int prim_mode, int index, int num_verts);

// {{{ INPUT
// KEYBOARD FUNCTIONS
// returns true if the key is held for two consequtive frames
bool ms_is_key_down         ( int key );
// returns true if the key has been pressed once
bool ms_is_key_pressed      ( int key );
// returns true if the key has been released once
bool ms_is_key_released     ( int key );
// gets the last key pressed
int   ms_get_key_pressed  ( void );
// gets the last key released
int   ms_get_key_released ( void );

// MOUSE FUNCTIONS
// gets mouse position relative to the active window
vec2s ms_mouse_pos        (void);
f32   ms_mouse_x          (void);
f32   ms_mouse_y          (void);
vec2s ms_mouse_delta      (void);
f32   ms_mouse_delta_x    (void);
f32   ms_mouse_delta_y    (void);

f32   ms_mouse_scroll_amt   (void); // NOTE: is this really needed???
f32   ms_mouse_scroll_delta (void);
bool  ms_is_mouse_down     ( u8 button );
bool  ms_is_mouse_pressed  ( u8 button );
bool  ms_is_mouse_released ( u8 button );

// GAMEPAD FUNCTIONS
// UNIMPLEMENTED:
bool ms_is_button_down    ( uint gamepad, u8 button );
// UNIMPLEMENTED:
bool ms_is_button_pressed ( uint gamepad, u8 button );
// UNIMPLEMENTED:
bool ms_is_button_up      ( uint gamepad, u8 button );
// u8   ms_get_button_pressed  ( uint gamepad );
// u8   ms_get_button_released ( uint gamepad );

// UNIMPLEMENTED:
f32 get_stick_axis_x ( uint gamepad, u8 stick );
// UNIMPLEMENTED:
f32 get_stick_axis_y ( uint gamepad, u8 stick );
// }}} INPUT

// }} FUNCTION PROTOTYPES


#ifdef __cplusplus
	}
#endif
