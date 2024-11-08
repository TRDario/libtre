#version 450

layout (location = 0) uniform mat4 u_camera;
layout (location = 0) in      vec2 v_pos;
layout (location = 1) in      vec2 v_uv;
layout (location = 2) in      vec4 v_color;
layout (location = 0) out     vec2 vf_uv;
layout (location = 1) out     vec4 vf_color;

void main() {
	vf_uv = v_uv;
	vf_color = v_color;
	gl_Position = u_camera * vec4(v_pos, 0.0, 1.0);
}