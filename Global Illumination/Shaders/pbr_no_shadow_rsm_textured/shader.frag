#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 3) uniform UniformBufferObjectLights
{
	vec4 camPos;
	
	vec4 dirDirLight;
	vec4 colorDirLight;

	float usePCF;
	float ambient;
} uboLights;

layout(binding = 4) uniform sampler2D texAlbedo;
layout(binding = 5) uniform sampler2D texNormal;
layout(binding = 6) uniform sampler2D texRoughness;
layout(binding = 7) uniform sampler2D texMetal;
layout(binding = 8) uniform sampler2D texAO;

layout(binding = 9) uniform sampler2D rsmPosition;
layout(binding = 10) uniform sampler2D rsmNormal;
layout(binding = 11) uniform sampler2D rsmFlux;

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in mat3 tbn;
layout(location = 6) in vec4 posLightSpace;

layout(location = 0) out vec4 outColor;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

const float PI = 3.14159265359;

vec2 poissonDisk[64] = vec2[]( 
	vec2( 0.063326, 0.142369 ),
	vec2( 0.155736, 0.065157 ),
	vec2( 0.170019, -0.040254 ),
	vec2( 0.203528, 0.214331 ),
	vec2( 0.222999, -0.215125 ),
	vec2( -0.098422, -0.295755 ),
	vec2( 0.099094, -0.308023 ),
	vec2( 0.034028, 0.325968 ),
	vec2( 0.039766, -0.3961 ),
	vec2( 0.175817, 0.382366 ),
	vec2( 0.421003, 0.02707 ),
	vec2( -0.44084, 0.137486 ),
	vec2( 0.487472, -0.063082 ),
	vec2( 0.464034, -0.188818 ),
	vec2( 0.470016, 0.217933 ),
	vec2( 0.200476, 0.49443 ),
	vec2( -0.075838, -0.529344 ),
	vec2( 0.385784, -0.393902 ),
	vec2( 0.503098, -0.308878 ),
	vec2( 0.062655, -0.611866 ),
	vec2( -0.467574, -0.405438 ),
	vec2( -0.178564, -0.596057 ),
	vec2( -0.149693, 0.605762 ),
	vec2( 0.50444, 0.372295 ),
	vec2( 0.364483, 0.511704 ),
	vec2( 0.634388, -0.049471 ),
	vec2( 0.643361, 0.164098 ),
	vec2( 0.315226, -0.604297 ),
	vec2( -0.688894, 0.007843 ),
	vec2( -0.620106, -0.328104 ),
	vec2( 0.678884, -0.204688 ),
	vec2( 0.078707, -0.715323 ),
	vec2( -0.667531, 0.32609 ),
	vec2( -0.545396, 0.538133 ),
	vec2( -0.772454, -0.090976 ),
	vec2( 0.001801, 0.780328 ),
	vec2( 0.69396, -0.366253 ),
	vec2( 0.64568, 0.49321 ),
	vec2( 0.566637, 0.605213 ),
	vec2( -0.299417, 0.791925 ),
	vec2( -0.248268, -0.814753 ),
	vec2( -0.817194, -0.271096 ),
	vec2( -0.494552, -0.711051 ),
	vec2( -0.613392, 0.617481 ),
	vec2( -0.146886, -0.859249 ),
	vec2( -0.016205, -0.872921 ),
	vec2( 0.751946, 0.453352 ),
	vec2( -0.69689, -0.549791 ),
	vec2( 0.789239, -0.419965 ),
	vec2( -0.084078, 0.898312 ),
	vec2( 0.145177, -0.898984 ),
	vec2( -0.885922, 0.215369 ),
	vec2( -0.780145, 0.486251 ),
	vec2( 0.488876, -0.783441 ),
	vec2( 0.724479, -0.580798 ),
	vec2( 0.612476, 0.705252 ),
	vec2( 0.391522, 0.849605 ),
	vec2( 0.354411, -0.88757 ),
	vec2( -0.371868, 0.882138 ),
	vec2( -0.578845, -0.768792 ),
	vec2( -0.651784, 0.717887 ),
	vec2( -0.705374, -0.668203 ),
	vec2( 0.034211, 0.97998 ),
	vec2( 0.97705, -0.108615 )
);

float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

void main() 
{
	vec3 albedo = pow(texture(texAlbedo, fragTexCoord).xyz, vec3(2.2));
	float roughness = texture(texRoughness, fragTexCoord).x;
	float metallic = texture(texMetal, fragTexCoord).x;
	float ao = texture(texAO, fragTexCoord).x;
	vec3 normal = (texture(texNormal, fragTexCoord).xyz * 2.0 - vec3(1.0)) * tbn;

	vec3 N = normalize(normal); 
    vec3 V = normalize(uboLights.camPos.xyz - worldPos);
	vec3 R = reflect(-V, N);  

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

	vec3 ambient = vec3(uboLights.ambient) * albedo;

    vec3 color = ambient + Lo;
	color = vec3(0.0);

	vec4 projCoords = posLightSpace / posLightSpace.w; 
	for(int i = 0; i < 64; i++)
	{
		int index = int(float(64)*random(worldPos.xyz, i))%64;
		vec4 randomProjCoords = projCoords + vec4(poissonDisk[i] / 30.0, 0.0, 0.0);

		vec3 indirectLightPos = texture(rsmPosition, randomProjCoords.xy).rgb;
		vec3 indirectLightNorm = texture(rsmNormal, randomProjCoords.xy).rgb;
		vec3 indirectLightFlux = texture(rsmFlux, randomProjCoords.xy).rgb;
		if(indirectLightFlux.b > 2.0 * indirectLightFlux.r){
		float dist = length(worldPos - indirectLightPos);
		
		color += indirectLightFlux;/* * 
			(max(dot(indirectLightNorm, worldPos - indirectLightPos), 0.0) * 
			max(dot(N, indirectLightPos - worldPos), 0.0);*/ 
		color = vec3(0.0, 0.0, 1.0);}
	}
	
    /*color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));*/

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