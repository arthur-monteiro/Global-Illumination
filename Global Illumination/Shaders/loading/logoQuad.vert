#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 2) uniform UniformBufferObjectOffset
{
	float offset;
} offset;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 outTexCoords;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    if(inPosition.y >= 1.0f)
        gl_Position = vec4(inPosition, 0.0, 1.0);
    else 
        gl_Position = vec4(inPosition.x, inPosition.y - offset.offset, 0.0, 1.0);

    outTexCoords = inTexCoord;
}  