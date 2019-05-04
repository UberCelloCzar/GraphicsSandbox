cbuffer VShaderConstants : register(b0)
{
	column_major matrix projViewWorld;
	column_major matrix world;
	column_major matrix projectorView1;
	column_major matrix projectorProj1;
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
	float2 uv			: TEXCOORD0;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float4 viewPosition : TEXCOORD1;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	output.position = mul(float4(input.position, 1.0f), projViewWorld);
	output.uv = input.uv;

	output.normal = normalize(mul(input.normal, (float3x3)world));
	output.tangent = normalize(mul(input.tangent, (float3x3)world));
	output.worldPos = mul(float4(input.position, 1.f), world).xyz;

	output.viewPosition = mul(mul(mul(float4(input.position, 1.0f), world), projectorView1), projectorProj1);

	return output;
}