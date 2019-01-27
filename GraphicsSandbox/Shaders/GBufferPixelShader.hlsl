struct PSOutput
{
	float4 color : SV_TARGET0; // Albedo and metalness
	float4 normals : SV_TARGET1; // Normals and roughness
	float4 worldPos : SV_TARGET2; // World pos and ambient occlusion
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
};

Texture2D AlbedoMap     	 : register(t0);
Texture2D NormalMap			 : register(t1);
Texture2D MetallicMap        : register(t2);
Texture2D RoughnessMap       : register(t3);
Texture2D AOMap              : register(t4);

SamplerState BasicSampler	: register(s0);

PSOutput main(VertexToPixel input)
{
	PSOutput output;
	output.color = float4(AlbedoMap.Sample(BasicSampler, input.uv).rgb, MetallicMap.Sample(BasicSampler, input.uv).r); // Sample albedo and metalness
	output.worldPos = float4(input.worldPos, AOMap.Sample(BasicSampler, input.uv).r); // World pos and sampled ambient occlusion

	float3 normalFromTexture = NormalMap.Sample(BasicSampler, input.uv).xyz * 2 - 1; // Sample and unpack normal

	input.normal = normalize(input.normal); // Re-normalize any interpolated values
	input.tangent = normalize(input.tangent);

	// Create the TBN matrix which allows us to go from TANGENT space to WORLD space
	float3 T = normalize(input.tangent - input.normal * dot(input.tangent, input.normal)); // Adjust tangent to be orthogonal if normal isn't already
	float3 B = cross(T, input.normal);
	float3x3 TBN = float3x3(T, B, input.normal);
	output.normals = float4(normalize(mul(normalFromTexture, TBN)), RoughnessMap.Sample(BasicSampler, input.uv).r); // Adjusted normals and sampled roughness
	
	return output;
}