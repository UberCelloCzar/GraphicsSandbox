cbuffer SkyboxVSConstants : register(b0)
{
	matrix view;
	matrix projection;
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
	float3 uvw			: TEXCOORD0;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	matrix viewNoMovement = view;
	viewNoMovement._41 = 0;
	viewNoMovement._42 = 0;
	viewNoMovement._43 = 0;

	matrix viewproj = mul(viewNoMovement, projection);
	output.position = mul(float4(input.position, 1.0f), viewproj).xyww; // Ensure the vertex's depth is going to be 1.0 exactly

	output.uvw = input.position;

	return output;
}