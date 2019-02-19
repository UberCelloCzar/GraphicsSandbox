cbuffer VShaderConstants : register(b0)
{
	column_major matrix projViewWorld;
	column_major matrix world;
	column_major matrix shadowProjViewWorld;
};

struct VertexShaderInput
{
	float3 position		: POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float2 uv			: TEXCOORD;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	output.position = mul(float4(input.position, 1.0f), shadowProjViewWorld);

	return output;
}