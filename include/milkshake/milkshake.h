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

// TODO:
// 	> maybe instead of having fixed length name strings i could instead have a library-wide arena for strings?

// {{ DEFINES
#define MS_VERSION "0.0.0"
#define MS_NAME_LEN 64     /* string length for name fields in structs to avoid having to allocate strings everywhere */
#define MAX_SHADER_LOCS 32 /* maximum uniform locations a shader can have */
// }} DEFINES

// {{ ENUMS
enum ms_uniform_type {
	MS_UNIFORM_FLOAT,
	MS_UNIFORM_FLOAT2,
	MS_UNIFORM_FLOAT3,
	MS_UNIFORM_FLOAT4,

	MS_UNIFORM_INT,
	MS_UNIFORM_INT2,
	MS_UNIFORM_INT3,
	MS_UNIFORM_INT4,

	MS_UNIFORM_UINT,
	MS_UNIFORM_UINT2,
	MS_UNIFORM_UINT3,
	MS_UNIFORM_UINT4,

	MS_UNIFORM_SAMPLER2D,

	MS_UNIFORM_COUNT
};

enum ms_camtype {
	MS_CAMTYPE_PROJ,
	MS_CAMTYPE_ORTHO
};

// }} ENUMS


// {{ TYPES

typedef const void ms_handle;

typedef struct ms_window ms_window;
typedef struct ms_shader ms_shader;   /* analogous to opengl shader programs */
typedef struct ms_texture ms_texture;

typedef struct ms_sampler {
	int wrap_t, wrap_s;
	int min_filter, max_filter;
} ms_sampler;

typedef struct ms_camera {	
	vec3s   pos;
	versors rot;
	vec3s target;

	vec3s front, forward, right;

	f32 near, far;
	f32 fov, aspect;

	mat4s view, proj;
} ms_camera;

typedef struct ms_camera2D { } ms_camera2D;

// }} TYPES

// {{ FUNCTION PROTOTYPES

// create and initialize a window
ms_window * ms_init_window( int width, int height, const char* title );
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
u32 ms_create_shader( const char * vert_file, const char * frag_file );

void ms_bind_shader        ( ms_handle* shader );
void ms_shader_set_value   ( ms_handle* shader, int loc, const void * data, enum ms_uniform_type val_type );
void ms_shader_set_value_v ( ms_handle* shader, int loc, const void * data, isize count, enum ms_uniform_type val_type );
void ms_shader_set_matrix  ( ms_handle* shader, int loc, mat4s * mat, bool transpose );
void ms_shader_set_matrix_v( ms_handle* shader, int loc, mat4s * mat, isize count, bool transpose );


// these functions take in an optional pointer to a sampler
// passing in NULL uses a default sampler value instead.
ms_texture ms_create_texture( int h, int w, int infmt, int outfmt, const void* data, ms_sampler * sampler );
ms_texture ms_load_texture( const char* path, ms_sampler * sampler );
ms_texture ms_load_texture_from_memory( const char* data, int data_len, ms_sampler * sampler );
void bind_texture(ms_texture * texture);
void bind_texture_slot(ms_texture * texture, uint slot);

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
