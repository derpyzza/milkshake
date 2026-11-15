#include "milkshake/milkshake.h"
#include <SDL3/SDL_stdinc.h>
#define MS_APP_IMPL
#include <milkshake/ms_app.h>
#include <milkshake/ms2d.h>

#define WIN_W 800
#define WIN_H 800

struct {
  ms2d_renderbatch batch;
  ms_shader shader;
  ms_uniform u_proj, u_view;
  mat4s proj;

  bool wireframe;

  float t;
} state;

#define RECT_SIZE (32)
#define H_RECT_SIZE (int)((int)RECT_SIZE/2)

#define V_RECT_SIZE (vec2s){{RECT_SIZE, RECT_SIZE}}

u64 now = 0, last_now = 0;
u64 fps_timer = 0;
float delta = 0;

void init(void) {
  state.shader = ms2d_sprite_shader();
  state.u_proj = ms_get_uniform(state.shader, "proj");
  state.u_view = ms_get_uniform(state.shader, "view");

  state.proj = glms_ortho(0, WIN_W, WIN_H, 0, 0, 10);

  state.batch = ms2d_new_renderbatch();
  ms2d_bind_renderbatch(&state.batch);
}

void gameloop(void) {
  last_now = now, now = SDL_GetTicks();
  delta = (now - last_now) / 1000.0f;

  state.t += delta;

  int spd = 120;

  if(ms_is_key_pressed(Key_Escape)) {
    msapp_quit();
  }

  if(ms_is_key_pressed(Key_W)) {
    state.wireframe = !state.wireframe;
  }

  if(state.wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  glViewport(0, 0, WIN_W, WIN_H);
  ms_clear_colour(MS_BLACK);
  glClear(GL_COLOR_BUFFER_BIT);
  ms_bind_shader(state.shader);
  ms_shader_set_mat4(state.shader, state.u_proj, &state.proj, false);
  ms_shader_set_mat4(state.shader, state.u_view, &GLMS_MAT4_IDENTITY, false);

  // lowk cursed af, but whatever
  // the idea is that i want rectangles on the screen at the following coordinates:
  // (1/5, 1/5) (3/5, 1/5) (1, 1/5)
  // (1/5, 3/5) (3/5, 3/5) (1, 3/5)
  // (1/5,   1) (3/5,   1) (1,   1)
  // times (window_w, window_h)
  //
  // along with each row having a different draw mode in the order: QUADS, TRIS, LINES.
  // as well as each column having one single colour in the order: RED, BLUE, GREEN
  //
  // the nums array exists because i couldn't come up with an equation that'd give me those numbers in that order :(
  u32 cols[3] = { MS_RED, MS_BLUE, MS_GREEN };
  f32 nums[3] = { 1, 3, 5 };
  for(int mode = 0; mode < 3; mode++) {
    ms2d_draw_mode(abs(mode-3)-1);
    for(int s = 0; s < 3; s++) {
      ms2d_rectr((WIN_W-100) * nums[s]/5 - 25, (WIN_H-100)*(nums[mode]/5) - 25, RECT_SIZE, RECT_SIZE, state.t * spd, cols[s]-3);
    }
  }

  ms2d_flush();

  fps_timer += delta * 1000;
  if(fps_timer >= 1000) {
    char t[64];
    sprintf(t, "DVD example :: num drawcalls: %i", ms_num_drawcalls());
    msapp_window_set_title(t);
    fps_timer = 0;
  }
}

void cleanup(void) {}

ms_app_desc ms_main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return (ms_app_desc) {
    .frame = gameloop,
    .init = init,
    .cleanup = cleanup,
    .width = WIN_W,
    .height = WIN_H,
    .title = "triangle app"
  };  
}
