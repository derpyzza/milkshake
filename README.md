
milkshake — tiny gamedev framework.

>[!Note]
> this library is currently under development and everything you see here is subject to change
>
> i haven't fully decided on what i want the final library to work like
> so the library is currently going through a heavy experimental phase where i try out ideas
> and potentially discard them

this is an attempt to take some scattered engines and games i've made with opengl and
combine them all into one handy little library so i don't have to continually
keep copy pasting the same initialization code over and over again per project.

calling raw opengl functions while using this library is expected and encouraged, as the library doesn't wrap over a lot of opengl functionality.

currently this is targetting opengl 3.0 as the main rendering backend.
however, i'd like to target a handful of extra backends in the future, namely:
- GLES for running on android
- webgl2 for the web
- Software renderer for fun :)

do note though that the library won't wrap over any limitations of any of those
APIs, and you as the user are expected to know the shortcomings of the opengl versions
you're using and structure your program accordingly. the library will do the barest
minimum by making sure the correct functions are being used internally for the chosen
gl versions, and that's it.

contents:
- milkshake.h:
  the necessary basics ( gl + sdl wrapper )
  - window and gl context creation
  - timer functionality
  - input management
  - viewport and render-to-texture functionality
  - shader loading and compilation
  - vertex layout structs

- app.h:
  tiny application wrapper similar to sokol_app.h that simply wraps over
  the main game loop stuff and provides a simple callback based api to use.
  
  recommended mainly for jams and small prototypes.
  for larger games i'd recommend controlling the main loop yourself.

- 2D.h:
  handy features for 2D rendering
    - sprite batcher

planned:
- text renderer
- simple tilemap renderer
- 3D.h: 3D rendering stuff:
  - gltf model loading
  - skeletal animation
  - mesh
- defaults/
  - 2D.h: handy defaults for 2D:
    - default 2D shapes rendering:
      - line rendering
      - curve rendering ( bezier )
      - rectangles
      - circles
      - regular polygons with arbitrary number of sides ( maybe capped at like 10 or 12? )
  - 3D.h: handy defaults for 3D:
    - default materials and shaders:
      - toon
      - pbr
      - retro psx
      - unshaded
    - default shapes and meshes:
      - 3D lines
      - 3D curves
      - cube
      - sphere
      - torus
      - billboarded quads

## simple theoretical 3D app?

```c
// simple3D.c
#include <milkshake/milkshake.h>
// loads the default psx style renderer settings
// comes with:
// preset vertex format,
// preset material format
// preset shaders
#define MILKSHAKE3D_DEFAULT_PSX
#include <milkshake/defaults/3D.h>
#include <milkshake/defaults/camera.h>

int
main(void) {
  int window_flags =
    MS_WINDOW_MSAA4
  | MS_WINDOW_RESIZABLE
  | MS_WINDOW_FULLSCREEN
  ;
  
  ms_init_window(940, 720, "simple 3D", window_flags);

  int flags = MS3D_VERTEX_SNAPPING;
  // initializes the default psx renderer, loads up the default shaders and such
  ms3d_init_renderer(flags);

  // loads in a car model from a gltf file
  // models are treated as immutable readonly data
  ms_handle * car = ms3D_load_model("assets/car.gltf", 0);

  // initializes a new 3D camera
  ms_camera cam = new_cam(...);
  // creates a new mutable instance of a model
  ms_modinst car_inst = ms_minst_from(car);

  while(!ms_should_quit()) {
    // polls for input events, and does timekeeping stuff
    ms_update_engine();

    // updates camera using a default camera update system
    msdef_update_cam(&(msdef_cam_type){
      .camtype = MSCAM_ORBITAL,
      .autorotate = true,
      .rotspeed = 0.1f
    });

    // sets up renderer state
    ms3D_begin_render();
      ms_clear_bg(0x050505ff);
      // binds the global camera state
      ms3D_bind_cam();
      // submits a model instance for rendering
      ms3D_draw_modinst(car_inst);
    // sorts the render commands and submits them to the GPU for rendering
    ms3D_end_render();
  }

  // frees all held resources
  ms3D_cleanup();
  ms_cleanup();
  return 0;
}

```

# Why's it called milkshake?
because i had a really nice strawberry cheesecake milkshake once and i named my engine that i was working on at the time "cheesecake" after it.
a lot of the basic code in this library was taken from that engine, so i named it milkshake.
it really was a really nice milkshake.

# License
MIT License

Copyright (c) 2026 derpyzza

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

