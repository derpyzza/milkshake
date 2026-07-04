#define MS_APP_IMPL
#include <milkshake/ms_app.h>
#define WIN_W 800
#define WIN_H 600

const static dstr vert = dstr(
"#version 330 core\n"
"layout (location=0) in vec3 a_pos;\n"
"layout (location=1) in vec4 a_col;\n"
"out vec4 v_col;\n"
"void main(){\n"
" gl_Position = vec4(a_pos, 1.0);\n"
" v_col = a_col;\n"
"}\n"
);

const static dstr frag = dstr(
"#version 330 core\n"
"out vec4 FragColour;\n"
"in vec4 v_col;\n"
"void main(){\n"
"FragColour = v_col;\n"
"}\n"
);

struct {
  ms_vao vao;
  ms_buffer vbo;
  ms_shader shader;
} state;

void init(void) {
  const float verts[] = {
     -0.5f, -0.5f, 0.0f, 0, 0, 0, 1,
      0.5f, -0.5f, 0.0f, 1, 0, 0, 1,
      0.0f,  0.5f, 0.0f, 0, 1, 0, 1,
  };

  state.shader = ms_create_shader_from_source(vert, frag);
  state.vao = ms_create_vao();
  state.vbo = ms_create_buffer(MS_BufferType_Vertex , MS_BufferUsage_StaticDraw , sizeof(verts) , verts);
  ms_vao_attach_vbo(state.vao, state.vbo, MS_VERTLAYOUT_POSCOL);  
}

void gameloop(void) {
  if(ms_is_key_pressed(Key_Escape)) {
    ms_app_quit();
  }

  ms_clear_colour(0x98a96dFF);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ms_bind_shader(state.shader);
  ms_draw_arrays(state.vao, MS_PrimMode_Triangle, 0, 3);
}

void cleanup(void) {
  ms_destroy_vao(state.vao);
  ms_destroy_buffer(state.vbo);
}

ms_app_desc ms_main(int argc, char **argv) {
  return (ms_app_desc) {
    .frame = gameloop,
    .init = init,
    .cleanup = cleanup,
    .width = WIN_W,
    .height = WIN_H,
    .title = "triangle app"
  };  
}
