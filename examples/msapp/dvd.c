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
} state;

#define RECT_SIZE (32)
#define H_RECT_SIZE (int)((int)RECT_SIZE/2)

#define V_RECT_SIZE (vec2s){{RECT_SIZE, RECT_SIZE}}

#define COLOURS \
  COL(MS_RED, "MS_RED") \
  COL(MS_PINK, "MS_PINK") \
  COL(MS_PURPLE, "MS_PURPLE") \
  COL(MS_BLUE, "MS_BLUE") \
  COL(MS_CYAN, "MS_CYAN") \
  COL(MS_GREEN, "MS_GREEN") \
  COL(MS_YELLOW, "MS_YELLOW") \
  COL(MS_ORANGE, "MS_ORANGE") \
  COL(MS_WHITE, "MS_WHITE") \
  COL(MS_GREY, "MS_GREY") \
  \
  COL(MS_DARK_RED, "MS_DARK_RED") \
  COL(MS_DARK_PINK, "MS_DARK_PINK") \
  COL(MS_DARK_PURPLE, "MS_DARK_PURPLE") \
  COL(MS_DARK_BLUE, "MS_DARK_BLUE") \
  COL(MS_DARK_CYAN, "MS_DARK_CYAN") \
  COL(MS_DARK_GREEN, "MS_DARK_GREEN") \
  COL(MS_DARK_YELLOW, "MS_DARK_YELLOW") \
  COL(MS_DARK_ORANGE, "MS_DARK_ORANGE") \
  COL(MS_DARK_WHITE, "MS_DARK_WHITE") \
  COL(MS_DARK_GREY, "MS_DARK_GREY") \

#define NUM_COLOURS 20
int colours[] = {
  #define COL(v, s) v,
  COLOURS
  #undef COL
};

char* c_strings[] = {
  #define COL(v, s) s,
  COLOURS
  #undef COL
};


struct {
  vec2s pos;
  vec2s vel;
  f32 spd;
} rects[NUM_COLOURS];

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

  for(int i = 0; i < NUM_COLOURS; i++) {
    rects[i].pos.x = SDL_rand(200) + 300;
    rects[i].pos.y = SDL_rand(200) + 300;
    int vx = SDL_rand(2);
    int vy = SDL_rand(2);
    if(vx == 0) vx = -1;
    if(vy == 0) vy = -1;

    rects[i].vel.x = vx;
    rects[i].vel.y = vy;
    rects[i].spd = (SDL_randf() + 0.4) * 250;
  }
}

void gameloop(void) {
  last_now = now, now = SDL_GetTicks();
  delta = (now - last_now) / 1000.0f;

  if(ms_is_key_pressed(Key_Escape)) {
    msapp_quit();
  }

  for(int i = 0; i < NUM_COLOURS; i++) {
    rects[i].pos.x += rects[i].vel.x * rects[i].spd * delta;
    rects[i].pos.y += rects[i].vel.y * rects[i].spd * delta;

    if( (rects[i].pos.x - H_RECT_SIZE) < 0) {
      rects[i].pos.x = H_RECT_SIZE;
      rects[i].vel.x = 1;
    }
    else if(rects[i].pos.x + H_RECT_SIZE > WIN_W) {
      rects[i].pos.x = WIN_W - H_RECT_SIZE;
      rects[i].vel.x = -1;
    }

    if(rects[i].pos.y - H_RECT_SIZE < 0 ) {
      rects[i].pos.y = H_RECT_SIZE;
      rects[i].vel.y = 1;
    } else if (rects[i].pos.y + H_RECT_SIZE > WIN_H) {
      rects[i].pos.y = WIN_H - H_RECT_SIZE;
      rects[i].vel.y = -1;
    }
  }

  glViewport(0, 0, WIN_W, WIN_H);
  ms_clear_colour(MS_BLACK);
  glClear(GL_COLOR_BUFFER_BIT);
  ms_bind_shader(state.shader);
  ms_shader_set_mat4(state.shader, state.u_proj, &state.proj, false);
  ms_shader_set_mat4(state.shader, state.u_view, &GLMS_MAT4_IDENTITY, false);

  for(int i = 0; i < NUM_COLOURS; i++) {
    ms2d_rect(rects[i].pos.x, rects[i].pos.y, RECT_SIZE, RECT_SIZE, colours[i]);
  }

  ms2d_flush();

  fps_timer += delta * 1000;
  if(fps_timer >= 1000) {
    // dlog_debug("num drawcalls: %i", ms_num_drawcalls());
    char t[64];
    sprintf(t, "DVD example :: num drawcalls: %i", ms_num_drawcalls());
    msapp_window_set_title(t);
    fps_timer = 0;
  }
}

void cleanup(void) {
  // ms2d_destroy_renderbatch(&state.batch);
}

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
