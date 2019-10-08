#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObjectVP
{
    mat4 view;
    mat4 proj;
} uboVP;

layout(binding = 1) uniform UniformBufferObjectModel
{
    mat4 model;
} uboModel;

layout(binding = 2) uniform UniformBufferObjectLightSpace
{
    mat4 lightSpace;
} uboLightSpace;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out mat3 tbn;
layout(location = 6) out vec4 posLightSpace;

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
    gl_Position = uboVP.proj * uboVP.view * uboModel.model * vec4(inPosition, 1.0);

	mat3 usedModelMatrix = transpose(inverse(mat3(uboModel.model)));
    vec3 n = normalize(usedModelMatrix * inNormal);
	vec3 t = normalize(usedModelMatrix * inTangent);
	//t = normalize(t - dot(t, n) * n);
	vec3 b = normalize(cross(t, n));
	tbn = inverse(mat3(t, b, n));

	worldPos = vec3(uboModel.model * vec4(inPosition, 1.0));
    fragTexCoord = inTexCoord;

	posLightSpace = biasMat * uboLightSpace.lightSpace * vec4(inPosition, 1.0);
} 