#include "internal.h"
#include "glad/glad.h"
#include <GL/gl.h>

void ms_cleanup(void) {
  for(int i = 0; i < MAX_SHADERS; i++) {
    glDeleteShader(shader_cache[i].value);
  }
}
