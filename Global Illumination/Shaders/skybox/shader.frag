#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler skyboxSampler;
layout(binding = 2) uniform textureCube skyboxImage;

layout(location = 0) in vec3 texCoords;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(samplerCube(skyboxImage, skyboxSampler), texCoords);
}