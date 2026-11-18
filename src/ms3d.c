#include "milkshake/ms3d.h"
#include "cgltf/cgltf.h"
#include "derp.h"
#include "milkshake/milkshake.h"

#define MAX_MESHES 128
int mesh_offsets[MAX_MESHES];

struct mod_ctx {
    ms3d_model * m;
    cgltf_data * data;
    d_allocator alloc;
};

void load_mesh_data (struct mod_ctx ctx, ms_vertex_layout layout);
void load_node_data (struct mod_ctx ctx);
void load_skin_data (struct mod_ctx ctx);
void load_anim_data (struct mod_ctx ctx);
void update_global_transforms (ms3d_model * m);

cgltf_result
_cgltf_file_read_cb (const struct cgltf_memory_options *mem_opts, const struct cgltf_file_options *file_opts, const char *path, cgltf_size *size, void **data) {
    d_allocator alloc = *(d_allocator*)mem_opts->user_data;
    dstr file = dfile_read_alloc(path, alloc);

    if(file.cptr == NULL || file.len == 0) {
        return cgltf_result_io_error;
    }

    *size = file.len;
    *data = file.cptr;
    return cgltf_result_success;
}

void _cgltf_file_release_cb (const struct cgltf_memory_options *mem_opts, const struct cgltf_file_options *file_opts, void *data, cgltf_size size) {
    d_allocator alloc = *(d_allocator*)mem_opts->user_data;
    dalloc_free(alloc, data);
}

void * _cgltf_alloc_cb (void * user, cgltf_size size) {
    d_allocator alloc = *(d_allocator*)user;
    return dalloc_alloc(alloc, size);
}

void _cgltf_free_cb (void * user, void * ptr) {
    d_allocator alloc = *(d_allocator*)user;
    dalloc_free(alloc, ptr);
}


cgltf_data* load_gltf_data ( const char *path, d_allocator alloc ) {
    cgltf_options opts = {0};
    cgltf_data * data = NULL;

    dstr file_data = dfile_read_alloc(path, alloc);
    if(file_data.cptr == NULL || file_data.len == 0) {
        return data;
    }
    opts.file.read    = _cgltf_file_read_cb;
    opts.file.release = _cgltf_file_release_cb;
    opts.memory.alloc_func = _cgltf_alloc_cb;
    opts.memory.free_func  = _cgltf_free_cb;
    opts.memory.user_data  = (void*)&alloc;

    if ( cgltf_parse(&opts, file_data.cptr, file_data.len, &data) != cgltf_result_success ) {
        dlog_error("[GLTF Loader]: Could not parse gltf file: %s", path);
        return NULL;
    };

    if ( cgltf_load_buffers(&opts, data, path) != cgltf_result_success ) {
        dlog_error("[GLTF Loader]: Could not load buffers for file: %s", path);
        return NULL;
    }

    if ( cgltf_validate(data) != cgltf_result_success ) {
        dlog_error("[GLTF Loader]: Could not validate data for file: %s", path);
        return NULL;
    }

    if ( data->file_type == cgltf_file_type_glb ) {
        dlog_trace("[GLTF]: Model basic data (glb) loaded successfully: %s", path);
    }
    else if ( data->file_type == cgltf_file_type_gltf ) {
        dlog_trace("[GLTF]: Model basic data (glTF) loaded successfully: %s", path);
    }
    else {
        dlog_error("[GLTF] model format not recognized: %s", path);
        return NULL;
    }
    return data;
}

static const ms3d_model_opts default_opts = {
    .layout = MS_VERTLAYOUT_POSUVCOL,
    .mat_callback = NULL,

    .load_anims = false,
    .load_skins = false
};

bool ms3d_load_model(ms3d_model *m, const char *path, d_allocator alloc, const ms3d_model_opts * opts) {
    dassert(m != NULL, "nullptr passed to ms3d_load_model");
    if(opts == NULL) opts = (ms3d_model_opts*)&default_opts;

    d_arena scratch = {0};
    darena_init(512, alloc);
    d_allocator scratch_alloc = darena_make_allocator(&scratch);
    cgltf_data* data = load_gltf_data(path, scratch_alloc);

    if(!data) {
        return false;
    } if(data->scenes_count > 1) {
        dlog_warn("[GLTF] file %s has more than one scene (%lu), which is unsupported for now...", path, data->scenes_count);
    }

    for(int i = 0; i < (int)data->meshes_count; i++) {
       m->num_meshes += data->meshes[i].primitives_count;
    }

    // preload mesh offsets
    memset(mesh_offsets, 0, MAX_MESHES);
    int offset = 0;
    for(int i = 0; i < data->meshes_count; i++) {
        mesh_offsets[i] = offset;
        offset += data->meshes[i].primitives_count;
    }

    m->path = (dstr){ (char*)path, strlen(path) };

    m->num_nodes      = data->nodes_count;
    m->num_skins      = data->skins_count;
    m->num_anims      = data->animations_count;
    m->num_textures   = data->textures_count;
    m->num_materials  = data->materials_count;

    dlog_debug("     >> Node count: %i", m->num_nodes);
    dlog_debug("     >> Mesh count: %i", m->num_meshes);
    dlog_debug("     >> Skin count: %i", m->num_skins);
    dlog_debug("     >> Animation count: %i", m->num_anims);
    dlog_debug("     >> Material count: %zu", data->materials_count);
    dlog_debug("     >> Texture count: %i", m->num_textures);
    dlog_debug("     >> Image count: %zu", data->images_count);
    if(data->extras.data) {
        dlog_debug("     >> Extras: %s", data->extras.data);
    }

    struct mod_ctx ctx;
    ctx.alloc = alloc;
    ctx.m = m;
    ctx.data = data;

    m->meshes = d_calloc(m->num_meshes, sizeof(ms3d_mesh));
    m->nodes  = d_calloc(m->num_nodes,  sizeof(ms3d_node));
    if(m->num_textures > 0) {
        // load textures
    }

    if(m->num_materials > 0) {
        // load materials
        if(opts->mat_callback) {
            opts->mat_callback(data->materials, opts->mat_cb_data);
        } else {
            dlog_warn(
                "model %s has materials, but no callback assigned. skipping loading materials", path
            );
        }
    }

    // load all the mesh data and such into the model
    load_mesh_data(ctx, opts->layout);
    load_node_data(ctx);

    if(opts->load_skins && m->num_skins > 0) {
        m->skins = d_calloc(m->num_skins, sizeof(ms3d_skin));
        load_skin_data(ctx);
    }

    if(m->num_anims > 0) {
        m->anims = d_calloc(m->num_anims, sizeof(ms3d_anim));
        load_anim_data(ctx);
    }

    update_global_transforms(m);

    if(data != NULL) {
        cgltf_free(data);
        return false;
    }

    return true;
}

void load_mesh_data (struct mod_ctx ctx, ms_vertex_layout layout) {

    d_arena arena = { 0 };
    darena_init(512, ctx.alloc);
    // temporary allocator
    d_allocator vert_alloc = darena_make_allocator(&arena);

    int mesh_index = 0;
    for (int i = 0; i < (int)ctx.data->meshes_count; i++) {
        cgltf_mesh * cgltf_mesh = &(ctx.data->meshes[i]);
        if(cgltf_mesh->extras.data) dlog_debug("[Mesh::%s] extras: %s", cgltf_mesh->name, cgltf_mesh->extras.data);

        // each primitive turns into an ms3d_mesh
        for(int j = 0; j < (int)cgltf_mesh->primitives_count; j++) {
            const cgltf_primitive* prim = &(cgltf_mesh->primitives[j]);

            if (prim->type != cgltf_primitive_type_triangles) {
                continue;
            }

            ms3d_mesh * cmesh = &(ctx.m->meshes[mesh_index]);
            cmesh->name = dstr_from(cgltf_mesh->name, vert_alloc);
            if(prim->material) {
                int index = (int)(prim->material - ctx.data->materials);
                cmesh->mat_id = index;
            } else {
                cmesh->mat_id = -1;
            }

            const isize num_attrs = prim->attributes_count;
            const isize num_verts = prim->attributes[0].data->count;
            const isize num_index  = prim->indices->count; 
            cmesh->num_verts = num_verts;

            void ** verts = dalloc_calloc(vert_alloc, num_attrs, sizeof(void*));
            uint  * indices = NULL;
            if(num_index != 0) {
                indices = d_calloc(num_index, sizeof(uint));
                cmesh->num_indices = prim->indices->count;
                cgltf_accessor_unpack_indices(prim->indices, indices, sizeof(uint), prim->indices->count);
            }


            for(int a = 0; a < (int)num_attrs; a++) {
                const cgltf_accessor *acc = prim->attributes[a].data;
                const cgltf_attribute_type attr_type = prim->attributes[a].type;

                if(!acc) {
                    dlog_error("[GLTF Model]: NULL accessor");
                    continue;
                }
                
                switch(attr_type) {
                    case cgltf_attribute_type_position: {
                        verts[a] = dalloc_calloc(vert_alloc, num_verts, sizeof(vec3));
                        float * vp = verts[a];
                        for(int v = 0; v < num_verts; v++) {
                            cgltf_accessor_read_float(acc, v, &vp[v*3], 3);
                        }
                        break;
                    };
                    case cgltf_attribute_type_normal: {
                        verts[a] = dalloc_calloc(vert_alloc, num_verts, sizeof(vec3));
                        float * vp = verts[a];
                        for(int v = 0; v < num_verts; v++) {
                            cgltf_accessor_read_float(acc, v, &vp[v*3], 3);
                        }
                        break;
                    };
                    case cgltf_attribute_type_tangent: {
                        verts[a] = dalloc_calloc(vert_alloc, num_verts, sizeof(vec3));
                        float * vp = verts[a];
                        for(int v = 0; v < num_verts; v++) {
                            cgltf_accessor_read_float(acc, v, &vp[v*3], 3);
                        }
                        break;
                    }
                    case cgltf_attribute_type_texcoord: {
                        verts[a] = dalloc_calloc(vert_alloc, num_verts, sizeof(vec2));
                        float * vp = verts[a];
                        for(int v = 0; v < num_verts; v++) {
                            cgltf_accessor_read_float(acc, v, &vp[v*2], 2);
                        }
                        break;      
                    };
                    case cgltf_attribute_type_color: {
                        verts[a] = dalloc_calloc(vert_alloc, num_verts, sizeof(vec4));
                        float * vp = verts[a];
                        for(int v = 0; v < num_verts; v++) {
                            cgltf_accessor_read_float(acc, v, &vp[v*4], 4);
                            // make sure mesh ins't transparent if colour is in RGB
                            // format vs RGBA
                            if (acc->type == cgltf_type_vec3) {
                                vp[v * 4 + 3] = 1.0f;
                            }
                        }
                        break;
                    }
                    case cgltf_attribute_type_joints: {
                        verts[a] = dalloc_calloc(vert_alloc, num_verts, sizeof(ivec4));
                        uint * vp = verts[a];
                        for(int v = 0; v < num_verts; v++) {
                            cgltf_accessor_read_uint(acc, v, &vp[v*4], 4);
                        }
                        break;
                    };
                    case cgltf_attribute_type_weights: {
                        verts[a] = dalloc_calloc(vert_alloc, num_verts, sizeof(vec4));
                        float * vp = verts[a];
                        for(int v = 0; v < num_verts; v++) {
                            cgltf_accessor_read_float(acc, v, vp, 4);
                            f32 sum = vp[v*4 + 0] + vp[v*4 + 1] + vp[v*4 + 2] + vp[v*4 + 3];
                            if (sum > 0.0f) {
                                f32 inv_sum = 1.0f / sum;
                                vp[v*4 + 0] *= inv_sum;
                                vp[v*4 + 1] *= inv_sum;
                                vp[v*4 + 2] *= inv_sum;
                                vp[v*4 + 3] *= inv_sum;
                            }
                        }
                        break;
                    };

                    case cgltf_attribute_type_custom: {
                        isize num_comps = cgltf_num_components(acc->type);
                        bool is_float = (acc->component_type == cgltf_component_type_r_32f) || acc->normalized;

                        if (is_float) {
                            verts[a] = dalloc_calloc(vert_alloc, num_verts * num_comps, sizeof(f32));
                            f32 *vp = verts[a];
                            for (int v = 0; v < num_verts; v++) {
                                cgltf_accessor_read_float(acc, v, &vp[v * num_comps], num_comps);
                            }
                        } else {
                            verts[a] = dalloc_calloc(vert_alloc, num_verts * num_comps, sizeof(u32));
                            u32 *vp = verts[a];
                            for (int v = 0; v < num_verts; v++) {
                                cgltf_accessor_read_uint(acc, v, &vp[v * num_comps], num_comps);
                            }
                        }
                        break;
                    };

                    default:
                        dlog_warn("invalid attribute type: %i", attr_type);
                        return;
                    break;
                }
            }

            // upload_mesh(cmesh, verts, indices);

            darena_clear(&arena);            
            mesh_index++;
        }
    }
}
