/* milkshake
 * 
 * Copyright (c) 2026 derpyzza
 * 
 * This file is released under the terms of the MIT license.
 * Read LICENSE for more information.
 *
 * no LLMS were used to write the code in this library.
*/ 

// ms_gpu.c:
// implements a simple wrapper over opengl functions
// the idea is to provide functions that wrap over common tedious opengl
// functionality, though you may directly use the proper opengl functions
// if you really want to.
//
// in the future i'd probably like to implement other backends to this as well
// mainly a software renderer and *maybe* vulkan? dunno.
// though i'd have to change up a lot of the api if i did that, as currently
// it's extremely opengl-coded.

#include "glad/glad.h"
#include "milkshake/milkshake.h"
#include "internal.h"

void ms_draw_elems(ms_vao vao, int prim_mode, int type, isize count, const int* start) {
	ms_bind_vao(vao);
	glDrawElements(prim_mode, count, type, start);
	G_core.stats.num_draw_calls++;
}

void ms_draw_arrays(ms_vao vao, int prim_mode, int index, int num_verts) {
	ms_bind_vao(vao);
	glDrawArrays(prim_mode, index, num_verts);
	G_core.stats.num_draw_calls++;
}

void ms_clear_colour(u32 hex) {
	vec4s col = ms_col_from_hex(hex);
	glClearColor(col.r, col.g, col.b, col.a);
}


ms_vao ms_create_vao(void) {
	ms_vao out;
	glGenVertexArrays(1, &out.id);
	return out;
}

void ms_destroy_vao(ms_vao vao) {
	glDeleteVertexArrays(1, &vao.id);
}

void ms_bind_vao(ms_vao vao) {
	if(G_core.gl_data.current_vao.id != vao.id) {
		glBindVertexArray(vao.id);
	}
}

isize _get_type_size(int type) {
	switch (type) {
		case GL_FLOAT:
			return sizeof(float);
		break;
		case GL_DOUBLE:
			return sizeof(double);
		break;

		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			return sizeof(short);
		break;

		case GL_INT:
		case GL_UNSIGNED_INT:
			return sizeof(int);
		break;

		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return sizeof(char);
		break;

		// yeah idk what to put here tbh
		default: return sizeof(float); break;
	}
}

ms_buffer ms_create_buffer(ms_buffertype type, ms_buffer_usage usage, isize size, const void * data) {
	ms_buffer out = {0};
	out.size = size;
	out.type = type;

	glGenBuffers(1, &out.id);
	glBindBuffer(type, out.id);
	glBufferData(type, size, data, usage);

	// unbind buffer, uhhhhh just because
	// glBindBuffer(type, 0);
	return out;
}

void ms_destroy_buffer(ms_buffer buffer) {
	glDeleteBuffers(1, &buffer.id);
}


void ms_vao_attach_vbo(ms_vao vao, ms_buffer buffer, ms_vertex_layout layout) {
	ms_bind_vao(vao);
	glBindBuffer(buffer.type, buffer.id);

	dforeach(ms_vertex_attrib, attr, layout.attribs, layout.num_attribs) {
		glEnableVertexAttribArray(attr->index);

		glVertexAttribPointer(
		  attr->index,
		  attr->size,
		  attr->type,
		  attr->normalized,
		  layout.stride,
		  (const void*) attr->offset
		);
	}
}

void ms_vao_attach_ebo(ms_vao vao, ms_buffer buffer) {
	ms_bind_vao(vao);
	glBindBuffer(buffer.type, buffer.id);
	glBindVertexArray(0);
}

// FRAMEBUFFERS {{

ms_framebuffer ms_create_fbo (void) {
	ms_framebuffer out = { 0 };
	glCreateFramebuffers(1, &out.id);
	return out;
}

void ms_destroy_fbo(ms_framebuffer fbo) {
	glDeleteFramebuffers(1, &fbo.id);
}

void ms_bind_fbo(ms_framebuffer fbo) {
	if(G_core.gl_data.current_fbo.id != fbo.id) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
	}
}

// binds 0 (the default screen)
void ms_unbind_fbo (void) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ms_fbo_attach_texture(ms_framebuffer fbo, ms_texture tex, enum ms_attachment slot, int num, int mip_level) {
	ms_bind_fbo(fbo);
	int _slot = slot == MS_ATTACHMENT_COLOR_N ? (int)slot+num : (int)slot;
  glFramebufferTexture(GL_FRAMEBUFFER, _slot, tex.id, mip_level);
}

void ms_fbo_attach_rbo(ms_framebuffer fbo, ms_renderbuffer rbo, enum ms_attachment slot, int num) {
	ms_bind_fbo(fbo);
	int _slot = slot == MS_ATTACHMENT_COLOR_N ? (int)slot+num : (int)slot;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, _slot, GL_RENDERBUFFER, rbo.id);
}

bool ms_fbo_is_complete(ms_framebuffer fbo) {
	ms_bind_fbo(fbo);
  u32 status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return status == GL_FRAMEBUFFER_COMPLETE;
}
// }} FRAMEBUFFERS

// RENDER BUFFERS {{

ms_renderbuffer ms_create_rbo (void) {
	ms_renderbuffer out = { 0 };
	glCreateRenderbuffers(1, &out.id);
	return out;
}

void ms_destroy_rbo(ms_renderbuffer rbo) {
	glDeleteRenderbuffers(1, &rbo.id);
}

// }} RENDER BUFFERS
