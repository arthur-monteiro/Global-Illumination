#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D samplerColor;

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

void main() 
{
	vec3 depth = texture(samplerColor, texCoords).rgb;
	outColor = vec4(depth, 1.0);
}