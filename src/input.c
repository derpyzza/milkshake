#include "internal.h"
#include <cglm/struct.h>

bool
ms_is_key_pressed(int key) {
  return G_core.keyboard.keystate[key] && !(G_core.keyboard.prev_state[key]);
}

bool
ms_is_key_released(int key) {
  return !G_core.keyboard.keystate[key];
}

bool
ms_is_key_down(int key) {
  return G_core.keyboard.keystate[key] && G_core.keyboard.prev_state[key];
}

bool
ms_mouse_pressed(u8 key) {
  return
    (G_core.mouse.btn_state & SDL_BUTTON_MASK(key))
    && !(G_core.mouse.prev_state & SDL_BUTTON_MASK(key));
}

bool
ms_mouse_released(u8 key) {
  return !(G_core.mouse.btn_state & SDL_BUTTON_MASK(key));
}

bool
ms_mouse_down(u8 key) {
  return
    (G_core.mouse.btn_state & SDL_BUTTON_MASK(key))
    && (G_core.mouse.prev_state & SDL_BUTTON_MASK(key));
}

vec2s ms_mouse_pos (void) {
  return G_core.mouse.pos;
}

f32 ms_mouse_x (void) {
  return G_core.mouse.pos.x;
}

f32 ms_mouse_y (void) {
  return G_core.mouse.pos.y;
}

vec2s ms_mouse_delta (void) {
  return G_core.mouse.pos_delta;
}

f32 ms_mouse_delta_x (void) {
  return G_core.mouse.pos_delta.x;
}

f32 ms_mouse_delta_y (void) {
  return G_core.mouse.pos_delta.y;
}

f32 ms_mouse_scroll_amt (void) {
  return G_core.mouse.scroll;
}

f32 ms_mouse_scroll_delta (void) {
  return G_core.mouse.scroll - G_core.mouse.prev_scroll;
}
