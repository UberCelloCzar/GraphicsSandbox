struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D SceneTexture     	 : register(t0);
Texture2D BloomTexture		 : register(t1);

SamplerState BasicSampler	: register(s0);

static const float PI = 3.14159265359;
static const float MAX_REF_LOD = 4.0f;

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 sceneColor = SceneTexture.Sample(BasicSampler, input.uv).rgb; // Sample any and all textures
	float3 bloomColor = BloomTexture.Sample(BasicSampler, input.uv).rgb;

	float3 color = sceneColor+bloomColor; // Additive blend
	color = color / (color + float3(1.0f, 1.0f, 1.0f)); // Reinhard tone mapping
	color = pow(color, float3(0.45454545f, 0.45454545f, 0.45454545f));

	return float4(color, 1);
}