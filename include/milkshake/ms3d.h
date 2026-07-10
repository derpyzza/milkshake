#pragma once
#include "milkshake/milkshake.h"


// batch rendered primitives.
// unlike "proper" 3d models, i wouldn't want to use
// a matrix system with these and would just prefer to upload
// the vertices directly on to the gpu
// they'd be totally immediate mode.
//
// also, they'd be totally untextured, since i wouldn't want
// to use them as anything apart from playtesting stuff.
// the only thing i can see myself using one of these guys for
// is for vfx stuff, but even then i can only see myself using
// the sphere. or like, a half-sphere.
// but even then i feel like it'd be much less painful and more
// effecient to just do that in blender and bake the animations
// or something and export as a mesh?
// idk
//
// im also thinking i should probably split this up into two libraries:
//   - an immediate mode primitive mesh and simple model renderer like
//     raylib which basically just renders a bunch of static models
//   - a generic retained mode renderer that manages a scene tree
//     and renders it out and does all the culling stuff. though
//     this one would be a little annoying to get right while also
//     keeping it lightweight and nice to use.
//     though i suppose all the renderers i'd use for my games would
//     have a bunch of samey things i guess, especially when it comes
//     to things like culling and sorting and draw-call batching and
//     stuff.
//
// for any 3D game, id need:
//  - skeletal animations ( generic )
//  - billboards? ( generic )
//  - draw call ordering ( generic )
//  - camera management ( generic )
//  - frustum culling ( generic )
//  - some sort of multi-pass rendering ( generic )
//  - shadow mapping ( generic-ish, and also not necessary for every game )
//  - lighting system ( mostly generic )
//  - occlusion culling ( potentially non-generic? )
//  - materials system ( not generic )

typedef struct {} ms3d_camera;
typedef struct {} ms3d_node;
typedef struct {} ms3d_mesh;
typedef struct {} ms3d_material;
typedef struct {} ms3d_transform;
typedef struct {} ms3d_animation;
typedef struct {} ms3d_skeleton;
typedef struct {} ms3d_pose;

typedef struct {
  int num_meshes
    , num_nodes
    , num_skeletons
    , num_animations
    , num_textures
    , num_materials
    ;
  
  ms3d_mesh * meshes;
  ms3d_node * nodes;
  ms3d_skeleton * skeletons;
  ms3d_animation * animations;

  int * textures;
  int * materials; // ids into some global materials array?
} ms3d_model;

typedef struct {
  ms3d_model * model;
  ms3d_transform transform;

  ms3d_pose pose;
  mat4s* final_skeleton;
} ms3d_model_inst;

void ms3d_cube(vec3s pos, f32 size, uint colour);
void ms3d_cuber(vec3s pos, f32 size, vec3s rot, uint colour);
void ms3d_cuboid(vec3s pos, vec3s scale, uint colour);
void ms3d_cuboidr(vec3s pos, vec3s scale, vec3s rot, uint colour);

void ms3d_sphere(vec3s center, f32 rad, uint colour);
// some kind of half sphere function too?

// void ms3d_ellipse(vec3s center, vec3s size, uint colour);
// void ms3d_ellipser(vec3s center, vec3s size, vec3s rot, uint colour);

// for both cylinder and capsule, the `pos` variable is where the bottom-most point is
// plus the radius for the capsule.
// however, `bottom_pos` indicates the bottom-most point *minus* the radius.
// in other words, `pos` is the center point of the bottom-most sphere + it's radius,
// and `bottom_pos` is the center point of the bottom-most sphere.
// obviously the radius stuff only applies to the capsule and not the cylinder
void ms3d_cylinder(vec3s pos, f32 height, f32 rad, uint colour);
void ms3d_cylindero(vec3s bottom_pos, vec3s top_pos, f32 rad, uint colour);
void ms3d_cylinderpro(vec3s pos, f32 height, f32 top_rad, f32 bottom_rad, int segments, int rings, uint colour);

void ms3d_capsule(vec3s pos, f32 height, f32 rad, uint colour);
void ms3d_capsuleo(vec3s bottom_pos, vec3s top_pos, f32 rad, uint colour);
void ms3d_capsulepro(vec3s bottom_pos, vec3s top_pos, f32 rad, int segments, int rings, uint colour);
