#include <milkshake/milkshake.h>
#include <milkshake/2D.h>

#include "common.h"

#define NUM_SPRITES 32
#define WINW 1056
#define WINH 800

// not-so-tiny framebuffer rendering example
// sets up a simple framebuffer with a texture target and then renders
// that texture as a quad ( like a little minimap )
//
// press 'r' to reposition the sprites randomly.
// hover over a sprite to select, hold lmb to drag it around.

ms2D_sprite sprites[NUM_SPRITES] = { 0 };
ms2D_spritebatch batch;
vec2s ssize = {{64, 64}};
vec2s mpos;

ms2D_sprite * last_sprite = NULL;
bool grabbed = false;
bool overlap = false;

// fill the sprites array with ball positions
void generate_balls(void) {
  for ( int i = 0; i < NUM_SPRITES; i++ ) {
    // spawn sprites in a 600x600 region in the middle of the screen
    // the +100 is to offset it to the middle of the screen ((WINW-600)/2)
    // the +256 is to offset the region from the end of the minimap
    float x = SDL_rand(600) + 100 + 256;
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
}

void submit_balls (void) {
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
}

int
main(void) {
  ms_window window = ms_init_window(WINW, WINH, "spritebatch", 0);
  SDL_HideCursor();

  ms_texture tex = ms_load_texture("./res/ball.png", &PIXEL_SPRITE_SAMPLER);
  // use the default sprite shader, defined in `milkshake/2D.h`
  ms_shader shader = ms2D_sprite_shader();
  ms_uniform
      u_proj_loc = ms_get_uniform(shader, "proj")
    , u_view_loc = ms_get_uniform(shader, "view");
  mat4s proj = glms_ortho( 0 , WINW , WINH , 0 , 0 , 10 );

  ms_framebuffer fbo = ms_create_fbo();
  ms_texture minimap = ms_create_texture(256, 256, GL_RGBA8, GL_RGBA, NULL, &PIXEL_SPRITE_SAMPLER);
  ms_fbo_attach_texture(fbo, minimap, MS_ATTACHMENT_COLOR_N, 0, 0);
  if(!ms_fbo_is_complete(fbo)) {
    dlog_error("framebuffer incomplete: %i", glGetError());
    return -1;
  }
  ms2D_sprite minimap_sprite = ms2D_create_sprite(
    (vec2s){{0,   0  }}, // pos
    (vec2s){{256, 256}}, // size
    (vec2s){{0,   0  }}, // uv pos
    (vec2s){{256, 256}}  // uv size
  );
  minimap_sprite.origin = GLMS_VEC2_ZERO;
  minimap_sprite.flip_y = true;
  
  // timing stuff, used to change the window title once every second.
  u64 now = 0, last_now = 0;
  u64 fps_timer = 0;
  float delta = 0;

  // NUM_SPRITES + 1 for the custom cursor sprite
  batch = ms2D_create_spritebatch(tex, NUM_SPRITES + 1);
  mpos = ms_mouse_pos();
  // setup the initial sprite array
  generate_balls();
  

  ms2D_sprite cursor =
    ms2D_create_sprite(
      mpos,
      (vec2s){{32, 32}},
      (vec2s){{0, 64}},
      (vec2s){{32, 32}}
    );
  cursor.origin = GLMS_VEC2_ZERO;

  bool quit = false;

  while(!quit) {
    last_now = now, now = SDL_GetTicks();
    delta = (now - last_now) / 1000.0f;
    ms_update();

    if(ms_is_key_pressed(Key_Escape) || ms_should_quit()) {
      quit = true;
    }

    // regenerate balls
    if(ms_is_key_pressed(Key_R)) {
      generate_balls();
    }

    mpos = ms_mouse_pos();
    cursor.pos = mpos;

    if(overlap && ms_is_mouse_down(SDL_BUTTON_LEFT)) {
      grabbed = true;
    } else {
      grabbed = false;
    }
    overlap = false;

    if(grabbed) {
      last_sprite->pos = mpos;
    }

  	glEnable(GL_BLEND);
  	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  	// RENDER TO MINIMMAP ( framebuffer w/ render texture )
    ms_bind_fbo(fbo);
    glViewport(0, 0, 256, 256);
    ms_clear_colour(0x352a60ff);
    glClear(GL_COLOR_BUFFER_BIT);
      proj = glms_ortho(256, WINW, WINH, 0, 0, 10);
      ms_shader_set_mat4(shader, u_proj_loc, &proj, false);
      ms_shader_set_mat4(shader, u_view_loc, &GLMS_MAT4_IDENTITY, false);

      submit_balls();
      ms2D_spritebatch_submit(&batch, cursor);

      ms2D_spritebatch_flush(&batch);

    ms_unbind_fbo();
      glViewport(0, 0, WINW, WINH);
      ms_clear_colour(0x140e2cff);
      glClear(GL_COLOR_BUFFER_BIT);
        proj = glms_ortho(0, WINW, WINH, 0, 0, 10);
        ms_shader_set_mat4(shader, u_proj_loc, &proj, false);
        ms_shader_set_mat4(shader, u_view_loc, &GLMS_MAT4_IDENTITY, false);

        // draw balls to the screen
        submit_balls();
        ms2D_spritebatch_flush(&batch);

        // draw minimap
        ms2D_spritebatch_swap_texture(&batch, minimap);
        glDisable(GL_BLEND);
        ms2D_spritebatch_submit(&batch, minimap_sprite);
        ms2D_spritebatch_flush(&batch);
        glEnable(GL_BLEND);

        // draw cursor
        ms2D_spritebatch_swap_texture(&batch, tex);
        ms2D_spritebatch_submit(&batch, cursor);
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
