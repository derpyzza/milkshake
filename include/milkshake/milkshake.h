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

// NOTE:
// 	> maybe instead of having fixed length name strings i could instead have a library-wide arena for strings?
// 	> maybe instead of making the window be part of G_core i instead just return a window object that the user
// 		controls?
// 		i'll still keep the input stuff private to G_core since the user really does not really need to care about those
// 		( and can always just poll inputs directly with SDL if they do )
// 		but the window i can see the user wanting to control by themselves
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
  // vec4 float or uint32_t
  MS_VertKind_Colour_N,
  // uint[4]
  MS_VertKind_Weights_N,
  // uint[4]
  MS_VertKind_Joints_N,

  MS_VertKind_Custom_N,

  MS_VERTKIND_COUNT
} ms_vert_attr_kinds;

enum ms_camtype {
	MS_CamType_Proj,
	MS_CamType_Ortho
};


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

// }} ENUMS


/*

vertex_layout := {}
vertex_layout.attribs = malloc(sizeof(vertex_attrib) * prim.accessors.length)

for attr in prim.accessors[] {
	switch attr.type {
		Postion => {
			vertex_layout.attribs.push(attr.data.to_attribs());
		},
		
	}

}


*/

// {{ TYPES

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

typedef struct {
	int wrap_t, wrap_s;
	int min_filter, max_filter;
} ms_sampler;

typedef struct ms_texture {
	u32 id;
	int width, height, nrChannel;

	ms_sampler sampler; // NOTE: do i even need to store this in the texture object?

	char path[MS_NAME_LEN]; // filepath ( optional )
} ms_texture;

// pipeline state object, similar to the kinds of pipeline objects found in modern graphics apis like Vulkan, or sokol_gfx
// NOTE: Should i get rid of this?
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

typedef struct ms_camera2D {
	vec3s pos;
	vec2s size;
	mat4s view;
} ms_camera2D;

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
			.offset = 3,
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
			.offset = 2,
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
			.attr_kind = MS_VertKind_Position,
		},
		{
			.index = 1,
			.size = 2,
			.type = GL_FLOAT,
			.normalized = false,
			.attr_kind = MS_VertKind_TexCoord_N,
		},
		{
			.index = 2,
			.size = 4,
			.type = GL_FLOAT,
			.normalized = false,
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
			.attr_kind = MS_VertKind_Position,
		},
		// normal, vec3f
		{
			.index = 1,
			.size = 3,
			.type = GL_FLOAT,
			.normalized = false,
			.attr_kind = MS_VertKind_Normal,
		},
		// uv, vec2f
		{
			.index = 2,
			.size = 2,
			.type = GL_FLOAT,
			.normalized = false,
			.attr_kind = MS_VertKind_TexCoord_N,
		},
		// colour, vec4f
		{
			.index = 3,
			.size = 4,
			.type = GL_FLOAT,
			.normalized = false,
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
			.attr_kind = MS_VertKind_Position,
		},
		// normal, vec3f
		{
			.index = 1,
			.size = 3,
			.type = GL_FLOAT,
			.normalized = true,
			.attr_kind = MS_VertKind_Normal,
		},
		// uv, vec2f
		{
			.index = 2,
			.size = 2,
			.type = GL_FLOAT,
			.normalized = false,
			.attr_kind = MS_VertKind_TexCoord_N,
		},
		// colour, vec4f
		{
			.index = 3,
			.size = 4,
			.type = GL_FLOAT,
			.normalized = true,
			.attr_kind = MS_VertKind_Colour_N,
		},
		// joints, vec4i
		{
			.index = 4,
			.size = 4,
			.type = GL_INT,
			.normalized = false,
			.attr_kind = MS_VertKind_Joints_N,
		},
		// weights, vec4f
		{
			.index = 5,
			.size = 4,
			.type = GL_FLOAT,
			.normalized = true,
			.attr_kind = MS_VertKind_Weights_N,
		},
	}
};



// }} TYPES

// {{ FUNCTION PROTOTYPES

// create and initialize a window
void ms_init_window( int width, int height, const char* title, int flags );
// update internal library state, like input state etc.
void ms_update(void);
void ms_cleanup(void);

// calls the sdl_swap thingy
void ms_end_drawing(void);
void ms_clear_colour(u32 hex);

// NOTE: all the colour functions deal with normalized colours
// and will automatically normalize their inputs and outputs
static inline vec4s ms_col_from_hex(u32 hex){
  return (vec4s) {{
	  ((hex >> 24) & 0xFF) / 255.0f,
    ((hex >> 16) & 0xFF) / 255.0f,
    ((hex >> 8)  & 0xFF) / 255.0f,
     (hex        & 0xFF) / 255.0f 
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
// UNIMPLEMENTED:
void ms_shader_set_value   ( ms_shader shader, int loc, const void * data, enum ms_uniform_type val_type );
// UNIMPLEMENTED:
void ms_shader_set_value_v ( ms_shader shader, int loc, const void * data, isize count, enum ms_uniform_type val_type );


// these functions take in an optional pointer to a sampler
// passing in NULL uses a default sampler value instead.
ms_texture ms_create_texture( int h, int w, int infmt, int outfmt, const void* data, ms_sampler * sampler );
ms_texture ms_load_texture( const char* path, ms_sampler * sampler );
ms_texture ms_load_texture_from_memory( const u8* data, int data_len, ms_sampler * sampler );
void bind_texture(ms_texture * texture);
void bind_texture_slot(ms_texture * texture, uint slot);
void ms_unload_texture(ms_texture tex);

ms_vao ms_create_vao(void);
// NOTE: if usage is any of the DYNAMIC_* types then data may be NULL
ms_buffer ms_create_buffer(ms_buffertype type, ms_buffer_usage usage, isize size, const void * data);
void ms_vao_attach_vbo(ms_vao *vao, ms_buffer buffer, ms_vertex_layout layout);
void ms_vao_attach_ebo(ms_vao *vao, ms_buffer buffer);

void ms_destroy_vao(ms_vao vao);
void ms_destroy_buffer(ms_buffer buffer);

// {{{ INPUT
// KEYBOARD FUNCTIONS
// returns true if the key is held for two consequtive frames
bool ms_is_key_down         ( int key );
// returns true if the key has been pressed once
bool ms_is_key_pressed      ( int key );
// returns true if the key has been released once
bool ms_is_key_released     ( int key );
// gets the last key pressed
// u8   ms_get_key_pressed  ( int key );
// gets the last key released
// u8   ms_get_key_released ( int key );

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
