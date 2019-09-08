#version 450

layout(binding = 0) uniform UniformBufferObjectVP
{
    mat4 view;
    mat4 proj;
} uboVP;

layout(binding = 1) uniform UniformBufferObjectModel
{
    mat4 model;
} uboModel;

// Per vertex
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main()
{
	gl_Position =  uboVP.proj * uboVP.view * uboModel.model * vec4(inPosition, 1.0);
}