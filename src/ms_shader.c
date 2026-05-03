#include "cglm/types-struct.h"
#include "internal.h"
#include "glad/glad.h"

#include <GL/gl.h>
#include <libderp/derp.h>
#include <string.h>


int _compile_shader(u32 * shader, GLint shader_type, const char* source) {
  for(int i = 0; i < MAX_SHADERS; i++ ) {
    if( !strcmp(source, shader_cache[i].name) ) {
      *shader = shader_cache[i].value;
      return 1;
    }
  }

  *shader = glCreateShader(shader_type);
  isize _slen = strlen(source);
  isize len = (_slen > 128) ? 128 : _slen;
  memcpy(shader_cache[shader_cache_index].name, source, len);
  shader_cache[shader_cache_index].value = *shader;
  shader_cache_index++;
  
	dstr src_file = dfile_read((char*)source);
	if (!src_file.cptr) {
		dlog_error("vertex shader file [%s] invalid", source);
	}

	i32 success;
	char infoLog[512];

	glShaderSource(*shader, 1, (const char* const*)&src_file.cptr, (int*)&src_file.len);
	glCompileShader(*shader);

	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(*shader, 512, NULL, infoLog);
		dlog_error("Vert shader [%s] comp failed:\n %s", source, infoLog);
	}

	d_free(src_file.cptr);
	return success;
}


// reads shader code from provided file paths and compiles and links the shader program.
u32 ms_create_shader(
  const char * vert_source,
  const char * frag_source
  ) {

	dlog_debug("compiling shader\n\t> vert: [%s]\n\t> frag: [%s]", vert_source, frag_source);

	bool had_error = false;
	i32 success;
	char infoLog[512];

	u32 v_shader;
	u32 f_shader;

	_compile_shader(&v_shader, GL_VERTEX_SHADER, vert_source);
	_compile_shader(&f_shader, GL_FRAGMENT_SHADER, frag_source);

	u32 shader_program = 0;
	shader_program = glCreateProgram();

	glAttachShader(shader_program, v_shader);
	glAttachShader(shader_program, f_shader);

	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if(!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
		dlog_error("Shader program comp failed: %s", infoLog);
		had_error = true;
	}

	// glDeleteShader(v_shader);
	// glDeleteShader(f_shader);

	if(had_error) exit(-1);

	dlog_debug("compiled shader [%i] from {%s, %s}", shader_program, vert_source, frag_source);

	return shader_program;
}

void ms_bind_shader( ms_handle * shader ) {
  // if( __current_shader != shader ) {
    glUseProgram(((ms_shader*)shader)->id);
  // }
}

void ms_shader_set_value  (ms_handle * shader, int loc, const void * data, enum ms_uniform_type val_type) {
  ms_shader_set_value_v(shader, loc, data, 1, val_type);
}

void ms_shader_set_value_v(ms_handle * shader, int loc, const void * data, isize count, enum ms_uniform_type val_type) {
  switch(val_type) {
    case MS_UNIFORM_FLOAT:  glUniform1fv(loc, count, (float*)data); break;
    case MS_UNIFORM_FLOAT2: glUniform2fv(loc, count, (float*)data); break;
    case MS_UNIFORM_FLOAT3: glUniform3fv(loc, count, (float*)data); break;
    case MS_UNIFORM_FLOAT4: glUniform4fv(loc, count, (float*)data); break;

    case MS_UNIFORM_INT:  glUniform1iv(loc, count, (int*)data); break;
    case MS_UNIFORM_INT2: glUniform2iv(loc, count, (int*)data); break;
    case MS_UNIFORM_INT3: glUniform3iv(loc, count, (int*)data); break;
    case MS_UNIFORM_INT4: glUniform4iv(loc, count, (int*)data); break;

    case MS_UNIFORM_UINT:  glUniform1uiv(loc, count, (uint*)data); break;
    case MS_UNIFORM_UINT2: glUniform2uiv(loc, count, (uint*)data); break;
    case MS_UNIFORM_UINT3: glUniform3uiv(loc, count, (uint*)data); break;
    case MS_UNIFORM_UINT4: glUniform4uiv(loc, count, (uint*)data); break;

    case MS_UNIFORM_SAMPLER2D: glUniform1iv(loc, count, (int*)data); break;
    default: dlog_error("unknown uniform type %i for shader [%s]", val_type, ((ms_shader*)shader)->name );
    case MS_UNIFORM_COUNT: break;
  }
}

void ms_shader_set_matrix  (ms_handle* shader, int loc, mat4s * mat, bool transpose) {
  ms_shader_set_matrix_v(shader, loc, mat, 1, transpose);
}

void ms_shader_set_matrix_v(ms_handle* shader, int loc, mat4s * mat, isize count, bool transpose) {
  glUniformMatrix4fv(loc, count, transpose, (float*)mat->raw);
}
