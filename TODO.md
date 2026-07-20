opengl render loop thingy:

- create window
- create rendering context
- [create any rendering surfaces you may need]
- load and prime shaders
- create buffer, element and array objects
- start draw loop
- [bind rendering surface]
- clear background
- bind shader
- [bind uniforms]
- [bind textures]
- bind array object
- draw vertices
- switch backbuffers
- goto start of loop

important features i need to add support for:
	-	text rendering ( some sort of simple system that supports bitmap fonts, ttf fonts, and sdf-based rendering )
	- sound handling?


i'll also need some sort of callback system for resource acquiring and handling.
particularly:
- a callback for loading textures ( mainly so the user can cache textures if they wish, instead of loading in textures directly from the gltf model, per model )
- a callback for loading materials ( so that the user can have their own custom material formats without the loader needing to know about them )
one crucial detail for both of these however is that if the user doesn't pass in a
callback, it'll default to just directly loading in the information from the gltf file
as usual. however, it'll need some mechanism to store that data directly on the model.

i'd probably just put a couple void* arrays in there ig
or a void* array for the materials and a ms_texture array.
idk

``` c

struct VertexData {
	// all these arrays are of size N, where N is the number of
	// position attributes in the model.

	// for fields with the suffix _v, their sizes are actually
	// N*R, where R is the accompanying num_[fieldname] number

  int num_vertices;

	vec3* positions;
	vec3* normals;
	vec3* tangents;

	int num_colours;
	vec4** colours_v;

	int num_texcoords;
	vec2** texcoords_v;

	int num_joints;
	ivec4** joints_v;

	int num_weights;
	vec4** weights_v;

	int num_custom_attributes;
	void** custom_v;
	// these could be wrapped up in a struct
	int * custom_sizes;
}


ms_error
ms_load_gltf(ms_model * m, ms_vertex_layout l, const char* path) {
	...
	load_vertex_data(ctx, l);
}

void
load_vertex_data(ctx, l) {
	for mesh in ctx.m.meshes {
		for prim in mesh.primitives {
			...
			VertexData * vert_data = malloc(...);
			for attr in prim.attributes {
				switch attr.type {
					case position: {...} break;
					...
				}
			}

			int stride, offset;
			for attr in l.attributes {
				stride += attr.stride;
				offset += attr.offset;
			}

			void* buffer = alloc(num_verts * vert_stride);
			char* dst = (char*)buffer;
			for v in vertices {
				for attr in l.attributes {
					int size = get_size(attr);
					memcpy(dst, get_attr(vert_data, attr), size);
					dest += size;
				}
			}
		}
	}
}
```
