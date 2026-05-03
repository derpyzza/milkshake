#include "milkshake.h"

static ms_pipeline ms_base_pipeline = {
  .shader = 0,
  .cullmode = MS_CULLMODE_NONE,
  .blend_mode = MS_BLENDMODE_OPAQUE,
  .primitives = MS_PRIM_TRIANGLE,
  .poly_mode = MS_POLY_WIRE,
};

#define MATPART_DEF(_name, _type) (ms_material_part){.name = _name, .kind = _type}

// static ms_material ms_base_material = {
//   .pipeline = &ms_base_pipeline,
//   .mat_parts = (ms_material_part[]) {
//     MATPART_DEF("albedo_tex", MS_MATPART_TEX),
//     MATPART_DEF("normal_tex", MS_MATPART_TEX),
//     MATPART_DEF("roughness_tex", MS_MATPART_TEX),
//     MATPART_DEF("metallic_tex", MS_MATPART_TEX),
//   }
// };


typedef u32 ms_material_id;

ms_material_id create_material_props(ms_shader program, ms_material_part * parts, int num_parts);

ms_shader base_shader;

static ms_material_id ms_base_material;

#define MS_BASEMAT_PROPS \
  MATELEM(ALBEDO, "albedo_tex", MS_MATPART_TEX)\
  MATELEM(NORMALMAP, "normal_tex", MS_MATPART_TEX)\
  MATELEM(ROUGHNESSMAP, "roughness_tex", MS_MATPART_TEX)\
  MATELEM(METALLICMAP, "metallic_tex", MS_MATPART_TEX)\

enum ms_base_material_props {
  #define MATELEM(e, t, k) MS_BASEMAT_ ## e,
  MS_BASEMAT_PROPS
  #undef MATELEM
  MS_BASEMAT_PROPS_COUNT
};

void init_default_resources(void) {
  ms_base_material = create_material_props(
    base_shader,
    (ms_material_part[]){
      #define MATELEM(e, t, k) MATPART_DEF(t, k),
      MS_BASEMAT_PROPS
      #undef MATELEM
    },
    MS_BASEMAT_PROPS_COUNT
  );
}


// usage:

