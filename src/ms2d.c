/* milkshake
 * 
 * Copyright (c) 2026 derpyzza
 * 
 * This file is released under the terms of the MIT license.
 * Read LICENSE for more information.
 *
 * no LLMS were used to write the code in this library.
*/ 

#include "internal.h"
#include "milkshake/milkshake.h"
#include <milkshake/ms2d.h>

static ms2d_renderbatch * cur_batch;

void ms2d_bind_renderbatch(ms2d_renderbatch* batch) {
  cur_batch = batch;
}

ms2d_renderbatch ms2d_new_renderbatch(void) {
  return ms2d_create_renderbatch(MS2D_DEFAULT_QUADS, MS2D_DEFAULT_DRAWCALLS);
}

ms2d_renderbatch ms2d_create_renderbatch(isize num_quads, isize num_drawcalls) {
  ms2d_renderbatch out = { 0 };
  const isize num_verts = num_quads * 4;
  const isize num_indices = num_verts * 6;

  out.num_drawcalls = 1;

  out.max_verts = num_verts;
  out.max_drawcalls = num_drawcalls;

  out.verts     = (sprite_vertex*)d_calloc(out.max_verts,     sizeof(sprite_vertex));
  out.drawcalls = (ms2d_drawcall*)d_calloc(out.max_drawcalls, sizeof(ms2d_drawcall));

  for(int i = 0; i < out.max_drawcalls; i++) {
    ms2d_drawcall * call = &out.drawcalls[i];
    call->mode = MS2D_QUADS;
    call->texture = ms2d_default_texture();
    call->num_verts = 0;
    call->vert_align = 0;
  }


  out.vao = ms_create_vao();
  out.vbo = ms_create_buffer(
    MS_BufferType_Vertex,
    MS_BufferUsage_DynamicDraw,
    sizeof(sprite_vertex) * num_verts,
    NULL
  );

  // preload indices
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

  ms_vao_attach_vbo(out.vao, out.vbo, MS2D_VERTLAYOUT_SPRITE);
  ms_vao_attach_ebo(out.vao, out.ebo);

  return out;
}

void ms2d_flush(void) {
  glBindBuffer(GL_ARRAY_BUFFER, cur_batch->vbo.id); 
  glBufferSubData(
      GL_ARRAY_BUFFER, 
      0, 
      cur_batch->num_verts * sizeof(sprite_vertex), 
      cur_batch->verts
  );

  for(int i = 0, offset = 0; i < cur_batch->num_drawcalls; i++) {
    ms2d_drawcall * call = &cur_batch->drawcalls[i];
    ms_bind_texture(call->texture);

    if(call->mode == MS2D_LINES || call->mode == MS2D_TRIS) {
      int prim_mode = call->mode == MS2D_LINES ? GL_LINES : GL_TRIANGLES;
      ms_draw_arrays(cur_batch->vao, prim_mode, offset, call->num_verts);
    } else {
      ms_draw_elems(
        cur_batch->vao,
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        call->num_verts/4*6,
        (const int*)(offset/4*6*sizeof(GLuint))
      );
    }

    offset += call->num_verts + call->vert_align;
  }

  // reset state
  for(int i = 0; i < cur_batch->max_drawcalls; i++) {
    ms2d_drawcall * call = &cur_batch->drawcalls[i];
    call->mode = MS2D_QUADS;
    call->texture = ms2d_default_texture();
    call->num_verts = 0;
    call->vert_align = 0;
  }

  cur_batch->num_verts = 0;
  cur_batch->num_drawcalls = 1;
}

void ms2d_rect(f32 x, f32 y, f32 w, f32 h, uint colour) {
  sprite_vertex verts[4] = {
    { {{x, y    }}, {{0, 0}}, ms_hex_to_bytes(colour) },
    { {{x, y+h  }}, {{0, 0}}, ms_hex_to_bytes(colour) },
    { {{x+w, y+h}}, {{0, 0}}, ms_hex_to_bytes(colour) },
    { {{x+w, y  }}, {{0, 0}}, ms_hex_to_bytes(colour) },
  };

  ms2d_poly_submit(MS2D_QUADS, ms2d_default_texture(), verts, 4);
}

void ms2d_rectr(f32 x, f32 y, f32 w, f32 h, f32 rot, uint colour) {
  ms2d_rectangle r = { x, y, w, h };
  ms2d_rectpro(r, GLMS_VEC2_ZERO, rot, colour);
}

void ms2d_rectv(vec2s pos, vec2s size, f32 rot, uint colour) {
  ms2d_rectangle r = { pos.x, pos.y, size.x, size.y };
  ms2d_rectpro(r, GLMS_VEC2_ZERO, rot, colour);
}

void ms2d_rectpro(ms2d_rectangle r, vec2s origin, f32 rot, uint colour) {
  ms2d_rectangle src = { 0, 0, r.w, r.h };
  ms2d_texrectpro(ms2d_default_texture(), src, r, origin, rot, colour, false, false);
}

void ms2d_texrect(ms_texture tex, ms2d_rectangle src, ms2d_rectangle dest, uint colour) {
  ms2d_texrectpro(tex, src, dest, GLMS_VEC2_ZERO, 0.0f, colour, false, false);
}

void ms2d_texrectr(ms_texture tex, ms2d_rectangle src, ms2d_rectangle dest, f32 rot, uint colour) {
  ms2d_texrectpro(tex, src, dest, GLMS_VEC2_ZERO, rot, colour, false, false);
}

void ms2d_texrectw(ms_texture tex, vec2s pos, uint colour, bool flip_x, bool flip_y) {
  ms2d_rectangle src = { 0, 0, tex.width, tex.height };
  ms2d_rectangle dest = { pos.x, pos.y, tex.width, tex.height };
  ms2d_texrectpro(tex, src, dest, GLMS_VEC2_ZERO, 0.0f, colour, flip_x, flip_y);
}

void ms2d_texrectpro(ms_texture tex, ms2d_rectangle src, ms2d_rectangle dest, vec2s origin, f32 rot, uint colour, bool flip_x, bool flip_y) {

  sprite_vertex verts[4] = { 0 };

  vec2s voffsets[4] = {
    {{     0,      0}},
    {{dest.w,      0}},
    {{dest.w, dest.h}},
    {{     0, dest.h}}
  };

  vec2s toffsets[4] = {
    {{    0,     0}},
    {{src.w,     0}},
    {{src.w, src.h}},
    {{    0, src.h}}
  };

  vec2s norm_origin = glms_vec2_scale(glms_vec2_add(origin, GLMS_VEC2_ONE), 0.5);
  vec2s origin_offset = glms_vec2_mul(norm_origin, (vec2s){{dest.w, dest.h}});

  float rad = glm_rad(rot);
  float cosa = cosf(rad);
  float sina = sinf(rad);
  u8 bytes[4] = ms_hex_to_bytes(colour);

  for(int i = 0; i < 4; i++) {
    float local_x = voffsets[i].x - origin_offset.x;
    float local_y = voffsets[i].y - origin_offset.y;

    // rotated coordinates in local-space ( pixels )
    float rot_x = local_x * cosa - local_y * sina;
    float rot_y = local_x * sina + local_y * cosa;

    // final coordinates in world-space ( pixels )
    verts[i].pos.x = rot_x + dest.x;
    verts[i].pos.y = rot_y + dest.y;

    float t_u;
    if(flip_x) {
      t_u = src.x + (src.w - toffsets[i].x);
    } else {
      t_u = src.x + toffsets[i].x;
    }
    float t_v;
    if(flip_y) {
      t_v = src.y + (src.h - toffsets[i].y);
    } else {
      t_v = src.y + toffsets[i].y;
    }

    verts[i].uv.x = t_u / tex.width ;
    verts[i].uv.y = t_v / tex.height;
    
    memcpy(verts[i].col, bytes, sizeof(u8)*4);
  }

  ms2d_poly_submit(MS2D_QUADS, tex, verts, 4);
}

void ms2d_colquad  (vec2s a, vec2s b, vec2s c, vec2s d, uint colour) {
  ms2d_quad q = { a, b, c, d };
  ms2d_texquad(ms2d_default_texture(), q, colour);
}

void ms2d_colquadq (ms2d_quad quad, uint colour) {
  ms2d_texquad(ms2d_default_texture(), quad, colour);
}

void ms2d_texquad  (ms_texture tex, ms2d_quad q, uint colour) {
  sprite_vertex v[4] = {
    { {{q.a.x, q.a.y}}, {{0, 0}}, ms_hex_to_bytes(colour) },
    { {{q.b.x, q.b.y}}, {{1, 0}}, ms_hex_to_bytes(colour) },
    { {{q.c.x, q.c.y}}, {{1, 1}}, ms_hex_to_bytes(colour) },
    { {{q.d.x, q.d.y}}, {{0, 1}}, ms_hex_to_bytes(colour) }
  };

  ms2d_poly_submit(MS2D_QUADS, tex, v, 4);
}

void ms2d_circ(f32 x, f32 y, f32 r, uint colour) {
  const int num_quads = 9;
  const int total_verts = num_quads * 4;
  sprite_vertex verts[36];

  f32 angle_step = (f32)(2.0 * M_PI / (num_quads * 2));
  int v_idx = 0;

  uint8_t col_bytes[4] = ms_hex_to_bytes(colour);

  for (int i = 0; i < num_quads; i++) {
    f32 angle_a = (i * 2 + 0) * angle_step;
    f32 angle_b = (i * 2 + 1) * angle_step;
    f32 angle_c = (i * 2 + 2) * angle_step;

    verts[v_idx].pos.x = x;
    verts[v_idx].pos.y = y;
    verts[v_idx].uv.x  = 0.0f;
    verts[v_idx].uv.y  = 0.0f;
    memcpy(verts[v_idx].col, col_bytes, sizeof(u8)*4);
    v_idx++;

    verts[v_idx].pos.x = x + cosf(angle_a) * r;
    verts[v_idx].pos.y = y + sinf(angle_a) * r;
    verts[v_idx].uv.x  = 0.0f;
    verts[v_idx].uv.y  = 0.0f;
    memcpy(verts[v_idx].col, col_bytes, sizeof(u8)*4);
    v_idx++;

    verts[v_idx].pos.x = x + cosf(angle_b) * r;
    verts[v_idx].pos.y = y + sinf(angle_b) * r;
    verts[v_idx].uv.x  = 0.0f;
    verts[v_idx].uv.y  = 0.0f;
    memcpy(verts[v_idx].col, col_bytes, sizeof(u8)*4);
    v_idx++;

    verts[v_idx].pos.x = x + cosf(angle_c) * r;
    verts[v_idx].pos.y = y + sinf(angle_c) * r;
    verts[v_idx].uv.x  = 0.0f;
    verts[v_idx].uv.y  = 0.0f;
    memcpy(verts[v_idx].col, col_bytes, sizeof(u8)*4);
    v_idx++;
  }

  ms2d_poly_submit(MS2D_QUADS, ms2d_default_texture(), verts, total_verts);
}

void ms2d_poly_submit(ms2d_drawmode mode, ms_texture tex, sprite_vertex* verts, uint num_verts) {
  // flush batch if the limit's been met
  if(cur_batch->num_verts+num_verts >= cur_batch->max_verts) {
    ms2d_flush();
  }

  // create a new draw call if either the mode or the texture have changed
  // and do alignment stuff
  ms2d_drawcall * call = &cur_batch->drawcalls[cur_batch->num_drawcalls-1];
  if(call->mode != mode || call->texture.id != tex.id) {
    // create new draw call
    if (call->num_verts > 0) {
      if(call->mode == MS2D_LINES) {
        call->vert_align = (call->num_verts < 4) ? call->num_verts : call->num_verts%4;
      } else if (call->mode == MS2D_TRIS) {
        call->vert_align = (4 - (call->num_verts % 4)) % 4;
      } else {
        call->vert_align = 0;
      }
      bool overflow = false;
      if(!overflow) {
        cur_batch->num_verts += call->vert_align;
        cur_batch->num_drawcalls++;
        call = &cur_batch->drawcalls[cur_batch->num_drawcalls-1];
      }
    }

    if(cur_batch->num_drawcalls >= cur_batch->max_drawcalls) {
      ms2d_flush();
    }
    call->mode = mode;
    call->texture = tex;
  }

  // submit vertices to buffer
  for(int i = 0; i < num_verts; i++) {
    cur_batch->verts[cur_batch->num_verts] = verts[i];
    cur_batch->num_verts++;
  }

  call->num_verts += num_verts;
}

ms2D_spritebatch
ms2D_create_spritebatch(ms_texture texture, isize num_sprites) {
  isize num_verts = num_sprites * 4;
  isize num_indices = num_sprites * 6;
  isize verts_size = sizeof(sprite_vertex) * num_verts;

  ms2D_spritebatch out = { 0 };
  out.texture = texture;
  out.capacity = num_sprites;


  out.verts = (sprite_vertex*)d_alloc(verts_size);
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

  ms_vao_attach_vbo(out.vao, out.vbo, MS2D_VERTLAYOUT_SPRITE);
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
    
    u8 bytes[4] = ms_hex_to_bytes(sprite.colour);
    memcpy(batch->current->col, bytes, sizeof(u8)*4);

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
