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
