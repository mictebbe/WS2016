// attributes input from graphics memory
attribute vec4 vertex;
attribute vec4 color;
attribute vec4 texcoord;

// user defined dynamic input variables
uniform float u_time;

// pass interpolated variables to fragment shader
varying vec4 v_color;
varying vec2 v_texcoord;
varying vec3 v_position;

void main(void) {

	v_color = color;	
	v_texcoord = texcoord.xy;
	v_position = vertex.xyz;

	vec3 posNew = vec3 (vertex.xyz);
	posNew.x += sin (u_time * 0.001 + vertex.y);

	// screen positoin output
	gl_Position = vec4(posNew, 1.0);
}
