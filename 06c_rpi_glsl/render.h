#ifndef RENDER_H
#define RENDER_H

void render_init ();
void render_update (float dt);

void add_quad (float x, float y, float w, float h);
void add_point (float x, float y);
void set_color (float r, float g, float b, float a);
void clear_vertex_data();
void init_shader (char* vertexShaderFile, char* fragmentShaderFile);
#endif
