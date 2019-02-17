static const int NR_LIGHTS = 3;

struct Light
{
	float4 position;
	float4 color;
};

cbuffer PShaderConstants : register(b0)
{
	float3 CameraPosition;
	Light lights[NR_LIGHTS]; // YOUAREHEREYOUDUMBDIPSHITPIECEOFFUCK
};


struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D AlbedoMetalMap     	 : register(t0);
Texture2D NormalRoughMap		 : register(t1);
Texture2D WorldPosAOMap        : register(t2);
Texture2D BRDFLookup		 : register(t3);
TextureCube EnvIrradianceMap : register(t4);
TextureCube EnvPrefilterMap	 : register(t5);

SamplerState BasicSampler	: register(s0);

static const float PI = 3.14159265359;
static const float MAX_REF_LOD = 4.0f;

float NormalDistributionGGXTR(float3 normal, float3 halfway, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(normal, halfway), 0.0);
	NdotH = NdotH * NdotH;

	float denom = (NdotH * (a2 - 1.0f) + 1.0f);

	return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = roughness + 1.0f;
	float k = (r * r) / 8.0f;

	return NdotV / (NdotV * (1.0f - k) + k);
}

float GeometrySmith(float3 normal, float3 view, float3 light, float k)
{
	float NdotV = max(dot(normal, view), 0.0f);
	float NdotL = max(dot(normal, light), 0.0f);
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)   // cosTheta is n.v and F0 is the base reflectivity
{
	return (F0 + (1.f - F0) * pow(1.f - cosTheta, 5.f));
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)   // cosTheta is n.v and F0 is the base reflectivity
{
	return F0 + (max(float3(1.f - roughness, 1.f - roughness, 1.f - roughness), F0) - F0) * pow(1.f - cosTheta, 5.f);
}

void CalculateRadiance(float3 worldPos, float3 view, float3 normal, float3 albedo, float roughness, float metalness, int l, float3 F0, out float3 rad)
{

	float3 light = lights[l].position.xyz - worldPos; // Get the base radiance
	float distance = length(light);
	light = normalize(light);
	float3 halfway = normalize(view + light);
	float attenuation = 1.0f / (distance * distance);
	float3 radiance = lights[l].color.rgb * attenuation * PI *300;

	//Cook-Torrance BRDF
	float3 F = FresnelSchlick(max(dot(halfway, view), 0.0f), F0);
	float D = NormalDistributionGGXTR(normal, halfway, roughness);
	float G = GeometrySmith(normal, view, light, roughness);

	float denom = 4.0f * max(dot(normal, view), 0.0f) * max(dot(normal, light), 0.0);
	float3 specular = (D * G * F) / max(denom, .001);

	float3 kD = float3(1.0f, 1.0f, 1.0f) - F; // F = kS
	kD *= 1.0 - metalness;


	//Add to outgoing radiance Lo
	float NdotL = max(dot(normal, light), 0.0f);
	rad = (((kD * albedo / PI) + specular) * radiance * NdotL);
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 albedo = pow(AlbedoMetalMap.Sample(BasicSampler, input.uv).rgb, 2.2); // Sample any and all textures
	float metalness = AlbedoMetalMap.Sample(BasicSampler, input.uv).w; // Metallic
	float3 normal = NormalRoughMap.Sample(BasicSampler, input.uv).xyz; // Normal has already been adjusted
	float roughness = NormalRoughMap.Sample(BasicSampler, input.uv).w; // Rough
	float3 worldPos = WorldPosAOMap.Sample(BasicSampler, input.uv).xyz; // World position
	float ao = WorldPosAOMap.Sample(BasicSampler, input.uv).w; // AO

	float3 view = normalize(CameraPosition.xyz - worldPos); // View vector
	float3 reflection = reflect(-view, normal); // Reflection vector

	float3 F0 = float3(0.04f, 0.04f, 0.04f); // Approximates a base dielectric reflectivity
	F0 = lerp(F0, albedo, metalness); // The more metallic a surface is, the more its surface color is just full reflection

	float3 radiance = float3(0.0f, 0.0f, 0.0f);
	float3 Lo = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < NR_LIGHTS; i++)
	{
		CalculateRadiance(worldPos, view, normal, albedo, roughness, metalness, i, F0, radiance);
		Lo += radiance;
	}

	float3 kS = FresnelSchlickRoughness(max(dot(normal, view), 0.f), F0, roughness)*.8f;
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metalness;
	float3 diffuse = albedo * EnvIrradianceMap.Sample(BasicSampler, normal).rgb;

	float3 prefilteredColor = EnvPrefilterMap.SampleLevel(BasicSampler, reflection, roughness * MAX_REF_LOD).rgb;
	float2 brdf = BRDFLookup.Sample(BasicSampler, float2(max(dot(normal, view), 0.0f), roughness)).rg;
	float3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

	float3 color = ((kD * diffuse + specular) * ao) + Lo; // Ambient + Lo
	color = color / (color + float3(1.0f, 1.0f, 1.0f));
	color = pow(color, float3(0.45454545f, 0.45454545f, 0.45454545f));

	return float4(color, 1);
}