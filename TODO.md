opengl render loop thingy:

- create window
- create rendering context
- [create any rendering surfaces you may need]
- load and prime shaders
- create buffer, element and array objects
- start draw loop
- [bind rendering surface]
- clear background
- bind shader
- [bind uniforms]
- [bind textures]
- bind array object
- draw vertices
- switch backbuffers
- goto start of loop


how should i handle events?
callback based:
```c
// milkshake
void ms_set_event_handler(fnptr(void, handler, SDL_Event));

// game

void _process_events(SDL_Event e) {
  switch(e.type) {
    if (e.type == SDL_EVENT_QUIT) quit = true;
  }
  return;
}

int main() {
  ...  

  ms_set_event_handler(_process_events); 

  ...

  while(!quit) {
    ms_update(); // internally calls _process_events();
  }
}
```

or direct:

```c
int main() {
  ...
  while(!quit) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
      // handles internal events
      ms_process_events(e);

      if (e.type == SDL_EVENT_QUIT) quit = true;
    }
    // inputs internal input state
    ms_update();

    ...
  }
}
