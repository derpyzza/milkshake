#pragma once
#include "libderp/derp.h"
#include "milkshake/milkshake.h"

/*
  NOTE:
  > eventually i'd like to add a default texture that represents erroneous or incorrectly loaded textures.
    ideally this should be overridable by the user s they can provide their own texture if they so wish.
  > materials are still kinda weird, i need to figure out what to do with them.
  > idk what to do about vertex data handling. there needs to be some sort of fine-grained system where
    you can extract vertex data from a model or even specific sub-meshes for collision data or whatever
    else and then discard the rest, while it also being super easy to use and manage.
    currently the loader just loads all the vertex data into an arena, uploads it to the GPU, and then
    clears the arena for the next batch of vertices.
*/

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

  mat4s matrix;
  bool dirty;
} ms3d_transform;

typedef struct ms3d_node {
  dstr name;

  int parent_id;
  struct ms3d_node ** children; int num_children;

  ms3d_transform trs;
  mat4s world_mat;

  int mesh_id; int num_meshes;
  int skin_id;
} ms3d_node;

typedef struct ms3d_mesh {
  dstr name;
  ms_vao vao;
  ms_buffer vbo[MS_MAX_VBOS];
  int num_vbos;

  ms_buffer ebo;

  int mat_id;

  isize num_verts, num_indices;
} ms3d_mesh;

typedef struct ms3d_skin {
  dstr name;
  int * joints;
  mat4s * inv_bind;
  int num_joints;
} ms3d_skin;

typedef struct ms3d_anim_track {
  int node_id;

  ms3d_track_type type;
  ms3d_interp interp;

  f32 *times;
  int num_keys;

  vec4s *values;
} ms3d_anim_track;

typedef struct ms3d_anim {
  dstr name;
  f32 duration;

  int num_tracks;
  ms3d_anim_track *tracks;
} ms3d_anim;

typedef struct ms3d_model {
  dstr path;

  int num_meshes
    , num_nodes
    , num_skins
    , num_anims
    , num_textures
    , num_materials
    ;

  ms3d_mesh * meshes;
  ms3d_node * nodes ;
  ms3d_skin * skins ;
  ms3d_anim * anims ;

  // temporary. eventually i should replace textures with
  // indices to textures that are stored by the program itself,
  // so that you can skip loading textures that have already been loaded.
  ms_texture * textures;
  // also temporary, i'm not sure what to do with materials yet.
  void ** materials; // indexes into global material arrays
} ms3d_model;

typedef struct ms3d_model_data {
  int num_meshes, num_nodes, num_skins, num_anims, num_textures, num_materials;
} ms3d_model_data;

struct cgltf_material;
typedef void (*ms3d_mat_callback)(struct cgltf_material* materials, void * user);

typedef struct {
  ms_vertex_layout layout;
  ms3d_mat_callback mat_callback;
  void * mat_cb_data;

  bool load_anims; bool load_skins;
} ms3d_model_opts;

bool ms3d_load_model( ms3d_model * model, const char* path, d_allocator alloc, const ms3d_model_opts * opts );

ms3d_transform ms3d_trs_from_mat4 ( mat4s m );
mat4s ms3d_trs_to_mat4 ( ms3d_transform * t );

