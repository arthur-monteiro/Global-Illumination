#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObjectVP
{
    mat4 view;
    mat4 proj;
} uboVP;

layout(binding = 1) uniform UniformBufferObjectLightSpace
{
    mat4 lightSpace;
} uboLightSpace;

// Per vertex
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

// Out
layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec4 posLightSpace;

out gl_PerVertex
{
    vec4 gl_Position;
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
    gl_Position = uboVP.proj * uboVP.view * vec4(inPosition, 1.0);
	
	mat3 usedModelMatrix = transpose(inverse(mat3(1.0)));
    normal = usedModelMatrix * inNormal;
	
	worldPos = vec3(vec4(inPosition, 1.0));
    posLightSpace = biasMat * uboLightSpace.lightSpace * vec4(inPosition, 1.0);
}