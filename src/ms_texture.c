/* milkshake
 * 
 * Copyright (c) 2026 derpyzza
 * 
 * This file is released under the terms of the MIT license.
 * Read LICENSE for more information.
 *
 * no LLMS were used to write the code in this library.
*/ 

#include <glad/glad.h>
#include <GL/gl.h>
#include <libderp/derp.h>
#include <milkshake/milkshake.h>
#include "internal.h"
#include "stb_image.h"
#include <math.h>

void ms_destroy_texture(ms_texture tex) {
	glDeleteTextures(1, &tex.id);
}

void ms_bind_texture(ms_texture texture) {
	if(G_core.gl_data.current_texture.id != texture.id) {
		glBindTexture(GL_TEXTURE_2D, texture.id);
	}
}

void ms_bind_texture_slot(ms_texture texture, int slot) {
	dassert(slot > -1, "texture slot must not be negative")
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture.id);
}

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
  int w,
  int h,
  int infmt,
  int outfmt,
  const void* data,
  const ms_sampler * sampler
) {
	ms_texture out = {
		.width = w,
		.height = h,
	};
	
	glGenTextures(1, &out.id);
	glBindTexture(GL_TEXTURE_2D, out.id);

	if (sampler) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler->wrap_s);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler->wrap_t);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler->min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler->max_filter);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	int levels = 1 + floor(log2(fmax(out.width, out.height)));

	glTexImage2D(
	  GL_TEXTURE_2D,
	  0,
	  infmt,
	  out.width,
	  out.height,
	  0,
	  outfmt,
	  GL_UNSIGNED_BYTE,
	  data
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	return out;
}

ms_texture ms_load_texture( const char* path, const ms_sampler * sampler ) {
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
ms_load_texture_from_memory( const u8* data, int data_len, const ms_sampler * sampler ) {
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
