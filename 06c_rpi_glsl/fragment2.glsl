uniform sampler2D u_tex;
uniform float u_time;

// interpolated input from vertex shader
varying vec4 v_color;
varying vec2 v_texcoord;
varying vec3 v_position;

void main(void) {

	// read texture
	vec3 cc = texture2D (u_tex, v_texcoord*sin(u_time*0.001)).xyz;

	
	// output pixel color
	gl_FragColor = vec4(cc, v_color.a);
}

