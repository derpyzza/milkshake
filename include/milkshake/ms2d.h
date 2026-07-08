/* milkshake/2D.h
 * utility functions for 2D rendering.
 * 
 * Copyright (c) 2026 derpyzza
 * 
 * This file is released under the terms of the MIT license.
 * Read LICENSE for more information.
 *
 * no LLMS were used to write the code in this library.
*/ 

#pragma once
#include <milkshake/milkshake.h>

// NOTE:
//   > should i change the default shader and texture get-functions?
//     currently they use a static bool toggle to make sure not to create
//     the shader/texture more than once, but is there a better way to do
//     this?
//   > for the default texture, i could just load it when the main milkshake
//     library loads and keep it around for the lifetime of the app. it's a
//     tiny 1px texture, so it's not likely to eat up too much ram, plus it'll
//     be handy for a lot more than just a 2D shape renderer.
// TODO:
//   > deprecate the spritebatch :(

/*

  struct {
    ...
    ms2d_renderbatch batch;
  } state;

  void init(void) {
    ...
    state.batch = ms2d_create_renderbatch(2048);
  }

  void draw(void) {
    ms_bind_fbo(state.screen);
    ms_bind_shader(shader);
    ms_uniform_set_mat4(state.proj_loc, state.proj, false);
    ms_uniform_set_mat4(state.view_loc, state.view, false);

    ms2d_rect(10, 10, 30, 30, 0xFF0000FF);
    ms2d_rect(50, 60, 30, 30, 0xFF0000FF);
    ms2d_circ(40, 30, 30, 0xFF0000FF);
    ms2d_rect(70, 30, 30, 30, 0xFF0000FF);
    ms2d_rectr(40, 60, 30, 30, 60.0f, 0xFF0000FF);

    ms2d_flush();

    ms_unbind_fbo();
    ms_bind_shader(shader);
    ms_uniform_set_mat4(state.proj_loc, state.proj, false);
    ms_uniform_set_mat4(state.view_loc, state.view, false);

    ms2d_texrect(0, 0, WIN_H, WIN_W, 0, 0, SCR_H, SCR_W, 0xffffffff);
    ms2d_flush();
  }

*/

#ifdef __cplusplus
  extern "C" {
#endif

// these values can be overrided by `#define`-ing them before
// including <milkshake/ms2d.h>
#ifndef MS2D_DEFAULT_QUADS
#  define MS2D_DEFAULT_QUADS 1024
#endif
#ifndef MS2D_DEFAULT_DRAWCALLS
#  define MS2D_DEFAULT_DRAWCALLS 32
#endif

typedef enum ms2d_drawmode {
  MS2D_LINES,
  MS2D_TRIS,
  MS2D_QUADS
} ms2d_drawmode;

typedef struct sprite_vertex {
  vec2s pos;
  vec2s uv;
  u8 col[4];
} sprite_vertex;

typedef struct ms2d_drawcall {
  ms2d_drawmode mode;
  int vert_align, num_verts;
  ms_texture texture;
} ms2d_drawcall;

typedef struct ms2d_renderbatch {
  isize num_verts, max_verts;
  sprite_vertex * verts;

  isize num_drawcalls, max_drawcalls;
  ms2d_drawcall * drawcalls;

  ms_vao vao;
  ms_buffer vbo, ebo;
} ms2d_renderbatch;

typedef struct {
  vec2s pos;      // screen pos in pixels
  vec2s size;     // screen size in pixels
  vec2s origin;   // origin / pivot point for the sprite.
                  // defaults to (0.5, 0.5)
                  // (the center of a quad)
                  // in normalized coordinates

  // NOTE:
  // texture coordinates always start at the top-left and
  // go to the bottom-right and are not affected by the
  // origin point whatsoever
  vec2s tex_pos;  // texture pos on atlas, in pixels 
  vec2s tex_size; // texture size on atlas, in pixels

  bool flip_x, flip_y;
  float rotation; // angle in degrees

  u32 colour;     // optional tint
} ms2d_sprite;


// returns a new renderbatch with a default size for num_quads and num_drawcalls
// just for convenience
// the default values are:
// - MS_DEFAULT_QUADS: 1024
// - MS_DEFAULT_DRAWCALLS: 32
ms2d_renderbatch ms2d_new_renderbatch(void);
ms2d_renderbatch ms2d_create_renderbatch(isize num_quads, isize num_draw_calls);
void ms2d_destroy_renderbatch(ms2d_renderbatch * batch);
void ms2d_renderbatch_flush(ms2d_renderbatch * batch);
void ms2d_flush(void);
void ms2d_bind_renderbatch(ms2d_renderbatch* batch);

typedef struct {
  f32 x, y, w, h;
} ms2d_rectangle; 

typedef struct {
  f32 x, y, r;
} ms2d_circle; 

typedef struct {
  vec2s a, b, c, d;
} ms2d_quad; 

void ms2d_rect(f32 x, f32 y, f32 w, f32 h, uint colour);
// draw a rectangle with rotation
void ms2d_rectr(f32 x, f32 y, f32 w, f32 h, f32 rot, uint colour);
// draw a rectangle with vectors + rotation
void ms2d_rectv(vec2s pos, vec2s size, f32 rot, uint colour);
void ms2d_rectpro(ms2d_rectangle r, vec2s origin, f32 rot, uint colour);

// draw part of a texture as a rectangle
void ms2d_texrect(ms_texture tex, ms2d_rectangle src, ms2d_rectangle dest, uint colour);
void ms2d_texrectr(ms_texture tex, ms2d_rectangle src, ms2d_rectangle dest, f32 rot, uint colour);
// draw entire texture as a rect
void ms2d_texrectw(ms_texture tex, vec2s pos, uint colour, bool flip_x, bool flip_y);
void ms2d_texrectpro(
  ms_texture tex,
  ms2d_rectangle src,
  ms2d_rectangle dest,
  vec2s origin,
  f32 rot,
  uint colour,
  bool flip_x,
  bool flip_y
);

void ms2d_circ     (f32 x, f32 y, f32 r, uint colour);
void ms2d_circv    (vec2s pos, f32 r, uint colour);
void ms2d_circc    (ms2d_circle c, uint colour);

// draw circle sector
void ms2d_circ_sec  (f32 x, f32 y, f32 r, f32 start_angle, f32 end_angle, int segments, uint colour);
// void ms2d_secv (vec2s c, f32 start_angle, f32 end_angle, uint colour);

// NOTE:
// is this actually useful??
void ms2d_colquad  (vec2s a, vec2s b, vec2s c, vec2s d, uint colour);
void ms2d_colquadq (ms2d_quad quad, uint colour);
void ms2d_texquad  (ms_texture tex, ms2d_quad quad, uint colour);

ms2d_sprite ms2d_create_sprite(vec2s pos, vec2s size, vec2s tex_pos, vec2s tex_size);

// submit a polygon to the batch
void ms2d_poly_submit(
  ms2d_drawmode mode,
  ms_texture tex,
  sprite_vertex* verts,
  uint num_verts
);

static const ms_vertex_layout MS2D_VERTLAYOUT_SPRITE = {
  .num_attribs = 3,
  .stride = 4 * sizeof(float) + sizeof(u32),
  .attribs = (ms_vertex_attrib[]){
    {
      .index = 0,
      .size = 2,
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
      .offset = 2 * sizeof(float),
      .attr_kind = MS_VertKind_TexCoord_N,
    },
    {
      .index = 2,
      .size = 4,
      .type = GL_UNSIGNED_BYTE,
      .normalized = true,
      .offset = 4 * sizeof(float),
      .attr_kind = MS_VertKind_Colour_N,
    },
  }
};

static ms_shader ms2d_sprite_shader(void) {
  const dstr vert = dstr(
    "#version 330 core\n"
    "layout (location=0) in vec2 a_pos;"
    "layout (location=1) in vec2 a_uv;"
    "layout (location=2) in vec4 a_col;"
    "out vec2 v_uv;"
    "out vec4 v_col;"
    "uniform mat4 view;"
    "uniform mat4 proj;"
    "void main() {"
      // NOTE: hardcoding the depth value for now...
      "gl_Position = proj * view * vec4(a_pos, 0.0, 1.0);"
      "v_uv  = a_uv ;"
      "v_col = a_col;"
    "}"
  );
  const dstr frag = dstr(
    "#version 330 core\n"
    "out vec4 FragColour;"
    "in vec2 v_uv;"
    "in vec4 v_col;"
    "uniform sampler2D tex;"
    "void main() {"
      "vec4 _tex = texture(tex, v_uv);"
      "FragColour = _tex * v_col;"
    "}"
  );

  // cache shader
  static bool init = false;
  static ms_shader out;
  if(!init) {
    out = ms_create_shader_from_source(vert, frag);
    init = true;
  }

  return out;
}

static ms_texture ms2d_default_texture(void) {
  static bool init_texture = false;
  static ms_texture out;
  if(!init_texture) {
    int col = 0xFFFFFFFF;
    out = ms_create_texture(1, 1, GL_RGBA8, GL_RGBA, &col,
      &(ms_sampler){
        GL_CLAMP_TO_EDGE,
        GL_CLAMP_TO_EDGE,
        GL_NEAREST,
        GL_NEAREST
      }
    );
    init_texture = true;
  }

  return out;
}

#ifdef __cplusplus
	}
#endif
