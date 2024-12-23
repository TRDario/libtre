#version 450

#define UNTEXTURED_UV vec2(-100, -100)

layout(location = 1) uniform sampler2D u_texture;
layout(location = 0) in vec2 vf_uv;
layout(location = 1) in vec4 vf_color;
layout(location = 0) out vec4 f_color;

void main()
{
	// Untextured vertex.
	if (vf_uv == UNTEXTURED_UV) {
		f_color = vf_color;
	}
	// Textured vertex.
	else {
		f_color = vf_color * texture(u_texture, vf_uv);
	}
}