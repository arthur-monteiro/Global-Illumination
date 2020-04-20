#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inPosition;

layout(location = 0) out vec4 outColor;

void main() 
{
	if(inColor.x < 0.0 || inPosition.y < -0.7 || inPosition.y > 0.8)
		discard;
	outColor = vec4(inColor);
}