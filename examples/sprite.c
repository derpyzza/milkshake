#include "milkshake/milkshake.h"
#include "milkshake/ms2d.h"

#include "common.h"

#define NUM_SPRITES 32
#define WINW 800
#define WINH 800

// tiny batch sprite rendering example
// sets up a basic render batch ( defined in `milkshake/ms2d.h` and renders a bunch of
// textured sprites using it randomly.
//
// press 'r' to reposition the sprites randomly.
// hover over a sprite to select, hold lmb to drag it around.

typedef struct {
  ms2d_rectangle src, dest;
  uint colour;
} sprite;

int
main(void) {
  ms_window window = ms_init_window(WINW, WINH, "spritebatch", 0);
  SDL_HideCursor();

  ms_texture tex = ms_load_texture("./res/ball.png", &PIXEL_SPRITE_SAMPLER);

  // use the default sprite shader, defined in `milkshake/2D.h`
  ms_shader shader = ms2d_sprite_shader();

  ms_uniform
    u_proj_loc = ms_get_uniform(shader, "proj")
    , u_view_loc = ms_get_uniform(shader, "view")
    ;

  // NUM_SPRITES + 1 for the custom cursor sprite
  ms2d_renderbatch batch = ms2d_new_renderbatch();
  ms2d_bind_renderbatch(&batch);

  mat4s proj = glms_ortho(
    0
    , WINW
    , WINH
    , 0
    , 0
    , 10
  );
  
  vec2s ssize = {{64, 64}};

  // timing stuff, used to change the window title once every second.
  u64 now = 0, last_now = 0;
  u64 fps_timer = 0;
  float delta = 0;
  
  // init sprites array
  sprite sprites[NUM_SPRITES] = { 0 };
  for ( int i = 0; i < NUM_SPRITES; i++ ) {
    // spawn sprites in a 600x600 region in the middle of the screen
    // the +100 is to offset it to the middle of the screen ((WINW-600)/2)
    float x = SDL_rand(600) + 100;
    float y = SDL_rand(600) + 100;
    // uv offset to randomly pick a sprite
    float offset = SDL_rand(2);

    sprites[i] = (sprite){
      {offset * 64, 0, ssize.x, ssize.y},
      {x, y, ssize.x, ssize.y},
      0xFFFFFFFF
    };
  }

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  vec2s mpos = ms_mouse_pos();

  ms2d_sprite cursor = ms2d_create_sprite(mpos, (vec2s){{32, 32}}, (vec2s){{0, 64}}, (vec2s){{32, 32}});
  cursor.origin = (vec2s){{0, 0}};

  sprite * last_sprite = NULL;
  bool grabbed = false;
  bool overlap = false;

  bool quit = false;
  while(!quit) {
    last_now = now, now = SDL_GetTicks();
    delta = (now - last_now) / 1000.0f;

    mpos = ms_mouse_pos();

    ms_update();
    if(ms_is_key_pressed(Key_Escape) || ms_should_quit()) quit = true;

    if(overlap && ms_is_mouse_down(SDL_BUTTON_LEFT)) {
      grabbed = true;
    } else {
      grabbed = false;
    }

    overlap = false;


    if(ms_is_key_pressed(Key_R)) {
      for(int i = 0; i < NUM_SPRITES; i++ ) {
        float x = SDL_rand(600) + 100;
        float y = SDL_rand(600) + 100;
        sprites[i].dest.x = x;
        sprites[i].dest.y = y;
      }
    }

    if(grabbed) {
      last_sprite->dest.x = mpos.x;
      last_sprite->dest.y = mpos.y;
    }

    glViewport(0, 0, WINW, WINH);
    ms_clear_colour(0x140e2cff);
    glClear(GL_COLOR_BUFFER_BIT);
      ms_bind_shader(shader);
      ms_shader_set_mat4(shader, u_proj_loc, &proj, false);
      ms_shader_set_mat4(shader, u_view_loc, &GLMS_MAT4_IDENTITY, false);

      for(int i = 0; i < NUM_SPRITES; i++ ) {
        sprite spr = sprites[i];
        if(
          mpos.x > spr.dest.x - spr.dest.w/2 && mpos.x < spr.dest.x + spr.dest.w/2
          &&
          mpos.y > spr.dest.y - spr.dest.h/2 && mpos.y < spr.dest.y + spr.dest.h/2
        ) {
          spr.colour = 0xFF8888FF;
          overlap = true;
          if(!grabbed) last_sprite = &sprites[i];
        }
        ms2d_texrect(tex, spr.src, spr.dest, spr.colour);
      }

      // draw cursor
      ms2d_texrect(tex, (ms2d_rectangle){0, 64, 32, 32}, (ms2d_rectangle){mpos.x, mpos.y, 32, 32}, 0xFFFFFFFF);
      ms2d_flush();
    SDL_GL_SwapWindow(window.handle);

    // update the window title once per second.
    fps_timer += delta * 1000;
    if(fps_timer >= 1000) {
      char title[128];
      sprintf(title, "Spritebatch example: num drawcalls: %i", ms_num_drawcalls());
      SDL_SetWindowTitle(window.handle, title);
      fps_timer = 0;
    }
  }
}
