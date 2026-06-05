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

// milkshake/2D.h
// utility functions for 2D rendering.

#pragma once
#include <milkshake/milkshake.h>

typedef struct sprite_vertex {
  vec2s pos;
  vec2s uv;
  u32 col;
} sprite_vertex;

static const ms_vertex_layout MS_VERTLAYOUT_SPRITE = {
  .num_attribs = 2,
  .stride = 4 * sizeof(float) + sizeof(u32),
  .attribs = (ms_vertex_attrib[]){
    {
      .index = 0,
      .size = 2,
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
      .size = 1,
      .type = GL_INT,
      .normalized = false,
      .attr_kind = MS_VertKind_Colour_N,
    },
  }
};


typedef struct ms_sprite {
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

  float rotation; // angle in degrees

  // float opacity;  // optional opacity. 0-1.
  float z_depth;  // optional z-value
  u32 colour;     // optional tint
} ms_sprite;

typedef struct ms2D_spritebatch {
  // maybe in the future i could make a stronger spritebatch system where each
  // sprite batch can take in multiple texture draw calls and then automatically
  // sorts the draw calls by texture or whatnot, but for now, each spritebatch
  // only works with a single texture atlas at a time ( though one that can be
  // changed at runtime if you wish ). the user will simply have to manually
  // organize their drawcalls for now. the spritebatch exists solely to reduce
  // draw calls, and not much else.
  ms_texture texture;
  isize
    capacity // total number of sprites this batch can render at once before needing to flush
  , size;    // current number of requested sprite draws

  sprite_vertex
    *verts; // size: capacity * 4
  sprite_vertex *current;

  ms_vao vao;
  ms_buffer vbo, ebo;
} ms2D_spritebatch;

ms2D_spritebatch create_spritebatch(ms_texture texture, isize num_sprites);
void ms2D_destroy_spritebatch(ms2D_spritebatch * sprite_batch);

ms_sprite create_sprite(vec2s pos, vec2s size, vec2s tex_pos, vec2s tex_size);

void ms2D_spritebatch_submit(ms2D_spritebatch * batch, ms_sprite sprite);
void ms2D_spritebatch_swap_texture(ms2D_spritebatch * batch, ms_texture tex);
void ms2D_spritebatch_flush(ms2D_spritebatch * batch);

static const ms_shader ms2D_sprite_shader(void) {
  const dstr vert = dstr(
    "#version core 330\n"
  );
  const dstr frag = dstr(
    "#version core 330\n"
  );

  return ms_create_shader_from_source(vert, frag);
}
