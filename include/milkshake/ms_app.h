#pragma once
#include "derp.h"
#include "milkshake.h"
#include <SDL3/SDL_video.h>
#include <string.h>

// FIXME:
//   > currently i have a stupid little __ms_frame function that runs the
//     SDL_GL_SwapWindow function. this is dumb and annoying, i should instead
//     have something akin to sokol's sg_commit function that swaps the window
//     instead.
//     or give the user control of the window and make them swap themselves idk

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

static ms_app_desc app;
static ms_window __app_window;

ms_app_desc ms_main(int argc, char* argv[]);

// signals a shutdown event
static inline void ms_app_quit(void) {
  app.should_quit = true;
}

static void __ms_frame(void) {
  app.frame();
  SDL_GL_SwapWindow(__app_window.handle);
}

void ms_run(ms_app_desc app) {
  if(app.width  == 0) app.width  = 800;
  if(app.height == 0) app.height = 600;
  if(strlen(app.title) == 0) app.title = "milkshake app";
  __app_window = ms_init_window(app.width, app.height, app.title, app.window_flags);

  app.init();

  while(!app.should_quit) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
      if(app.handle_events) app.handle_events(e);

      if ( e.type == SDL_EVENT_QUIT ) app.should_quit = true;
    }
    ms_update();

    #if defined(MS_PLATFORM_WASM)
      emscripten_set_main_loop(__ms_frame(), 0, 1);
    #else
      __ms_frame();
    #endif
  }
  app.cleanup();
}

int main(int argc, char* argv[]) {
  app = ms_main(argc, argv);
  ms_run(app);
  return 0;
}
