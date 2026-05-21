#include "cglm/types-struct.h"
#include "glad/glad.h"
#include "internal.h"

#include <GL/gl.h>
#include <SDL3/SDL_events.h>
#include <libderp/derp.h>

ms_uniform ms_get_uniform(ms_shader shader, const char* name) {
  ms_uniform out = {0};
  out.id = glGetUniformLocation(shader.id, name);
  return out;
}

int _compile_shader(u32 * shader, GLint shader_type, dstr source) {
  *shader = glCreateShader(shader_type);
    
	i32 success;
	char infoLog[512];

	glShaderSource(*shader, 1, (const char* const*)&source.cptr, (int*)&source.len);
	glCompileShader(*shader);

	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(*shader, 512, NULL, infoLog);
		dlog_error("Vert shader [%.*s] comp failed:\n %s", dstr_fmt(source), infoLog);
	}

	return success;
}

ms_shader ms_create_shader_from_source(
  dstr vert_source,
  dstr frag_source ) {
	bool had_error = false;
	i32 success;
	char infoLog[512];

	u32 v_shader;
	u32 f_shader;

	_compile_shader(&v_shader, GL_VERTEX_SHADER, vert_source);
	_compile_shader(&f_shader, GL_FRAGMENT_SHADER, frag_source);
  
	ms_shader shader = {0};
	shader.id = glCreateProgram();

	glAttachShader(shader.id, v_shader);
	glAttachShader(shader.id, f_shader);

	glLinkProgram(shader.id);

	glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
	if(!success) {
    glGetProgramInfoLog(shader.id, 512, NULL, infoLog);
		dlog_error("Shader program comp failed: %s", infoLog);
		had_error = true;
	}

	if(had_error) exit(-1);

	return shader;
}


// reads shader code from provided file paths and compiles and links the shader program.
ms_shader ms_create_shader(
  const char * vert_file,
  const char * frag_file
  ) {

	dlog_debug("compiling shader\n\t> vert: [%s]\n\t> frag: [%s]", vert_file, frag_file);

	dstr vert_source = dfile_read((char*)vert_file);
	if (!vert_source.cptr) {
		dlog_error("vertex shader file [%.*s] invalid", dstr_fmt(vert_source));
	}
	dstr frag_source = dfile_read((char*)frag_file);
	if (!frag_source.cptr) {
		dlog_error("vertex shader file [%.*s] invalid", dstr_fmt(frag_source));
	}

	ms_shader prog = ms_create_shader_from_source(vert_source, frag_source);
	dlog_debug("compiled shader from {%s, %s}", vert_file, frag_file);

	return prog;
}

void ms_bind_shader( ms_shader shader ) {
  // if( __current_shader != shader ) {
    glUseProgram(shader.id);
  // }
}

void ms_shader_set_value  (ms_shader shader, int loc, const void * data, enum ms_uniform_type val_type) {
  ms_shader_set_value_v(shader, loc, data, 1, val_type);
}

void ms_shader_set_value_v(ms_shader shader, int loc, const void * data, isize count, enum ms_uniform_type val_type) {
  switch(val_type) {
    case MS_Uniform_Float:  glUniform1fv(loc, count, (float*)data); break;
    case MS_Uniform_Float2: glUniform2fv(loc, count, (float*)data); break;
    case MS_Uniform_Float3: glUniform3fv(loc, count, (float*)data); break;
    case MS_Uniform_Float4: glUniform4fv(loc, count, (float*)data); break;

    case MS_Uniform_Int:  glUniform1iv(loc, count, (int*)data); break;
    case MS_Uniform_Int2: glUniform2iv(loc, count, (int*)data); break;
    case MS_Uniform_Int3: glUniform3iv(loc, count, (int*)data); break;
    case MS_Uniform_Int4: glUniform4iv(loc, count, (int*)data); break;

    case MS_Uniform_Uint:  glUniform1uiv(loc, count, (uint*)data); break;
    case MS_Uniform_Uint2: glUniform2uiv(loc, count, (uint*)data); break;
    case MS_Uniform_Uint3: glUniform3uiv(loc, count, (uint*)data); break;
    case MS_Uniform_Uint4: glUniform4uiv(loc, count, (uint*)data); break;

    case MS_Uniform_Sampler2D: glUniform1iv(loc, count, (int*)data); break;
    default: {
      dlog_error("unknown uniform type %i for shader [%i]", val_type, shader.id );
      break;
    }
    case MS_UNIFORM_COUNT: break;
  }
}

void ms_shader_set_mat4(ms_shader shader, ms_uniform unif, mat4s *mat, bool transpose) {
  ms_shader_set_mat4_v(shader, unif, mat, 1, transpose);
}

void ms_shader_set_mat4_v(ms_shader shader, ms_uniform unif, mat4s * mat, isize count, bool transpose) {
  glUniformMatrix4fv(unif.id, count, transpose, (float*)mat->raw);
}
