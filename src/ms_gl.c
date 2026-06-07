#include "milkshake/milkshake.h"
#include "internal.h"

void ms_draw_elems(ms_vao vao, int prim_mode, int type, isize count, const int* start) {
	glBindVertexArray(vao.id);
	glDrawElements(prim_mode, count, type, start);
	G_core.stats.num_draw_calls++;
}

void ms_draw_arrays(ms_vao vao, int prim_mode, int index, int num_verts) {
	glBindVertexArray(vao.id);
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

const inline isize _get_type_size(int type) {
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
	glBindBuffer(type, 0);
	return out;
}

void ms_destroy_buffer(ms_buffer buffer) {
	glDeleteBuffers(1, &buffer.id);
}


void ms_vao_attach_vbo(ms_vao *vao, ms_buffer buffer, ms_vertex_layout layout) {
	glBindVertexArray(vao->id);
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

void ms_vao_attach_ebo(ms_vao *vao, ms_buffer buffer) {
	glBindVertexArray(vao->id);
	glBindBuffer(buffer.type, buffer.id);
}

