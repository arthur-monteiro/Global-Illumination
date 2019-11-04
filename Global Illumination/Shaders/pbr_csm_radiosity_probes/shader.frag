#version 450
#extension GL_ARB_separate_shader_objects : enable

#define CASCADES_COUNT 4

layout(binding = 2) uniform UniformBufferObjectLights
{
	vec4 camPos;
	
	vec4 dirDirLight;
	vec4 colorDirLight;

	float ambient;
} uboLights;

layout(binding = 3) uniform UniformBufferObjectRadiosityProbes
{
	vec4 radiosity[1000];
} uboRadiosityProbes;

layout(binding = 4) uniform sampler2D texAlbedo;
layout(binding = 5) uniform sampler2D texNormal;
layout(binding = 6) uniform sampler2D texRoughness;
layout(binding = 7) uniform sampler2D texMetal;
layout(binding = 8) uniform sampler2D texAO;

layout (binding = 9) uniform sampler2D screenShadows;

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 screenPos;
layout(location = 3) in mat3 tbn;

layout(location = 0) out vec4 outColor;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

const float PI = 3.14159265359;

float getRadiosity(int i, int j, int k, vec3 n)
{
	vec3 probePos = vec3(20.0 - i * 4.0, 0.1 + j * 1.7, -10.0 + k * 2.5);
	if(dot(n, probePos - worldPos) <= 0)
		return 0.0;

	if(i >= 0 && j >= 0 && j >= 0 && i <= 9 && j <= 9 && k <= 9)
		return uboRadiosityProbes.radiosity[100 * i + 10 * j + k].x * max(dot(n, normalize(probePos - worldPos)), 0.0);

	return 0.0;
}

void main() 
{
	vec2 coordShadow = ((screenPos.xy / screenPos.w) + 1.0) / 2.0;
	float shadow = texture(screenShadows, coordShadow).x;

	vec3 albedo = pow(texture(texAlbedo, fragTexCoord).xyz, vec3(2.2));
	if(shadow == 0.0)
		outColor = vec4(vec3(uboLights.ambient) * albedo, texture(texAlbedo, fragTexCoord).a);
		
	float roughness = texture(texRoughness, fragTexCoord).x;
	float metallic = texture(texMetal, fragTexCoord).x;
	float ao = texture(texAO, fragTexCoord).x;
	vec3 normal = (texture(texNormal, fragTexCoord).xyz * 2.0 - vec3(1.0)) * tbn;

	vec3 N = normalize(normal); 
    vec3 V = normalize(uboLights.camPos.xyz - worldPos);
	vec3 R = reflect(-V, N);  

	float probeI = (worldPos.x - 20.0) / -4.0;
	float probeJ = (worldPos.y - 0.1) / 1.7;
	float probeK = (worldPos.z + 10.0) / 2.5;

	int probeI0 = int(floor(probeI));
	int probeI1 = probeI0 + 1;
	float dProbI = fract(probeI);

	int probeJ0 = int(floor(probeJ));
	int probeJ1 = probeJ0 + 1;
	float dProbJ = fract(probeJ);

	int probeK0 = int(floor(probeK));
	int probeK1 = probeK0 + 1;
	float dProbK = fract(probeK);

	float ambient = 0.0;
	ambient += getRadiosity(probeI0, probeJ0, probeK0, N) * (1.0 - dProbI) * (1.0 - dProbJ) * (1.0 - dProbK);
	ambient += getRadiosity(probeI0, probeJ0, probeK1, N) * (1.0 - dProbI) * (1.0 - dProbJ) * dProbK;
	ambient += getRadiosity(probeI0, probeJ1, probeK0, N) * (1.0 - dProbI) * dProbJ * (1.0 - dProbK);
	ambient += getRadiosity(probeI0, probeJ1, probeK1, N) * (1.0 - dProbI) * dProbJ * dProbK;
	ambient += getRadiosity(probeI1, probeJ0, probeK0, N) * dProbI * (1.0 - dProbJ) * (1.0 - dProbK);
	ambient += getRadiosity(probeI1, probeJ0, probeK1, N) * dProbI * (1.0 - dProbJ) * dProbK;
	ambient += getRadiosity(probeI1, probeJ1, probeK0, N) * dProbI * dProbJ * (1.0 - dProbK);
	ambient += getRadiosity(probeI1, probeJ1, probeK1, N) * dProbI * dProbJ * dProbK;
	ambient /= 6.0;

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0,albedo, metallic);
	
	vec3 Lo = vec3(0.0);
	
	// calculate per-light radiance
	vec3 L = normalize(-uboLights.dirDirLight.xyz);
	vec3 H = normalize(V + L);
	vec3 radiance     = uboLights.colorDirLight.xyz;        
	
	// cook-torrance brdf
	float NDF = DistributionGGX(N, H, roughness);        
	float G   = GeometrySmith(N, V, L, roughness);      
	vec3 F    = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, roughness);       
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	  
	
	vec3 nominator    = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0); 
	vec3 specular     = nominator / max(denominator, 0.001);
		
	// add to outgoing radiance Lo
	float NdotL = max(dot(N, L), 0.0);                
	Lo += (kD * albedo / PI + specular) * radiance * NdotL;

	//vec3 ambient = vec3(uboLights.ambient) * albedo;

    vec3 color = max(ambient, 0.01) * albedo * (1.0 - shadow) + Lo * shadow;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, texture(texAlbedo, fragTexCoord).a);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}