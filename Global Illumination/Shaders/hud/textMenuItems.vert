#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uvec3 inIDs;

layout(location = 0) out vec2 outTexCoords;
layout(location = 1) out vec3 outColor;
layout(location = 2) out uvec3 outIDs;
layout(location = 3) out vec2 outPosition;

layout(binding = 0) uniform UniformBufferObject
{
	vec3 color[128];
    vec3 posOffset[128];
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = vec4(inPosition + ubo.posOffset[inIDs[1]].xy, 0.0, 1.0);

    outTexCoords = inTexCoord;
    outColor = ubo.color[inIDs[1]];
    outIDs = inIDs;
    outPosition = gl_Position.xy;
}  