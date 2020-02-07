#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() 
{
	if(inColor.x < 0.0)
		discard;
	outColor = vec4(inColor);
}