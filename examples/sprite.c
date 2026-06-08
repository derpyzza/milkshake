#include <SDL3/SDL_video.h>
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

int
main(void) {
  ms_window window = ms_init_window(WINW, WINH, "spritebatch", 0);

  ms_texture tex = ms_load_texture("./res/sprites.png",
     &(ms_sampler){GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST});

  // use the default sprite shader, defined in `milkshake/2D.h`
  ms_shader shader = ms2D_sprite_shader();

  ms_uniform
    u_proj_loc = ms_get_uniform(shader, "proj")
    , u_view_loc = ms_get_uniform(shader, "view")
    ;

  ms2D_spritebatch batch = ms2D_create_spritebatch(tex, NUM_SPRITES);

  mat4s proj = glms_ortho(
    0
    , WINW
    , WINH
    , 0
    , 0
    , 10
  );
  
  vec2s ssize = {{64, 64}};
  
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

  glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  bool quit = false;
  while(!quit) {
    ms_update();
    if(ms_is_key_down(Key_Escape)) quit = true;

    if(ms_is_key_pressed(Key_R)) {
      for(int i = 0; i < NUM_SPRITES; i++ ) {
        float x = SDL_rand(600) + 100;
        float y = SDL_rand(600) + 100;
        sprites[i].pos.x = x;
        sprites[i].pos.y = y;
      }
    }

    for(int i = 0; i < NUM_SPRITES; i++ ) {
      ms2D_spritebatch_submit(&batch, sprites[i]);
    }

    glViewport(0, 0, WINW, WINH);
    ms_clear_colour(0xaaaaaaff);
    glClear(GL_COLOR_BUFFER_BIT);
      ms_bind_shader(shader);
      ms_shader_set_mat4(shader, u_proj_loc, &proj, false);
      ms_shader_set_mat4(shader, u_view_loc, &GLMS_MAT4_IDENTITY, false);
      glBindTexture(GL_TEXTURE_2D, tex.id);

      ms2D_spritebatch_flush(&batch);

    SDL_GL_SwapWindow(window.handle);
  }
}
