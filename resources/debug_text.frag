#version 450

layout(location = 2) uniform sampler2D u_texture;
layout(location = 0) in vec2 vf_uv;
layout(location = 1) in vec4 vf_textColor;
layout(location = 2) in vec4 vf_backgroundColor;
layout(location = 0) out vec4 f_color;

void main()
{
	f_color = bool(texture(u_texture, vf_uv).r) ? vf_textColor : vf_backgroundColor;
}