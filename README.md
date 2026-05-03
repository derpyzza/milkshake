
milkshake — tiny gamedev framework.

>[!Note]
> this library is currently under development and everything you see here is subject to change
> i haven't fully decided on what i want the final library to work like
> so the library is currently going through a heavy experimental phase where i try out ideas
> and potentially discard them

this is an attempt to take some scattered engines and games i've made with opengl and
combine them all into one handy little library so i don't have to continually
keep copy pasting the same initialization code over and over again per project.


contents:
- milkshake.h: the necessary basics
  - window and gl context creation
  - timer functionality
  - input management
  - viewport and render-to-texture functionality
  - shader loading and compilation and management
  - text rendering
  - resource management?
  - draw-call batcher?
  - cmdline option handling?
- 2D.h: handy features for 2D rendering
  - sprite batcher
- 3D.h: 3D rendering stuff:
  - gltf model loading
  - skeleton
  - mesh

- defaults/2D.h: handy defaults for 2D:
  - 

- defaults/3D.h: handy defaults for 3D:
  - default materials and shaders:
    - toon
    - pbr
    - retro psx
    - unshaded
  - default shapes and meshes:
    - cube
    - sphere
    - torus
    - billboarded quads

## simple theoretical 3D app?
```c
// simple3D.c

#define MILKSHAKE_BACKEND_GL45
#include <milkshake/milkshake.h>
// loads the default psx style renderer settings
// comes with:
// preset vertex format,
// preset material format
// preset shaders
#define MILKSHAKE3D_DEFAULT_PSX
#include <milkshake/defaults/3D.h>

int
main(void) {
  int window_flags =
    MS_WINDOW_MSAA4
  | MS_WINDOW_RESIZABLE
  | MS_WINDOW_FULLSCREEN
  ;
  
  ms_init_window(940, 720, "simple 3D", window_flags);

  int flags = MS3D_VERTEX_SNAPPING;
  ms3d_init_renderer(flags);

  const ms_handle * car = ms_load_model("assets/car.gltf", 0);

  ms_camera cam = new_cam(...);
  ms_model_inst car_inst = ms_minst_from(car);

  while(!ms_should_quit()) {
    ms_clear_bg();

    ms_bind_shader(ms_default_3D);    
      ms_bind_cam(cam);
      ms3d_draw_cube(pos, rot, col);
    ms_swap();
  }
}

```

# License
MIT License

Copyright (c) 2026 derpyzza

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

