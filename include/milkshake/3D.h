#pragma once
#include "milkshake.h"

typedef struct ms_mesh {
  char name[MS_NAME_LEN]; // optional name

  uint
    vao
  , ebo
  , vbo;

  int
    num_verts
  , num_indices;
} ms_mesh;

void ms_upload_mesh(
  ms_mesh * mesh, // in value
  ms_vertex_layout * layout,
  size_t num_vertices,
  void * vertices,
  size_t num_indices,
  void * indices
);

typedef struct ms_skin {} ms_skin;
typedef struct ms_node {} ms_node;

typedef struct ms_model {
  uint num_meshes;
  uint num_textures;
  uint num_materials;
  uint num_skins;
  uint num_animations;
  uint num_nodes;

  ms_mesh * meshes;
  ms_skin * skin;
  ms_node * nodes;
  u32 * texture_ids;
  u32 * material_ids;
  u32 * animation_ids;
} ms_model;

typedef enum ms_buf_type {
  MS_BUFTYPE_NONE,
  MS_BUFTYPE_VERTEX,
  MS_BUFTYPE_INDEX
};

typedef enum ms_vert_attr_type {
  MS_VERTYPE_NONE,
  MS_VERTYPE_FLOAT,
  MS_VERTYPE_FLOAT_2,
  MS_VERTYPE_FLOAT_3,
  MS_VERTYPE_FLOAT_4,

  MS_VERTYPE_BYTE_4,
  MS_VERTYPE_UBYTE_4,

  MS_VERTYPE_INT,
  MS_VERTYPE_INT_2,
  MS_VERTYPE_INT_3,
  MS_VERTYPE_INT_4,
  MS_VERTYPE_UINT,
  MS_VERTYPE_UINT_2,
  MS_VERTYPE_UINT_3,
  MS_VERTYPE_UINT_4,
} ms_vert_attr_type;

typedef enum ms_vert_attr_kinds {
  MS_VERTKIND_POSITION,
  MS_VERTKIND_NORMAL,
  MS_VERTKIND_COLOUR,
  MS_VERTKIND_TEXCOORD_0,
  MS_VERTKIND_TEXCOORD_1,
  MS_VERTKIND_TANGENT
} ms_vert_attr_kinds;


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


typedef struct ms_buffer {
  int id;
  // ms_buf_type buf_type;
  size_t count, elem_size;
} ms_buffer;

// pipeline state object, similar to the kinds of pipeline objects found in modern graphics apis like Vulkan, or sokol_gfx
typedef struct ms_pipeline {
  ms_shader * shader;
  ms_cull_mode cullmode;
  ms_blend_mode blend_mode;
  ms_draw_primitives primitives;
  ms_poly_mode poly_mode;
} ms_pipeline;


typedef struct ms_vertex_attribute {
  char name[MS_NAME_LEN]; // optional attribute name;
  int index;
  ms_vert_attr_type attr_type;
  bool normalized;
} ms_vertex_attribute;

typedef struct ms_vertex_layout {
  size_t num_attributes;
  ms_vertex_attribute * attributes;
} ms_vertex_layout;

void ms_bind_pipeline(ms_pipeline * p);
