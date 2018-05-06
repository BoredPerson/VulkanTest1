#version 450
#extension GL_ARB_seperate_shader_objects : enable

layout(location = 0) out vec4 outColor;

void main(){
	outColor = vec4(0.0, 1.0, 0.0, 1.0);
}