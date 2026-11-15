#include "milkshake/milkshake.h"
#include <SDL3/SDL_stdinc.h>
#define MS_APP_IMPL
#include <milkshake/ms_app.h>
#include <milkshake/ms2d.h>
#include "../common.h"

#define SCR_W 128.0f
#define SCR_H 128.0f
#define WIN_W 800.0f
#define WIN_H 800.0f

struct {
  ms2d_renderbatch batch;
  ms_framebuffer fbo;
  ms_texture screen;
  ms_shader shader;
  ms_uniform u_proj, u_view;
  mat4s proj;

  bool wireframe;

  float t;
} state;

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

  state.fbo = ms_create_fbo();
  state.screen = ms_create_texture(SCR_W, SCR_H, GL_RGBA8, GL_RGBA, NULL, &PIXEL_SPRITE_SAMPLER);
  ms_fbo_attach_texture(state.fbo, state.screen, MS_ATTACHMENT_COLOR_N, 0, 0);
  if(!ms_fbo_is_complete(state.fbo)) {
    dlog_error("framebuffer incomplete: %i", glGetError());
    msapp_quit();
  }
}

void gameloop(void) {
  last_now = now, now = SDL_GetTicks();
  delta = (now - last_now) / 1000.0f;

  state.t += delta;

  int spd = 180;

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

  ms_bind_fbo(state.fbo);
  glViewport(0, 0, SCR_W, SCR_H);
  ms_clear_colour(MS_BLACK);
  glClear(GL_COLOR_BUFFER_BIT);
  state.proj = glms_ortho(0, SCR_W, SCR_H, 0, 0, 10);
  ms_bind_shader(state.shader);
  ms_shader_set_mat4(state.shader, state.u_proj, &state.proj, false);
  ms_shader_set_mat4(state.shader, state.u_view, &GLMS_MAT4_IDENTITY, false);

  // non-rotating
  ms2d_rect_outlinepro((ms2d_rectangle){SCR_W/5,       SCR_H/3, 6, 6}, MS2D_ORIGIN_CENTER, 0,  2, MS_RED);
  ms2d_rect_outlinepro((ms2d_rectangle){SCR_W/2,       SCR_H/3, 6, 6}, MS2D_ORIGIN_CENTER, 0, -2, MS_GREEN);
  ms2d_rect_outlinepro((ms2d_rectangle){SCR_W-SCR_W/5, SCR_H/3, 6, 6}, MS2D_ORIGIN_CENTER, 0,  2, MS_RED);
  ms2d_rect_outlinepro((ms2d_rectangle){SCR_W-SCR_W/5, SCR_H/3, 6, 6}, MS2D_ORIGIN_CENTER, 0, -2, MS_GREEN);

  // rotating
  ms2d_rect_outlinepro((ms2d_rectangle){SCR_W/5,       SCR_H-SCR_H/3, 6, 6}, MS2D_ORIGIN_CENTER, state.t * spd,  2, MS_RED);
  ms2d_rect_outlinepro((ms2d_rectangle){SCR_W/2,       SCR_H-SCR_H/3, 6, 6}, MS2D_ORIGIN_CENTER, state.t * spd, -2, MS_GREEN);
  ms2d_rect_outlinepro((ms2d_rectangle){SCR_W-SCR_W/5, SCR_H-SCR_H/3, 6, 6}, MS2D_ORIGIN_CENTER, state.t * spd,  2, MS_RED);
  ms2d_rect_outlinepro((ms2d_rectangle){SCR_W-SCR_W/5, SCR_H-SCR_H/3, 6, 6}, MS2D_ORIGIN_CENTER, state.t * spd, -2, MS_GREEN);

  ms2d_flush();

  // i really need to make this fbo business a lot less annoying
  ms_unbind_fbo();
  glViewport(0, 0, WIN_W, WIN_H);
  ms_clear_colour(MS_PINK);
  glClear(GL_COLOR_BUFFER_BIT);
    state.proj = glms_ortho(0, WIN_W, WIN_H, 0, 0, 10);
    ms_bind_shader(state.shader);
    ms_shader_set_mat4(state.shader, state.u_proj, &state.proj, false);
    ms_shader_set_mat4(state.shader, state.u_view, &GLMS_MAT4_IDENTITY, false);

    glDisable(GL_BLEND);
    ms2d_rectangle src = { 0, 0, SCR_W, SCR_H };
    ms2d_rectangle dest = { 0, 0, WIN_H, WIN_H };

    ms2d_texrectpro(state.screen, dest, src, MS2D_ORIGIN_TOP_LEFT, 0, MS_PURE_WHITE, false, true);
    glEnable(GL_BLEND);
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
