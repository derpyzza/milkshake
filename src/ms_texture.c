#include "internal.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <GL/gl.h>
#include <math.h>

// FIXME:
// instead of the texture functions returning a new texture, they should instead
// be requesting a texture from the library and then returning a handle to that
// texture

void image_get_fmt(int format, int *infmt, int* outfmt) {
	switch (format) {
		case 1: { *infmt = GL_R8;    *outfmt = GL_RED;  } break;
		case 2: { *infmt = GL_RG8;   *outfmt = GL_RG;   } break;
		case 3: { *infmt = GL_RGB8;  *outfmt = GL_RGB;  } break;
		case 4: { *infmt = GL_RGBA8; *outfmt = GL_RGBA; } break;
		default:
			dlog_error("Texture Format not supported!!");
		break;
	}
}

ms_texture
ms_create_texture(
  int h,
  int w,
  int infmt,
  int outfmt,
  const void* data, ms_sampler * sampler
) {
	ms_texture out = {
		.width = w,
		.height = h,
	};
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCreateTextures(GL_TEXTURE_2D, 1, (GLuint*)&out.id);

	if (sampler) {
		glTextureParameteri(out.id, GL_TEXTURE_WRAP_S, sampler->wrap_s);	
		glTextureParameteri(out.id, GL_TEXTURE_WRAP_T, sampler->wrap_t);
		glTextureParameteri(out.id, GL_TEXTURE_MIN_FILTER, sampler->min_filter);
		glTextureParameteri(out.id, GL_TEXTURE_MAG_FILTER, sampler->max_filter);
	} else {
		glTextureParameteri(out.id, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTextureParameteri(out.id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(out.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTextureParameteri(out.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	int levels = 1 + floor(log2(fmax(out.width, out.height)));

	glTextureStorage2D(
	  out.id,
	  levels,
	  infmt,
	  out.width,
	  out.height
	);
  glTextureSubImage2D(
    out.id,             // Texture handle
    0,               // Mipmap level
    0, 0,            // Offset (x, y)
    out.width,       // Width
    out.height,      // Height
    outfmt,         // Pixel format
    GL_UNSIGNED_BYTE,// Data type
    data             // Pointer to pixels
  );
	glGenerateTextureMipmap(out.id);

	return out;
}

ms_texture ms_load_texture( const char* path, ms_sampler * sampler ) {
	dassert(path, "filepath should not be null");

	ms_texture out = { 0 };
	dstr file = dfile_read((char *)path);

	out = ms_load_texture_from_memory((const u8*)file.cptr, file.len, sampler);
	isize size = strlen(path);
	size = size > MS_NAME_LEN ? MS_NAME_LEN : size;
	memcpy(out.path, path, size);

	d_free(file.cptr);
	return out;
}

ms_texture
ms_load_texture_from_memory( const u8* data, int data_len, ms_sampler * sampler ) {
	dassert( data, "input pixel data should not be null" );

	ms_texture out = { 0 };

	u8* out_data = stbi_load_from_memory(
	  (const u8*) data
	  , data_len
	  , &out.width
	  , &out.height
	  , &out.nrChannel
	  , 0
	);

	if (!out_data) {
		dlog_error("Could not load image");
		return out;
	}

	dlog_info(
		"[Texture] Loaded image, of size (%i, %i), and channels %i"
		, out.width
		, out.height
		, out.nrChannel
	);

	int infmt = 0, fmt = 0;
	image_get_fmt(out.nrChannel, &infmt, &fmt);

	out = ms_create_texture(out.width, out.height, infmt, fmt, out_data, sampler);

	stbi_image_free(out_data);

	return out;
}
