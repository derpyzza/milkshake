#pragma once
#include <libderp/derp.h>
#include "milkshake.h"
#include <string.h>

#if defined(MS_PLATFORM_WASM)
  #include <emscripten.h>
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

static ms_app_desc * app;
static ms_window __app_window;

ms_app_desc ms_main(int argc, char* argv[]);

// signals a shutdown event
static inline void ms_app_quit(void) {
  app->should_quit = true;
}

#if defined(MS_APP_IMPL)

static void __ms_frame(void) {
  app->frame();
  SDL_GL_SwapWindow(__app_window.handle);
}

void ms_run(ms_app_desc *app) {
  if(app->width  == 0) app->width  = 800;
  if(app->height == 0) app->height = 600;
  if(strlen(app->title) == 0) app->title = "milkshake app";
  __app_window = ms_init_window(app->width, app->height, app->title, app->window_flags);
  if(app->handle_events) ms_set_event_callback(app->handle_events);

  if(app->init) app->init();

  while(!app->should_quit) {
    ms_update();
    if(ms_should_quit()) ms_app_quit();

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
