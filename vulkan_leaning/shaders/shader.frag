#version 450
#extension GL_ARB_seperate_shader_objects : enable

layout(location = 0) in vec3 vs_col;

layout(location = 0) out vec4 frag_col;

void main () {
	frag_col = vec4(vs_col, 1.0);
}
