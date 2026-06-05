#include <SDL3/SDL_scancode.h>
#include <milkshake/milkshake.h>

const static dstr vert = dstr(
"#version 330 core\n"
"layout (location=0) in vec3 a_pos;\n"
"layout (location=1) in vec4 a_col;\n"
"out vec4 v_col;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 proj;\n"
"void main(){\n"
" gl_Position = proj * view * model * vec4(a_pos, 1.0);\n"
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
  // pos               // colour
  -0.5f, -0.5f,  0.5f, 1, 0, 0, 1,
   0.5f, -0.5f,  0.5f, 0, 1, 0, 1,
   0.5f,  0.5f,  0.5f, 0, 0, 1, 1,
  -0.5f,  0.5f,  0.5f, 0, 0, 0, 1,

   0.5f, -0.5f, -0.5f, 1, 0, 0, 1,
  -0.5f, -0.5f, -0.5f, 1, 0, 0, 1,
  -0.5f,  0.5f, -0.5f, 1, 0, 0, 1,
   0.5f,  0.5f, -0.5f, 1, 0, 0, 1,

  -0.5f, -0.5f, -0.5f, 0, 1, 0, 1,
  -0.5f, -0.5f,  0.5f, 0, 1, 0, 1,
  -0.5f,  0.5f,  0.5f, 0, 1, 0, 1,
  -0.5f,  0.5f, -0.5f, 0, 1, 0, 1,

   0.5f, -0.5f,  0.5f, 0, 0, 1, 1,
   0.5f, -0.5f, -0.5f, 0, 0, 1, 1,
   0.5f,  0.5f, -0.5f, 0, 0, 1, 1,
   0.5f,  0.5f,  0.5f, 0, 0, 1, 1,

  -0.5f,  0.5f,  0.5f, 1, 1, 0, 1,
   0.5f,  0.5f,  0.5f, 1, 1, 0, 1,
   0.5f,  0.5f, -0.5f, 1, 1, 0, 1,
  -0.5f,  0.5f, -0.5f, 1, 1, 0, 1,

  -0.5f, -0.5f, -0.5f, 0, 1, 1, 1,
   0.5f, -0.5f, -0.5f, 0, 1, 1, 1,
   0.5f, -0.5f,  0.5f, 0, 1, 1, 1,
  -0.5f, -0.5f,  0.5f,  0, 1, 1, 1
};

static const uint indices[] = {
  0, 1, 2,  2, 3, 0,
  4, 5, 6,  6, 7, 4,
  8, 9, 10,  10, 11, 8,
  12, 13, 14,  14, 15, 12,
  16, 17, 18,  18, 19, 16,
  20, 21, 22,  22, 23, 20
};


int
main(void) {
  ms_init_window(800, 800, "WINDOW", MS_WindowFlag_Resizable);
  bool quit = false;

  ms_shader shader = ms_create_shader_from_source(vert, frag);
  ms_uniform model_loc = ms_get_uniform(shader, "model");
  ms_uniform proj_loc =  ms_get_uniform(shader, "proj");
  ms_uniform view_loc =  ms_get_uniform(shader, "view");

  ms_vao cube = ms_create_vao();
  ms_buffer vert_buf =
    ms_create_buffer(
      MS_BufferType_Vertex
      , MS_BufferUsage_StaticDraw
      , sizeof(verts)
      , verts
    );
  ms_buffer index_buf =
    ms_create_buffer(
      MS_BufferType_Index
      , MS_BufferUsage_StaticDraw
      , sizeof(indices)
      , indices
    );
  ms_vao_attach_vbo(&cube, vert_buf, MS_VERTLAYOUT_POSCOL);  
  ms_vao_attach_ebo(&cube, index_buf);

  mat4s model = GLMS_MAT4_IDENTITY
      , view = glms_lookat((vec3s){{0, 0, -5}}, (vec3s){{0, 0, -1}}, (vec3s){{0, 1, 0}})
      , proj = glms_perspective(45, 1, 0.1, 10.0);

  glEnable(GL_DEPTH_TEST);

  while(!quit) {
    ms_update();
    if(ms_is_key_pressed(Key_Escape)) {
      quit = true;
    }

    model = glms_rotate(model, glm_rad(0.1), (vec3s){{0.4, 0.5, .7}});

    ms_clear_colour(0x98a96dFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      ms_bind_shader(shader);
      ms_shader_set_mat4(shader, view_loc,  &view, false);
      ms_shader_set_mat4(shader, proj_loc,  &proj, false);
      ms_shader_set_mat4(shader, model_loc, &model, false);

      glBindVertexArray(cube.id);
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    ms_end_drawing();
  }
}
