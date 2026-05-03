#include "milkshake/milkshake.hh"

void update() {

}

void render_game() {
  ms_clear(0x000000);

  ms_draw_cube(x, y, z, w, h, b, colour);
}

void render_ui() {

}

int
main(void) {
  ms_init_window(320, 460, "game window");

  string text = "Hello world";
  string other = "Beep Boop";

  RenderView game_view = new RenderView();

  while(!ms_should_quit()) {

    update();

    ms_begin_view(view);
      render_game();    
    ms_end_view();

    ms_begin_view(default_view);
      ms_draw_texture(game_view.texture, 0, 0, game_view.width, game_view.height, WHITE);
      render_ui();
    ms_end_view();
    

  }
}
