#include <milkshake/milkshake.h>
#include <milkshake/2D.h>

#define NUM_SPRITES 32
#define WINW 800
#define WINH 800

// tiny spritebatch rendering example
// sets up a basic spritebatch ( defined in `milkshake/2D.h` and renders a bunch of
// textured sprites using it randomly.
//
// press 'r' to reposition the sprites randomly.
// hover over a sprite to select, hold lmb to drag it around.

int
main(void) {
  ms_window window = ms_init_window(WINW, WINH, "spritebatch", 0);
  SDL_HideCursor();

  ms_texture tex = ms_load_texture("./res/ball.png",
     &(ms_sampler){GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST});

  // use the default sprite shader, defined in `milkshake/2D.h`
  ms_shader shader = ms2D_sprite_shader();

  ms_uniform
    u_proj_loc = ms_get_uniform(shader, "proj")
    , u_view_loc = ms_get_uniform(shader, "view")
    ;

  // NUM_SPRITES + 1 for the custom cursor sprite
  ms2D_spritebatch batch = ms2D_create_spritebatch(tex, NUM_SPRITES + 1);

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
  ms2D_sprite sprites[NUM_SPRITES] = { 0 };
  for ( int i = 0; i < NUM_SPRITES; i++ ) {
    // spawn sprites in a 600x600 region in the middle of the screen
    // the +100 is to offset it to the middle of the screen ((WINW-600)/2)
    float x = SDL_rand(600) + 100;
    float y = SDL_rand(600) + 100;
    // uv offset to randomly pick a sprite
    float offset = SDL_rand(2);

    sprites[i] = ms2D_create_sprite(
      (vec2s){{x, y}},
      ssize,
      (vec2s){{offset * 64, 0}},
      ssize
    );
  }

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  vec2s mpos = ms_mouse_pos();

  ms2D_sprite cursor = ms2D_create_sprite(mpos, (vec2s){{32, 32}}, (vec2s){{0, 64}}, (vec2s){{32, 32}});
  cursor.origin = (vec2s){{0, 0}};

  ms2D_sprite * last_sprite = NULL;
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
        sprites[i].pos.x = x;
        sprites[i].pos.y = y;
      }
    }

    if(grabbed) {
      last_sprite->pos = mpos;
    }

    for(int i = 0; i < NUM_SPRITES; i++ ) {
      ms2D_sprite spr = sprites[i];
      if(
        mpos.x > spr.pos.x - spr.size.x/2 && mpos.x < spr.pos.x + spr.size.x/2
        &&
        mpos.y > spr.pos.y - spr.size.y/2 && mpos.y < spr.pos.y + spr.size.y/2
      ) {
        spr.colour = 0xFF0000FF;
        overlap = true;
        if(!grabbed) last_sprite = &sprites[i];
      }
      ms2D_spritebatch_submit(&batch, spr);
    }

    cursor.pos = mpos;
    ms2D_spritebatch_submit(&batch, cursor);

    glViewport(0, 0, WINW, WINH);
    ms_clear_colour(0x140e2cff);
    glClear(GL_COLOR_BUFFER_BIT);
      ms_bind_shader(shader);
      ms_shader_set_mat4(shader, u_proj_loc, &proj, false);
      ms_shader_set_mat4(shader, u_view_loc, &GLMS_MAT4_IDENTITY, false);
      glBindTexture(GL_TEXTURE_2D, tex.id);

      ms2D_spritebatch_flush(&batch);

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
