#pragma once
#include <SDL3/SDL_video.h>
#include <libderp/derp.h>
#include "milkshake.h"
#include <string.h>

#if defined(MS_PLATFORM_WASM)
  #include <emscripten.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct {
  fnptr(void, init, void);
  fnptr(void, frame, void);
  fnptr(void, handle_events, SDL_Event);
  fnptr(void, cleanup, void);

  int   width;
  int   height;
  char* title;
  int   window_flags;

  bool should_quit;
} ms_app_desc;

ms_app_desc ms_main(int argc, char* argv[]);
// signals a shutdown event
void msapp_quit(void);

vec2s msapp_window_size(void);
f32 msapp_window_w(void);
f32 msapp_window_h(void);
char* msapp_window_get_title(void);
void msapp_window_set_title(char* title);

#if defined(MS_APP_IMPL)

static ms_app_desc * app;
static ms_window __app_window;

vec2s msapp_window_size(void) {
  return (vec2s){{ __app_window.width, __app_window.height }};
}

f32 msapp_window_w(void) {
  return __app_window.width;
}

f32 msapp_window_h(void) {
  return __app_window.height;
}

char* msapp_window_get_title(void) {
  return __app_window.title;
}

void msapp_window_set_title(char* title) {
  __app_window.title = title;
  SDL_SetWindowTitle(__app_window.handle, title);
}

void msapp_quit(void) {
  app->should_quit = true;
}

void __ms_frame(void) {
  app->frame();
  SDL_GL_SwapWindow(__app_window.handle);
}

void ms_run(ms_app_desc *app) {
  if(app->width  == 0) app->width  = 800;
  if(app->height == 0) app->height = 600;
  if(app->title == NULL) app->title = "milkshake app";
  __app_window = ms_init_window(app->width, app->height, app->title, app->window_flags);
  if(app->handle_events) ms_set_event_callback(app->handle_events);

  if(app->init) app->init();

  while(!app->should_quit) {
    ms_update();
    if(ms_should_quit()) msapp_quit();

    #if defined(MS_PLATFORM_WASM)
      emscripten_set_main_loop(__ms_frame(), 0, 1);
    #else
      __ms_frame();
    #endif
  }
  if(app->cleanup) app->cleanup();
}

int main(int argc, char* argv[]) {
  // idk if this is necessary but i'd like to do it anyway to zero initialize the struct's fields
  ms_app_desc a = {0};
  a = ms_main(argc, argv);
  app = &a;
  ms_run(app);
  return 0;
}

#endif

#ifdef __cplusplus
	}
#endif
