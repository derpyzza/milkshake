#include "internal.h"
#include <cglm/struct.h>

bool
ms_key_pressed(int key) {
  return G_core.keyboard.keystate[key] && !(G_core.keyboard.prev_state[key]);
}

bool
ms_key_released(int key) {
  return !G_core.keyboard.keystate[key];
}

bool
ms_key_down(int key) {
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
