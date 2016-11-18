#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include "bcm_host.h"
#include "util.h"
#include "render.h"
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "readBMP.h"

#define SHADER_SIZE 128*128
#define VERTEX_BUF_SIZE 512*512

int n_vertices = 0;
GLfloat vertex_data [VERTEX_BUF_SIZE*12];

typedef struct
{
   uint32_t screen_width;
   uint32_t screen_height;

   // OpenGL|ES objects
   EGLDisplay display;
   EGLSurface surface;
   EGLContext context;

   GLuint verbose;
   GLuint vshader;
   GLuint fshader;
   GLuint program;
   GLuint tex_fb;
   GLuint tex;
   GLuint buf_vertex;

   Image img;
   float time_passed;
   float resX;
   float resY;
   GLuint unif_color, attr_vertex, attr_color, attr_texcoord, unif_scale, unif_offset, unif_tex, unif_centre, unif_time;

} CUBE_STATE_T;

static CUBE_STATE_T _state, *state=&_state;

#define check() assert(glGetError() == 0)

static void showlog(GLint shader) {
	// Prints the compile log for a shader
   	char log[1024];
   	glGetShaderInfoLog(shader,sizeof log,NULL,log);
  	printf("%d:shader:\n%s\n", shader, log);
}

static void showprogramlog(GLint shader) {
   	// Prints the information log for a program object
   	char log[1024];
   	glGetProgramInfoLog(shader,sizeof log,NULL,log);
   	printf("%d:program:\n%s\n", shader, log);
}

static void init_ogl(CUBE_STATE_T *state) {

	int32_t success = 0;
   	EGLBoolean result;
   	EGLint num_config;

   	static EGL_DISPMANX_WINDOW_T nativewindow;

   	DISPMANX_ELEMENT_HANDLE_T dispman_element;
   	DISPMANX_DISPLAY_HANDLE_T dispman_display;
   	DISPMANX_UPDATE_HANDLE_T dispman_update;
   	VC_RECT_T dst_rect;
   	VC_RECT_T src_rect;

   	static const EGLint attribute_list[] = {
      		EGL_RED_SIZE, 8,
      		EGL_GREEN_SIZE, 8,
      		EGL_BLUE_SIZE, 8,
      		EGL_ALPHA_SIZE, 8,
      		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      		EGL_NONE
   	};

   	static const EGLint context_attributes[] = {
      		EGL_CONTEXT_CLIENT_VERSION, 2,
      		EGL_NONE
   	};

	EGLConfig config;

  	// get an EGL display connection
   	state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   	assert(state->display!=EGL_NO_DISPLAY);
   	check();

   	// initialize the EGL display connection
   	result = eglInitialize(state->display, NULL, NULL);
   	assert(EGL_FALSE != result);
   	check();

   	// get an appropriate EGL frame buffer configuration
   	result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
   	assert(EGL_FALSE != result);
   	check();

   	// get an appropriate EGL frame buffer configuration
   	result = eglBindAPI(EGL_OPENGL_ES_API);
   	assert(EGL_FALSE != result);
   	check();

   	// create an EGL rendering context
   	state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attributes);
   	assert(state->context!=EGL_NO_CONTEXT);
   	check();

   	// create an EGL window surface
   	success = graphics_get_display_size(0 /* LCD */, &state->screen_width, &state->screen_height);
   	assert( success >= 0 );

   	dst_rect.x = 0;
   	dst_rect.y = 0;
   	dst_rect.width = state->screen_width;
   	dst_rect.height = state->screen_height;

   	src_rect.x = 0;
   	src_rect.y = 0;
   	src_rect.width = state->screen_width << 16;
   	src_rect.height = state->screen_height << 16;

   	dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
   	dispman_update = vc_dispmanx_update_start( 0 );

   	dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
      		0/*layer*/, &dst_rect, 0/*src*/,
      		&src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);

   	nativewindow.element = dispman_element;
   	nativewindow.width = state->screen_width;
   	nativewindow.height = state->screen_height;
   	vc_dispmanx_update_submit_sync( dispman_update );
   	check();

   	state->surface = eglCreateWindowSurface( state->display, config, &nativewindow, NULL );
   	assert(state->surface != EGL_NO_SURFACE);
   	check();

   	// connect the context to the surface
   	result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
   	assert (EGL_FALSE != result);
   	check();

   	// Set background color and clear buffers
   	glClearColor (0.15f, 0.25f, 0.35f, 1.0f);
   	glClear (GL_COLOR_BUFFER_BIT);

   	check();
}



void init_shaders (char* vertexShaderFile, char* fragmentShaderFile) {

	const GLchar *vshader_source = (GLchar*)malloc(SHADER_SIZE);
   	const GLchar *fshader_source = (GLchar*)malloc(SHADER_SIZE);

	util_load_file ((char*)vshader_source, vertexShaderFile);
	util_load_file ((char*)fshader_source, fragmentShaderFile);

        state->vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(state->vshader, 1, &vshader_source, 0);
        glCompileShader(state->vshader);
        check();

        if (state->verbose)
            showlog(state->vshader);

        state->fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(state->fshader, 1, &fshader_source, 0);
        glCompileShader(state->fshader);
        check();

        if (state->verbose)
            showlog(state->fshader);


        state->program = glCreateProgram();
        glAttachShader(state->program, state->vshader);
        glAttachShader(state->program, state->fshader);
        glLinkProgram(state->program);
        check();

        if (state->verbose)
            showprogramlog(state->program);


        state->attr_vertex    = 0;
	glBindAttribLocation (state->program, state->attr_vertex, "vertex");

	state->attr_color   = 1;
	glBindAttribLocation (state->program, state->attr_color, "color");

	state->attr_texcoord = 2;
	glBindAttribLocation (state->program, state->attr_texcoord, "texcoord");


        state->unif_color  = glGetUniformLocation (state->program, "u_color");
        state->unif_scale  = glGetUniformLocation (state->program, "u_scale");
        state->unif_offset = glGetUniformLocation (state->program, "u_offset");
        state->unif_tex    = glGetUniformLocation (state->program, "u_tex");
        state->unif_centre = glGetUniformLocation (state->program, "u_centre");
        state->unif_time   = glGetUniformLocation (state->program, "u_time");



        glClearColor (0.0, 0.0, 0.0, 1.0);
        glGenBuffers (1, &state->buf_vertex);


	// Prepare a framebuffer for rendering
        glGenFramebuffers(1,&state->tex_fb);
        check();
        glBindFramebuffer(GL_FRAMEBUFFER,state->tex_fb);
        check();
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,state->tex,0);
        check();
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        check();

        // Prepare viewport
        glViewport ( 0, 0, state->screen_width, state->screen_height );
        check();
}





















void init_texture() {

	ImageLoad("lena.bmp", &state->img);
	state->resX = 16;
	state->resY = 16;

        // Prepare a texture image
        glGenTextures (1, &state->tex);
        check();
        glBindTexture (GL_TEXTURE_2D, state->tex);
        check();

        // glActiveTexture(0)
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, state->img.sizeX, state->img.sizeY, 0,
                      GL_RGB, GL_UNSIGNED_BYTE, state->img.data);
        check();
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        check();
}


void clear_vertex_data() {
	n_vertices = 0;
}

float current_color[4] = {1.0, 1.0, 1.0, 1.0};

void set_color (float r, float g, float b, float a) {
	current_color[0] = r;
	current_color[1] = g;
	current_color[2] = b;
	current_color[3] = a;
}


void add_point (float x, float y) {
	vertex_data [n_vertices*12+0] = x;
	vertex_data [n_vertices*12+1] = y;
	vertex_data [n_vertices*12+2] = 1.0;
	vertex_data [n_vertices*12+3] = 1.0;

	vertex_data [n_vertices*12+4] = current_color[0];
	vertex_data [n_vertices*12+5] = current_color[1];
	vertex_data [n_vertices*12+6] = current_color[2];
	vertex_data [n_vertices*12+7] = current_color[3];

	n_vertices++;
}

#define N_ATTR 3


void add_quad (float x, float y, float w, float h) {

	float tx0 = 0.0;
	float ty0 = 0.0;
	float tx1 = 1.0;
	float ty1 = 1.0;

	//
	vertex_data [n_vertices*4*N_ATTR+0] = x;
	vertex_data [n_vertices*4*N_ATTR+1] = y;
	vertex_data [n_vertices*4*N_ATTR+2] = 1.0;
	vertex_data [n_vertices*4*N_ATTR+3] = 1.0;

	vertex_data [n_vertices*4*N_ATTR+4] = current_color[0];
	vertex_data [n_vertices*4*N_ATTR+5] = current_color[1];
	vertex_data [n_vertices*4*N_ATTR+6] = current_color[2];
	vertex_data [n_vertices*4*N_ATTR+7] = current_color[3];

	vertex_data [n_vertices*4*N_ATTR+8] = tx0;
	vertex_data [n_vertices*4*N_ATTR+9] = ty0;
	vertex_data [n_vertices*4*N_ATTR+10] = 0;
	vertex_data [n_vertices*4*N_ATTR+11] = 0;
	n_vertices++;

	//
	vertex_data [n_vertices*4*N_ATTR+0] = x+w;
	vertex_data [n_vertices*4*N_ATTR+1] = y;
	vertex_data [n_vertices*4*N_ATTR+2] = 1.0;
	vertex_data [n_vertices*4*N_ATTR+3] = 1.0;

	vertex_data [n_vertices*4*N_ATTR+4] = 1.0;
	vertex_data [n_vertices*4*N_ATTR+5] = 1.0;
	vertex_data [n_vertices*4*N_ATTR+6] = 1.0;
	vertex_data [n_vertices*4*N_ATTR+7] = 1.0;

	vertex_data [n_vertices*4*N_ATTR+8] = tx1;
	vertex_data [n_vertices*4*N_ATTR+9] = ty0;
	vertex_data [n_vertices*4*N_ATTR+10] = 0;
	vertex_data [n_vertices*4*N_ATTR+11] = 0;
	n_vertices++;

	//
	vertex_data [n_vertices*4*N_ATTR+0] = x+w;
	vertex_data [n_vertices*4*N_ATTR+1] = y+w;
	vertex_data [n_vertices*4*N_ATTR+2] = 1.0;
	vertex_data [n_vertices*4*N_ATTR+3] = 1.0;

	vertex_data [n_vertices*4*N_ATTR+4] = current_color[0];
	vertex_data [n_vertices*4*N_ATTR+5] = current_color[1];
	vertex_data [n_vertices*4*N_ATTR+6] = current_color[2];
	vertex_data [n_vertices*4*N_ATTR+7] = current_color[3];

	vertex_data [n_vertices*4*N_ATTR+8] = tx1;
	vertex_data [n_vertices*4*N_ATTR+9] = ty1;
	vertex_data [n_vertices*4*N_ATTR+10] = 0;
	vertex_data [n_vertices*4*N_ATTR+11] = 0;
	n_vertices++;

	///////
	//
	vertex_data [n_vertices*4*N_ATTR+0] = x;
	vertex_data [n_vertices*4*N_ATTR+1] = y;
	vertex_data [n_vertices*4*N_ATTR+2] = 1.0;
	vertex_data [n_vertices*4*N_ATTR+3] = 1.0;

	vertex_data [n_vertices*4*N_ATTR+4] = current_color[0];
	vertex_data [n_vertices*4*N_ATTR+5] = current_color[1];
	vertex_data [n_vertices*4*N_ATTR+6] = current_color[2];
	vertex_data [n_vertices*4*N_ATTR+7] = current_color[3];

	vertex_data [n_vertices*4*N_ATTR+8] = tx0;
	vertex_data [n_vertices*4*N_ATTR+9] = ty0;
	vertex_data [n_vertices*4*N_ATTR+10] = 0;
	vertex_data [n_vertices*4*N_ATTR+11] = 0;
	n_vertices++;

	//
	vertex_data [n_vertices*4*N_ATTR+0] = x+w;
	vertex_data [n_vertices*4*N_ATTR+1] = y+w;
	vertex_data [n_vertices*4*N_ATTR+2] = 1.0;
	vertex_data [n_vertices*4*N_ATTR+3] = 1.0;

	vertex_data [n_vertices*4*N_ATTR+4] = current_color[0];
	vertex_data [n_vertices*4*N_ATTR+5] = current_color[1];
	vertex_data [n_vertices*4*N_ATTR+6] = current_color[2];
	vertex_data [n_vertices*4*N_ATTR+7] = current_color[3];

	vertex_data [n_vertices*4*N_ATTR+8] = tx1;
	vertex_data [n_vertices*4*N_ATTR+9] = ty1;
	vertex_data [n_vertices*4*N_ATTR+10] = 0;
	vertex_data [n_vertices*4*N_ATTR+11] = 0;
	n_vertices++;

	//
	vertex_data [n_vertices*4*N_ATTR+0] = x;
	vertex_data [n_vertices*4*N_ATTR+1] = y+w;
	vertex_data [n_vertices*4*N_ATTR+2] = 1.0;
	vertex_data [n_vertices*4*N_ATTR+3] = 1.0;

	vertex_data [n_vertices*4*N_ATTR+4] = current_color[0];
	vertex_data [n_vertices*4*N_ATTR+5] = current_color[1];
	vertex_data [n_vertices*4*N_ATTR+6] = current_color[2];
	vertex_data [n_vertices*4*N_ATTR+7] = current_color[3];

	vertex_data [n_vertices*4*N_ATTR+8] = tx0;
	vertex_data [n_vertices*4*N_ATTR+9] = ty1;
	vertex_data [n_vertices*4*N_ATTR+10] = 0;
	vertex_data [n_vertices*4*N_ATTR+11] = 0;
	n_vertices++;
}


static void draw (CUBE_STATE_T *state, GLfloat cx, GLfloat cy, GLfloat scale, GLfloat x, GLfloat y)
{
        glBindBuffer(GL_ARRAY_BUFFER, state->buf_vertex);
        glBufferData(GL_ARRAY_BUFFER, n_vertices*4*4*N_ATTR, vertex_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(state->attr_vertex);
        glVertexAttribPointer(state->attr_vertex, 4, GL_FLOAT, GL_FALSE, 4*4*N_ATTR, 32); //(GLvoid*)state->buf_vertex);
	check();

        glEnableVertexAttribArray(state->attr_color);
	glVertexAttribPointer(state->attr_color, 4, GL_FLOAT, GL_FALSE, 4*4*N_ATTR, 16); //(GLvoid*)state->buf_vertex+16);
	check();

        glEnableVertexAttribArray(state->attr_texcoord);
	glVertexAttribPointer(state->attr_texcoord, 4, GL_FLOAT, GL_FALSE, 4*4*N_ATTR, 0); //(GLvoid*)state->buf_vertex+32);
	check();


        // Now render to the main frame buffer
        glBindFramebuffer (GL_FRAMEBUFFER, 0);

        // Clear the background (not really necessary I suppose)
        glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glUseProgram (state->program);

	glActiveTexture (GL_TEXTURE0);
        glBindTexture (GL_TEXTURE_2D, state->tex);

        glUniform1i (state->unif_tex, 0);
        glUniform1f (state->unif_time, state->time_passed);

        glDrawArrays (GL_TRIANGLES, 0, n_vertices);
        check();

        glBindBuffer (GL_ARRAY_BUFFER, 0);

        glFlush();
        glFinish();

        eglSwapBuffers(state->display, state->surface);

	glDisableVertexAttribArray (state->attr_vertex);
	glDisableVertexAttribArray (state->attr_color);
	glDisableVertexAttribArray (state->attr_texcoord);
}

GLfloat cx, cy;

void render_init () {

	state->time_passed = 0;
 	bcm_host_init();

	// Clear application state
 	memset (state, 0, sizeof(*state));


 	// Start OGLES
 	init_ogl(state);
 	init_shaders ("vertex.glsl", "fragment.glsl");
	init_texture();

 	cx = state->screen_width/2;
 	cy = state->screen_height/2;
}


void render_update (float dt) {

	state->time_passed += dt;
	int x, y, b;
	draw (state, cx, cy, 0.003, x, y);
}
