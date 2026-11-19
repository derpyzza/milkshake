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

ms3d_transform trs_from_mat4 ( mat4s m ) {
	ms3d_transform t = { 0 };
	
	t.pos = (vec3s){{ m.col[3].x, m.col[3].y, m.col[3].z }};

	vec3s col0 = (vec3s){{m.col[0].x, m.col[0].y, m.col[0].z}};
	vec3s col1 = (vec3s){{m.col[1].x, m.col[1].y, m.col[1].z}};
	vec3s col2 = (vec3s){{m.col[2].x, m.col[2].y, m.col[2].z}};

	t.scale.x = glms_vec3_norm(col0);
	t.scale.y = glms_vec3_norm(col1);
	t.scale.z = glms_vec3_norm(col2);

	if (t.scale.x != 0.0f) col0 = glms_vec3_scale(col0, 1.0f / t.scale.x);
	if (t.scale.y != 0.0f) col1 = glms_vec3_scale(col1, 1.0f / t.scale.y);
	if (t.scale.z != 0.0f) col2 = glms_vec3_scale(col2, 1.0f / t.scale.z);

	mat3s rot;
	rot.col[0] = col0;
	rot.col[1] = col1;
	rot.col[2] = col2;

	t.rot = glms_mat3_quat(rot);

	return t;
}

mat4s trs_to_mat4 ( ms3d_transform * t ) {
	if (!t->dirty) {
	 return t->matrix;
	}
	
  mat4s mt, mr, ms;
  mt = glms_translate_make(t->pos);
  mr = glms_quat_mat4(t->rot);
  ms = glms_scale_make(t->scale);

  t->matrix = glms_mat4_mul(mt, glms_mat4_mul(mr, ms));
  return t->matrix;
}


void load_mesh_data (struct mod_ctx ctx, ms_vertex_layout layout);
void load_node_data (struct mod_ctx ctx);
void load_skin_data (struct mod_ctx ctx);
void load_anim_data (struct mod_ctx ctx);
void update_global_transforms (ms3d_model * m);

void update_global_transforms(ms3d_model * m) {
    for (int i = 0; i < m->num_nodes; i++) {
        ms3d_node * n = &m->nodes[i];

        ms3d_node * parent = &m->nodes[n->parent_id];
        if(n->parent_id >= 0) {
            n->world_mat = glms_mat4_mul(parent->world_mat , trs_to_mat4(&n->trs));
        } else {
            n->world_mat = trs_to_mat4(&n->trs);
        }
    }
}

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
    .mat_cb_data = NULL,

    .load_anims = false,
    .load_skins = false
};

bool ms3d_load_model(ms3d_model *m, const char *path, d_allocator alloc, const ms3d_model_opts * opts) {
    dassert(m != NULL, "nullptr passed to ms3d_load_model");
    if(opts == NULL) opts = (ms3d_model_opts*)&default_opts;

    d_arena scratch = darena_init(512, alloc);
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

void load_node_data(struct mod_ctx ctx) {
    for(int i = 0; i < ctx.m->num_nodes; i++) {
        const cgltf_node *src = &ctx.data->nodes[i];
        ms3d_node *dest = &(ctx.m->nodes[i]);
        dest->name = dstr_from(src->name, ctx.alloc);

        if(src->extras.data) {
            dlog_debug("[Node::%.*s] extras: %s", dstr_fmt(dest->name), src->extras.data);
        }

        if (src->parent) {
            dest->parent_id = (int)(src->parent - ctx.data->nodes);
        } else {
            dest->parent_id = -1;
        }

        if (src->children) {
            dest->children = dalloc_calloc(ctx.alloc, src->children_count, sizeof(ms3d_node*));
            for(int i = 0; i < src->children_count; i++) {
                cgltf_node* node = src->children[i];
                int index = (int)(node - ctx.data->nodes);
                dest->children[i] = ctx.m->nodes[index];
            }
        }

        // mat4s tmp;
        cgltf_node_transform_local(src, (f32*)dest->trs.matrix.raw);

        dest->trs = ms3d_trs_from_mat4(dest->trs.matrix);
        dest->world_mat = glms_mat4_identity();

        dest->mesh_id = -1;
        dest->skin_id = -1;

        if(src->skin) {
            dest->skin_id = (int)(src->skin - ctx.data->skins);
        }

        if(src->mesh) {
            dest->mesh_id = mesh_offsets[src->mesh - ctx.data->meshes];
            dest->num_meshes = src->mesh->primitives_count;
        }
    }
}


void load_skin_data (struct mod_ctx ctx) {
    for(int i = 0; i < (int)ctx.data->skins_count; i++) {
        const cgltf_skin * src = &ctx.data->skins[i];
        ms3d_skin * skin = &(ctx.m->skins[i]);
        skin->name = dstr_from(src->name, ctx.alloc);

        skin->num_joints = src->joints_count;
        skin->joints = d_calloc(skin->num_joints, sizeof(int));
        skin->inv_bind = d_calloc(skin->num_joints, sizeof(mat4s));

        for(int j = 0; j < skin->num_joints; j++) {
            skin->joints[j] = (int)(src->joints[j] - ctx.data->nodes);            

            if(src->inverse_bind_matrices) {
                cgltf_accessor_read_float(
                    src->inverse_bind_matrices,
                    j,
                    (f32*)skin->inv_bind[j].raw,
                    16);
            } else {
                skin->inv_bind[j] = glms_mat4_identity();
            }
        }
    }
}


ms3d_track_type get_track_type(cgltf_animation_channel ch) {
    switch(ch.target_path) {
        case cgltf_animation_path_type_translation:
            return MS3D_TRACK_TRANSLATION;
        case cgltf_animation_path_type_scale:
            return MS3D_TRACK_SCALE;
        case cgltf_animation_path_type_rotation:
            return MS3D_TRACK_ROTATION;
        default: return MS3D_TRACK_NONE;
    }
}

ms3d_anim_track load_track( const cgltf_animation_channel * ch, const cgltf_animation_sampler * samp, const cgltf_data* data) {
    ms3d_anim_track track = {0};

    track.node_id = (int)(ch->target_node - data->nodes);

    track.num_keys = samp->input->count;
    track.values = d_alloc(sizeof(vec4s) * track.num_keys);

    track.times = d_alloc(sizeof(f32) * track.num_keys);
    for ( int i = 0; i < track.num_keys; i++ ) {
        cgltf_accessor_read_float(samp->input, i, &track.times[i], 1);
    }

    track.type = get_track_type(*ch);

    switch(samp->interpolation) {
        case cgltf_interpolation_type_step: track.interp = MS3D_INTERP_STEP; break;
        case cgltf_interpolation_type_cubic_spline: track.interp = MS3D_INTERP_CUBIC; break;
        default: track.interp = MS3D_INTERP_LINEAR; break;
    }

    cgltf_accessor * out = samp->output;
    switch(track.type) {
        case MS3D_TRACK_TRANSLATION:
            for (int i = 0; i < track.num_keys; i++) {
                float v[3];
                cgltf_accessor_read_float(out, i, v, 3);
                track.values[i] = (vec4s){{ v[0], v[1], v[2], 0}};
            }
        break;
        case MS3D_TRACK_SCALE:
            for (int i = 0; i < track.num_keys; i++) {
                float v[3];
                cgltf_accessor_read_float(out, i, v, 3);
                track.values[i] = (vec4s){{ v[0], v[1], v[2], 0 }};
            }
        break;
        case MS3D_TRACK_ROTATION:
            for (int i = 0; i < track.num_keys; i++) {
                float q[4];
                cgltf_accessor_read_float(out, i, q, 4);
                track.values[i] = (vec4s){{ q[0], q[1], q[2], q[3] }};
            }
        break;
        default:;
    }

    return track;
}

void load_animation_data( struct mod_ctx ctx ) {
    for(int i = 0; i < ctx.m->num_anims; i++) {
        const cgltf_animation *src = &ctx.data->animations[i];
        ms3d_anim * dest = &ctx.m->anims[i];
        dest->num_tracks = src->channels_count;
        dest->tracks = d_calloc(dest->num_tracks, sizeof(ms3d_anim_track));
        dest->name = dstr_from(src->name, ctx.alloc);

        f32 max_time = 0;

        for(int j = 0; j < dest->num_tracks; j++) {
            ms3d_anim_track * track = &dest->tracks[j];

            cgltf_animation_channel * ch = &src->channels[j];
            cgltf_animation_sampler * samp = ch->sampler;

            *track = load_track(ch, samp, ctx.data);

            if(track->num_keys > 0) {
                f32 last = track->times[track->num_keys - 1];
                if (last > max_time) {
                    max_time = last;
                }
            }
        }
        dest->duration = max_time;
    }
}


struct vert_data {
    ms_vert_attr_kinds attrib;
    int kind_index;
    dstr name;
    float * _floats;
    uint  * _uints;
    // for custom vertex attributes
    isize num_comps; bool is_float;
};

bool _read_attr(const cgltf_accessor * acc, d_allocator alloc, bool is_float, int num_comps, struct vert_data* out) {
    isize num_verts = acc->count;
    isize src_comps = cgltf_num_components(acc->type);
    out->num_comps = num_comps;
    out->is_float = is_float;

    if (is_float) {
        float* p = dalloc_calloc(alloc, num_verts * num_comps, sizeof(float));
        for (isize v = 0; v < num_verts; v++) {
            if (!cgltf_accessor_read_float(acc, v, &p[v*num_comps], src_comps)) {
                out->_floats = p;
                return false;
            }
            for (isize c = src_comps; c < num_comps; c++) {
                p[v*num_comps + c] = 1.0f; // RGB -> RGBA
            }
        }
        out->_floats = p;
    } else {
        uint* p = dalloc_calloc(alloc, num_verts * num_comps, sizeof(uint));
        for (isize v = 0; v < num_verts; v++) {
            if (!cgltf_accessor_read_uint(acc, v, &p[v*num_comps], src_comps)) {
                out->_uints = p;
                return false;
            }
        }
        out->_uints = p;
    }
    return true;
}

bool _validate_layout(dstr name, ms_vertex_layout layout, struct vert_data* verts, isize num_attrs) {
    for(int i = 0; i < layout.num_attribs; i++) {
        bool has_attr = false;
        for(int j = 0; j < num_attrs; j++) {
            if(layout.attribs[i].attr_kind == verts[j].attrib) {
                has_attr = true;
                break;
            }
        }

        if(!has_attr) {
            dlog_error("[MESH] %.*s requires attribute that input mesh does not have", dstr_fmt(name));
            return false;
        }
    }
    return true;
}

const struct vert_data * _find_source(struct vert_data * verts, isize num_attrs, ms_vert_attr_kinds kind, int set_id) {
    for(int i = 0; i < num_attrs; i++) {
        const struct vert_data * out = &verts[i];
        if(out->attrib == kind && out->kind_index == set_id) {
            return out;
        }
    }
    return NULL;
}

void _pack_attr(u8* buffer, isize stride, const struct vert_data * vdata, const ms_vertex_attrib* attr, isize num_attrs) {
    for (isize v = 0; v < num_attrs; v++) {
        u8* dptr = buffer + v * stride;
        for (int c = 0; c < attr->size; c++) {
            // fetch as float or uint, with defaults when there's no source
            float f = 0.0f;
            uint u = 0;
            if (vdata && c < vdata->num_comps) {
                if (vdata->is_float) {
                    f = vdata->_floats[v*vdata->num_comps + c];
                    u = (uint)f;
                }
                else {
                    u = vdata->_uints [v*vdata->num_comps + c];
                    f = (float)u;
                }
            } else if (attr->attr_kind == MS_VertKind_Colour_N) {
                f = 1.0f;
                u = 255;
            }

            switch (attr->type) {
                case GL_FLOAT:
                    ((float*)dptr)[c] = f;
                break;
                case GL_UNSIGNED_INT:
                    ((uint*) dptr)[c] = u;
                break;
                case GL_UNSIGNED_SHORT:
                    if(attr->normalized && vdata && vdata->is_float) {
                        ((u16*)dptr)[c] = (u16)(MIN(MAX(f, 0), 1) * 65535.0f + 0.5f);
                    } else {
                        ((u16*)dptr)[c] = (u16)u;
                    }
                break;
                case GL_UNSIGNED_BYTE:
                    if (attr->normalized && vdata && vdata->is_float) {
                        dptr[c] = (u8)(MIN(MAX(f,0),1) * 255.0f + 0.5f);
                    } else {
                        dptr[c] = (u8)u;
                    }
                break;
            }
        }
    }
}

bool upload_mesh(ms3d_mesh* mesh, ms_vertex_layout layout, struct vert_data * verts, isize num_attrs, uint* indices, d_allocator alloc) {
    if(!mesh) {
        dlog_error("[MESH] mesh is null");
        return false;
    }
    if(!verts) {
        dlog_error("[MESH] verts array is null");
        return false;
    }
    if (layout.num_vbos <= 0) {
        dlog_error("[MESH] layout has no vbos");
        return false;
    }
    if (mesh->num_indices > 0 && !indices) {
        dlog_error("[MESH] indices missing");
        return false;
    }
    for (isize i = 0; i < layout.num_attribs; i++) {
        if (layout.attribs[i].vbo < 0 || layout.attribs[i].vbo >= layout.num_vbos) {
            dlog_error("[MESH] attribute %li has bad vbo slot", i);
            return false;
        }
    }

    mesh->num_vbos = layout.num_vbos;

    glGenVertexArrays(1, &mesh->vao.id);
    glBindVertexArray(mesh->vao.id);

    for(int i = 0; i < layout.num_vbos; i++) {
        glGenBuffers(1, &mesh->vbo[i].id);
    }

    if(mesh->num_indices > 0) {
        glGenBuffers(1, &mesh->ebo.id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo.id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(uint), indices, GL_STATIC_DRAW);
    }

    int last_vbo = -1;
    for(int i = 0; i < layout.num_vbos; i++) {
        int stride = layout.strides[i];
        u8* buf = dalloc_calloc(alloc, mesh->num_verts, stride);

        // pack attribute arrays into single buffer per vbo
        for(int j = 0; j < layout.num_attribs; j++) {
            const ms_vertex_attrib* attr = &layout.attribs[i];
            if (attr->vbo != i) continue;
            const struct vert_data* src = _find_source(verts, num_attrs, attr->attr_kind, attr->attr_id);
            _pack_attr(buf + attr->offset, stride, src, attr, mesh->num_verts);
        }

        glGenBuffers(1, &mesh->vbo[i].id);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo[i].id);
        glBufferData(GL_ARRAY_BUFFER, mesh->num_verts * stride, buf, GL_STATIC_DRAW);
        
        for (isize j = 0; j < layout.num_attribs; j++) {
            const ms_vertex_attrib* a = &layout.attribs[j];
            if (a->vbo != i) continue;

            glEnableVertexAttribArray(a->loc);
            
            void* off = (void*)(intptr_t)a->offset;
            if (a->type != GL_FLOAT && !a->normalized) {
                glVertexAttribIPointer(a->loc, a->size, a->type, stride, off);
            } else {
                glVertexAttribPointer(a->loc, a->size, a->type, a->normalized, stride, off);
            }
        }
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

void _norm_weights(float * vp, isize num_verts) {
    for(int v = 0; v < num_verts; v++) {
        f32 sum = vp[v*4 + 0] + vp[v*4 + 1] + vp[v*4 + 2] + vp[v*4 + 3];
        if (sum > 0.0f) {
            f32 inv_sum = 1.0f / sum;
            vp[v*4 + 0] *= inv_sum;
            vp[v*4 + 1] *= inv_sum;
            vp[v*4 + 2] *= inv_sum;
            vp[v*4 + 3] *= inv_sum;
        }
    }
}

void load_mesh_data (struct mod_ctx ctx, ms_vertex_layout layout) {

    d_arena vert_arena = darena_init(512, ctx.alloc);
    // temporary allocator
    d_allocator vert_alloc = darena_make_allocator(&vert_arena);

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
            cmesh->name = dstr_from(cgltf_mesh->name, ctx.alloc);

            if(prim->material) {
                int index = (int)(prim->material - ctx.data->materials);
                cmesh->mat_id = index;
            } else {
                cmesh->mat_id = -1;
            }

            const isize num_attrs = prim->attributes_count;
            const isize num_verts = prim->attributes[0].data->count;
            isize num_index = 0;

            cmesh->num_verts = num_verts;

            struct vert_data * verts = dalloc_calloc(vert_alloc, num_attrs, sizeof(struct vert_data));
            uint  * indices = NULL;
            if(prim->indices != NULL) {
                num_index = prim->indices->count; 
                indices = dalloc_calloc(vert_alloc, num_index, sizeof(uint));
                cmesh->num_indices = prim->indices->count;
                cgltf_accessor_unpack_indices(prim->indices, indices, sizeof(uint), prim->indices->count);
            }

            isize loaded = 0;
            for(int a = 0; a < (int)num_attrs; a++) {
                const cgltf_accessor *acc = prim->attributes[a].data;
                const cgltf_attribute_type attr_type = prim->attributes[a].type;

                if(!acc) {
                    dlog_error("[GLTF Model]: NULL accessor");
                    continue;
                }

                struct vert_data vdata = { .kind_index = prim->attributes[a].index };
                bool is_float = true;
                int comps = (int)cgltf_num_components(acc->type);
                
                switch(attr_type) {
                    case cgltf_attribute_type_position:
                        vdata.attrib = MS_VertKind_Position;
                        comps = 3;
                    break;
                    case cgltf_attribute_type_normal:
                        vdata.attrib = MS_VertKind_Normal;
                        comps = 3;
                    break;
                    case cgltf_attribute_type_tangent:
                        vdata.attrib = MS_VertKind_Tangent;
                        comps = 4;
                    break;
                    case cgltf_attribute_type_texcoord:
                        vdata.attrib = MS_VertKind_TexCoord_N;
                        comps = 2;
                    break;      
                    case cgltf_attribute_type_color:
                        vdata.attrib = MS_VertKind_Colour_N;
                        comps = 4;
                    break;
                    case cgltf_attribute_type_joints:
                        vdata.attrib = MS_VertKind_Joints_N;
                        is_float = false;
                        comps = 4;
                    break;
                    case cgltf_attribute_type_weights:
                        vdata.attrib = MS_VertKind_Weights_N;
                        comps = 4;
                    break;
                    case cgltf_attribute_type_custom:
                        vdata.attrib = MS_VertKind_Custom_N;
                        is_float = (acc->component_type == cgltf_component_type_r_32f) || acc->normalized;
                        vdata.name = dstr_from(prim->attributes[a].name, vert_alloc);
                    break;
                    default:
                        dlog_warn("invalid attribute type: %i", attr_type);
                        continue;
                    break;
                }
                if(!_read_attr(acc, vert_alloc, is_float, comps, &vdata)) {
                    dlog_warn("[MESH] failed to read attribute %i, skipping", attr_type);
                    continue;
                }
                if(attr_type == cgltf_attribute_type_weights) {
                    _norm_weights(vdata._floats, num_verts);
                }
                verts[loaded++] = vdata;
            }

            if(!_validate_layout(cmesh->name, layout, verts, loaded)) {
                dlog_error("Failed to validate mesh vertices");
                return;
            }

            // NOTE: i don't feel very good about directly uploading mesh data to the
            // GPU and then discarding it. i think i'd much prefer that this function
            // copied the vertex data into nicer structs and then gave them to the user
            // so that the user can do all the processing they need on the vertices
            // and use the vertices for collision and stuff.
            // but i'm too tired to design that split right now so i just wont.
            upload_mesh(cmesh, layout, verts, loaded, indices, vert_alloc);

            darena_clear(&vert_arena);            
            mesh_index++;
        }
    }
    darena_free(&vert_arena);
}
