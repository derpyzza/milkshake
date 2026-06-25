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
#include "cglm/struct/vec2.h"
#include "internal.h"
#include "milkshake/milkshake.h"
#include <math.h>
#include <milkshake/2D.h>

ms2D_spritebatch
ms2D_create_spritebatch(ms_texture texture, isize num_sprites) {
  isize num_verts = num_sprites * 4;
  isize num_indices = num_sprites * 6;
  isize verts_size = sizeof(sprite_vertex) * num_verts;

  ms2D_spritebatch out = { 0 };
  out.texture = texture;
  out.capacity = num_sprites;


  out.verts = d_alloc(verts_size);
  out.current = out.verts;

  out.vao = ms_create_vao();
  out.vbo =
    ms_create_buffer(
      MS_BufferType_Vertex
      , MS_BufferUsage_DynamicDraw
      , verts_size
      , NULL
    );

  u32 indices[num_indices];
  int offset = 0;
  for (int i = 0; i < num_indices; i += 6) {
    indices[i + 0] = offset + 0;
    indices[i + 1] = offset + 1;
    indices[i + 2] = offset + 2;
    indices[i + 3] = offset + 2;
    indices[i + 4] = offset + 3;
    indices[i + 5] = offset + 0;
    offset += 4;
  }
  
  out.ebo = ms_create_buffer(
    MS_BufferType_Index
    , MS_BufferUsage_StaticDraw
    , sizeof(indices)
    , indices
  );

  ms_vao_attach_vbo(out.vao, out.vbo, MS_VERTLAYOUT_SPRITE);
  ms_vao_attach_ebo(out.vao, out.ebo);

  return out;
}


void ms2D_destroy_spritebatch(ms2D_spritebatch * batch) {
  d_free(batch->verts);
  ms_destroy_vao(batch->vao);
  
  ms_destroy_buffer(batch->vbo);
  ms_destroy_buffer(batch->ebo);
}

ms2D_sprite ms2D_create_sprite(vec2s pos, vec2s size, vec2s tex_pos, vec2s tex_size) {
  ms2D_sprite out = { 0 };
  out.pos = pos;
  out.size = size;

  out.tex_pos = tex_pos;
  out.tex_size = tex_size;

  out.flip_x = false;
  out.flip_y = false;

  out.origin = (vec2s){{0.5, 0.5}};
  out.colour = 0xFFFFFFFF;

  return out;
}

void ms2D_spritebatch_submit(ms2D_spritebatch * batch, ms2D_sprite sprite) {
  if(batch->size >= batch->capacity) {
    ms2D_spritebatch_flush(batch);
  }

  vec2s voffsets[4] = {
    {{            0,             0}},
    {{sprite.size.x,             0}},
    {{sprite.size.x, sprite.size.y}},
    {{            0, sprite.size.y}}
  };

  vec2s toffsets[4] = {
    {{                0,                 0}},
    {{sprite.tex_size.x,                 0}},
    {{sprite.tex_size.x, sprite.tex_size.y}},
    {{                0, sprite.tex_size.y}}
  };

  vec2s origin_offset = glms_vec2_mul(sprite.origin, sprite.size);

  float rad = glm_rad(sprite.rotation);
  float cosa = cosf(rad);
  float sina = sinf(rad);

  for(int i = 0; i < 4; i++) {
    float local_x = voffsets[i].x - origin_offset.x;
    float local_y = voffsets[i].y - origin_offset.y;

    // rotated coordinates in local-space ( pixels )
    float rot_x = local_x * cosa - local_y * sina;
    float rot_y = local_x * sina + local_y * cosa;

    // final coordinates in world-space ( pixels )
    batch->current->pos.x = rot_x + sprite.pos.x;
    batch->current->pos.y = rot_y + sprite.pos.y;

    float t_u;
    if(sprite.flip_x) {
      t_u = sprite.tex_pos.x + (sprite.tex_size.x - toffsets[i].x);
    } else {
      t_u = sprite.tex_pos.x + toffsets[i].x;
    }
    float t_v;
    if(sprite.flip_y) {
      t_v = sprite.tex_pos.y + (sprite.tex_size.y - toffsets[i].y);
    } else {
      t_v = sprite.tex_pos.y + toffsets[i].y;
    }

    batch->current->uv.x = t_u / batch->texture.width ;
    batch->current->uv.y = t_v / batch->texture.height;
    
    batch->current->col = sprite.colour;

    batch->current++;
  }

  batch->size++; // pushes a new sprite to the array.
}

void ms2D_spritebatch_flush(ms2D_spritebatch * batch) {
  if(G_core.gl_data.current_texture.id != batch->texture.id) {
    ms_bind_texture(batch->texture);
  }
  glBindBuffer(GL_ARRAY_BUFFER, batch->vbo.id); 
  glBufferSubData(
      GL_ARRAY_BUFFER, 
      0, 
      batch->size * 4 * sizeof(sprite_vertex), 
      batch->verts
  );

  ms_draw_elems(
    batch->vao
    , GL_TRIANGLES
    , GL_UNSIGNED_INT
    , batch->size * 6
    , NULL
  );

  batch->size = 0;
  batch->current = batch->verts;
}

void ms2D_spritebatch_swap_texture(ms2D_spritebatch * batch, ms_texture tex) {
  batch->texture = tex;
  // ms2D_spritebatch_flush(batch);
}
