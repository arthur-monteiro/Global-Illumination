#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D tex;
layout (binding = 1) uniform UniformBufferObjectOpacity
{
	float opacity;
} opacity;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor; 	

void main() 
{
	vec3 color = texture(tex, inTexCoord).rgb;
	outColor = vec4(color.rgb, min(opacity.opacity, color.b));
}