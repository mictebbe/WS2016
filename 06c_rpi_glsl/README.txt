basic opengl es2 demo with shaders for raspberry pi

to build: 
  make

to run: change permissions to vchiq device once:
  sudo chmod a+rw /dev/vchiq
  ./glsl_demo.bin

files:
  fragment.glsl fragment shader file, is loaded an compiled on runtime
  vertex.glsl   vertex shader file, "
  main.c 	main function. initalizes render pipeline and sets up basic animator loop
  render.c	function to initialize render pipeline and call draw function
  util.c 	util function for file loading
  readBMP.c 	functions to load bmp
