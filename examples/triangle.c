#include <milkshake/milkshake.h>

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

static const float verts[] = {
   -0.5f, -0.5f, 0.0f, 1, 0, 0, 1,
    0.5f, -0.5f, 0.0f, 0, 1, 0, 1,
    0.0f,  0.5f, 0.0f, 0, 0, 1, 1,
};

int
main(void) {
  ms_window window = ms_init_window(800, 800, "WINDOW", MS_WindowFlag_Resizable);
  bool quit = false;

  ms_shader shader = ms_create_shader_from_source(vert, frag);
  ms_vao tri = ms_create_vao();
  ms_buffer vert_buf = ms_create_buffer(MS_BufferType_Vertex , MS_BufferUsage_StaticDraw , sizeof(verts) , verts);
  ms_vao_attach_vbo(&tri, vert_buf, MS_VERTLAYOUT_POSCOL);  

  while(!quit) {
    // inputs internal input list and polls for events
    ms_update();

    if(ms_is_key_pressed(Key_Escape)) {
      quit = true;
    }

    ms_clear_colour(0x98a96dFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      ms_bind_shader(shader);
      ms_draw_arrays(tri, MS_PrimMode_Triangle, 0, 3);
    SDL_GL_SwapWindow(window.handle);
  }
}
