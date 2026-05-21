/* MIT License
 * 
 * Copyright (c) 2026 derpyzza
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// small opengl framework for gamedev
// made and written by derpyzza, by hand.
// no LLMS were used to develop the code in this library.


#pragma once
#include <SDL3/SDL.h>
#include <libderp/derp.h>
#include <cglm/struct.h>
#include "glad/glad.h"
#include <GL/gl.h>

#include "input_enums.h"

// TODO:
// 	> maybe instead of having fixed length name strings i could instead have a library-wide arena for strings?
// FIXME:
// 	> ms_create_shader needs some sort of error throwing functionality. currently it logs an error but it needs to return some sort of error value that can be checked too.

// {{ DEFINES
#define MS_VERSION "0.0.0"
#define MS_NAME_LEN 128     /* string length for name fields in structs to avoid having to allocate strings everywhere */
#define MAX_SHADER_LOCS 32  /* maximum uniform locations a shader can have */
#define MAX_SHADERSTAGES 64
#define MAX_SHADERS 64


#define IS_FLAG_SET(n, f) ((n) & (f)) == (f)
#define WRAP(f, x, y) if(f < x){ f = y; } else if(f>y){f = x;} 
// }} DEFINES

// {{ ENUMS

enum ms_window_flags {
	MS_WindowFlag_Resizable    = 1,
	MS_WindowFlag_Centered     = 1<<1,
	MS_WindowFlag_MsaaX4       = 1<<2,
	MS_WindowFlag_EnableVsync  = 1<<3,

	MS_WINDOWFLAG_COUNT
};

enum ms_uniform_type {
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
};

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

	MS_VertexFormat_COUNT
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
  MS_VertKind_Texcoord_N,
  // vec4 float or uint32_t
  MS_VertKind_Colour_N,
  // uint[4]
  MS_VertKind_Weights_N,
  // uint[4]
  MS_VertKind_Joints_N,

  MS_VertKind_Custom_N,

  MS_VertKind_Count
} ms_vert_attr_kinds;

enum ms_camtype {
	MS_Camtype_Proj,
	MS_Camtype_Ortho
};


typedef enum ms_cull_mode {
  MS_CULLMODE_NONE,
  MS_CULLMODE_BACK,
  MS_CULLMODE_FRONT,
} ms_cull_mode;

typedef enum ms_blend_mode {
  MS_BLENDMODE_OPAQUE,
  MS_BLENDMODE_ALPHA,
  MS_BLENDMODE_MASK,
} ms_blend_mode;

typedef enum ms_draw_primitives {
  MS_PRIM_TRIANGLE,
  MS_PRIM_LINES,
  MS_PRIM_POINTS,
  MS_PRIM_TRIANGLE_STRIP,
  MS_PRIM_TRIANGLE_FAN
} ms_draw_primitives;

typedef enum ms_poly_mode {
  MS_POLY_FILL,
  MS_POLY_WIRE
} ms_poly_mode;

typedef enum ms_buffertype {
	MS_BufferInvalid,
	MS_BufferVertex,
	MS_BufferIndex,
} ms_buffertype;

// }} ENUMS


// {{ TYPES

typedef const void ms_handle;

typedef struct {
	int wrap_t, wrap_s;
	int min_filter, max_filter;
} ms_sampler;

typedef struct {
	u32 id;
} ms_shader;

typedef struct {
	u32 id;
} ms_uniform;

typedef struct ms_texture {
	u32 id;
	int width, height, nrChannel;

	ms_sampler sampler;

	char path[MS_NAME_LEN]; // filepath ( optional )
} ms_texture;

typedef struct ms_camera {	
	vec3s   pos;
	versors rot;
	vec3s target;

	vec3s front, forward, right;

	f32 near, far;
	f32 fov, aspect;

	mat4s view, proj;
} ms_camera;

typedef struct ms_camera2D {
	vec3s pos;
	vec2s size;
	mat4s view;
} ms_camera2D;

// pipeline state object, similar to the kinds of pipeline objects found in modern graphics apis like Vulkan, or sokol_gfx
typedef struct ms_pipeline {
  ms_shader * shader;
  ms_cull_mode cullmode;
  ms_blend_mode blend_mode;
  ms_draw_primitives primitives;
  ms_poly_mode poly_mode;
} ms_pipeline;


/*
	uniform types:
		vec2-4,
		mat4,
		sampler
		single numbers

	vertex format types:
		f, f2, f3, f4, f16
		i, i2, i3, i4
		u, u2, u3, u4
		b, b2, b3, b4
		ub, ub2, ub3, ub4

*/

typedef struct ms_vertex_attrib {
	int index;
	int size;
	int type;
	bool normalized;
	int stride;
	isize offset;
} ms_vertex_attrib;

typedef struct ms_vertex_layout {
	isize num_attribs;
	ms_vertex_attrib * attribs;
} ms_vertex_layout;

typedef struct ms_mesh {
	u32 vao, vbo, ebo;

	size_t num_verts, num_indices;
} ms_mesh;

static const ms_vertex_layout MS_VERTLAYOUT_POSCOL = {
	.num_attribs = 2,
	.attribs = (ms_vertex_attrib[]){
		{
			.index = 0,
			.size = 3,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		},
		{
			.index = 1,
			.size = 4,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		}
	}
};

static const ms_vertex_layout MS_VERTLAYOUT_POSUV = {
	.num_attribs = 2,
	.attribs = (ms_vertex_attrib[]){
		{
			.index = 0,
			.size = 3,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		},
		{
			.index = 1,
			.size = 2,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		}
	}
};

static const ms_vertex_layout MS_VERTLAYOUT_POSUVCOL = {
	.num_attribs = 2,
	.attribs = (ms_vertex_attrib[]){
		{
			.index = 0,
			.size = 3,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		},
		{
			.index = 1,
			.size = 2,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		},
		{
			.index = 2,
			.size = 4,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		}
	}
};

static const ms_vertex_layout MS_VERTLAYOUT_LIT = {
	.num_attribs = 2,
	.attribs = (ms_vertex_attrib[]){
		// position, vec3f
		{
			.index = 0,
			.size = 3,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		},
		// normal, vec3f
		{
			.index = 1,
			.size = 3,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		},
		// uv, vec2f
		{
			.index = 1,
			.size = 2,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		},
		// colour, vec4f
		{
			.index = 2,
			.size = 4,
			.type = GL_FLOAT,
			.stride = 0,
			.normalized = false,
		}
	}
};


typedef struct ms_buffer {
	u32 id;
	ms_buffertype type;
} ms_buffer;

// }} TYPES

// {{ FUNCTION PROTOTYPES

// create and initialize a window
void ms_init_window( int width, int height, const char* title, int flags );
// update internal library state, like input state etc.
void ms_update(void);
// used for controlling the game loop, signals when it's time to quit
bool ms_should_quit(void);
void ms_cleanup(void);

// sets up rendering stuff
void ms_begin_drawing(void);
// actually renders
void ms_end_drawing(void);
void ms_set_viewport(int x, int y, f32 w, f32 h);
void ms_clear_bg(vec4s colour);

// NOTE: all the colour functions deal with normalized colours
// and will automatically normalize their inputs and outputs
static inline vec4s ms_col_from_hex(u32 hex){
  return (vec4s) {{
    hex >> 24,
    hex >> 16,
    hex >> 8,
    hex
  }};
}

static inline u32 ms_col_to_hex(vec4s c) {
	c = glms_vec4_normalize(c);
  return ((uint)c.x << 24) & ((uint)c.y << 16) & ((uint)c.z << 8) & (uint)c.w;
}

static inline vec4s ms_col(u8 r, u8 g, u8 b, u8 a) {
	return (vec4s) {{
		r, g, b, a
	}};
}

static inline vec4s ms_col_f(f32 r, f32 g, f32 b, f32 a) {
	if( r > 1 ) r /= 255;
	if( g > 1 ) g /= 255;
	if( b > 1 ) b /= 255;
	if( a > 1 ) a /= 255;
	return (vec4s){{r,g,b,a}};
}


// reads shader code from provided file paths and compiles and links the shader program.
// return's a handle to the shader
ms_shader ms_create_shader( const char * vert_file, const char * frag_file );
ms_shader ms_create_shader_from_source( dstr vert_source, dstr frag_source );

ms_uniform ms_get_uniform(ms_shader, const char* name);

void ms_shader_set_mat4   (ms_shader, ms_uniform, mat4s*, bool);
void ms_shader_set_mat4_v (ms_shader, ms_uniform, mat4s*, isize count, bool);

void ms_bind_shader        ( ms_shader shader );
void ms_shader_set_value   ( ms_shader shader, int loc, const void * data, enum ms_uniform_type val_type );
void ms_shader_set_value_v ( ms_shader shader, int loc, const void * data, isize count, enum ms_uniform_type val_type );
void ms_shader_set_matrix  ( ms_shader shader, int loc, mat4s * mat, bool transpose );
void ms_shader_set_matrix_v( ms_shader shader, int loc, mat4s * mat, isize count, bool transpose );


// these functions take in an optional pointer to a sampler
// passing in NULL uses a default sampler value instead.
ms_texture ms_create_texture( int h, int w, int infmt, int outfmt, const void* data, ms_sampler * sampler );
ms_texture ms_load_texture( const char* path, ms_sampler * sampler );
ms_texture ms_load_texture_from_memory( const u8* data, int data_len, ms_sampler * sampler );
void bind_texture(ms_texture * texture);
void bind_texture_slot(ms_texture * texture, uint slot);

ms_mesh ms_create_mesh(const void* verts, isize num_verts, const float * indices, isize num_indices, ms_vertex_layout layout);
void ms_draw_mesh(ms_mesh mesh);
void ms_destroy_mesh(u32 mesh);

// {{{ INPUT
// KEYBOARD FUNCTIONS
// returns true if the key is held for two consequtive frames
bool ms_key_down         ( int key );
// returns true if the key has been pressed once
bool ms_key_pressed      ( int key );
// returns true if the key has been released once
bool ms_key_released     ( int key );
// gets the last key pressed
u8   ms_get_key_pressed  ( int key );
// gets the last key released
u8   ms_get_key_released ( int key );

// MOUSE FUNCTIONS
// gets mouse position relative to the active window
vec2s get_mouse_pos        (void);
f32   get_mouse_x          (void);
f32   get_mouse_y          (void);

// gets mouse position relative to the active screen/monitor
vec2s get_mouse_screen_pos (void);
f32   get_mouse_screen_x   (void);
f32   get_mouse_screen_y   (void);

f32   get_scroll_amt    (void);
bool  ms_mouse_down     ( u8 button );
bool  ms_mouse_pressed  ( u8 button );
bool  ms_mouse_released ( u8 button );

// GAMEPAD FUNCTIONS
bool ms_button_down    ( uint gamepad, u8 button );
bool ms_button_pressed ( uint gamepad, u8 button );
bool ms_button_up      ( uint gamepad, u8 button );
u8 get_button_pressed  ( uint gamepad );
u8 get_button_released ( uint gamepad );

f32 get_stick_axis_x ( uint gamepad, u8 stick );
f32 get_stick_axis_y ( uint gamepad, u8 stick );
// }}} INPUT

// }} FUNCTION PROTOTYPES
