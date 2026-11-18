#pragma once
#include "milkshake/milkshake.h"

typedef enum ms3d_interp {
	MS3D_INTERP_STEP,
	MS3D_INTERP_LINEAR,
	MS3D_INTERP_CUBIC,
	MS3D_NUM_INTERPOLATIONS,
} ms3d_interp;

typedef enum {
  MS3D_TRACK_NONE,
	MS3D_TRACK_TRANSLATION,
	MS3D_TRACK_SCALE,
	MS3D_TRACK_ROTATION,
	MS3D_NUM_TRACK_TYPES
} ms3d_track_type;

typedef struct ms3d_transform {
  vec3s   pos;
  vec3s   scale;
  versors rot;

  mat4s mat_cached;
  bool dirty;
} ms3d_transform;

typedef struct ms3d_node {
  dstr name;

  int parent_id;
  struct ms3d_node * children; int num_children;

  ms3d_transform trs;

  int mesh_id; int num_meshes;
  int skin_id;
} ms3d_node;

typedef struct ms3d_mesh {
  dstr name;
  ms_vao vao;
  ms_buffer *vbo;
  int num_vbos;

  ms_buffer ebo;

  uint mat_id;

  isize num_verts, num_indices;
} ms3d_mesh;

typedef struct ms3d_skin {
  int * joints;
  mat4s * inv_bind;
  int num_joints;
} ms3d_skin;

typedef struct ms3d_anim_samp {
  f32 *times;
  f32 *values;
  int count;
  ms3d_interp interp;
} ms3d_anim_samp;

typedef struct ms3d_anim_track {
  int node;
  ms3d_track_type path;
  int sampler;
} ms3d_anim_track;

typedef struct ms3d_anim {
  char name[MS_NAME_LEN];
  f32 duration;

  ms3d_anim_track *channels;
  ms3d_anim_samp *samplers;
  int num_channels;
} ms3d_anim;

typedef struct ms3d_model {
  dstr path;

  int num_meshes
    , num_nodes
    , num_skins
    , num_anims
    // these two actually aren't stored by the model
    // itself, and are instead held in global caches.
    // the num_* fields exist purely for checking
    // purposes.
    , num_textures
    , num_materials
    ;

  ms3d_mesh * meshes;
  ms3d_node * nodes ;
  ms3d_skin * skins ;
  ms3d_anim * anims ;
  void * materials;
} ms3d_model;

struct cgltf_material;
typedef void (*ms3d_mat_callback)(struct cgltf_material* materials, void * user);

typedef struct {
  ms_vertex_layout layout;
  ms3d_mat_callback mat_callback;
  void * mat_cb_data;

  bool load_anims; bool load_skins;
} ms3d_model_opts;

bool ms3d_load_model( ms3d_model * model, const char* path, d_allocator alloc, const ms3d_model_opts * opts );
