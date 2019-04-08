struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

float4 main(VertexToPixel input) : SV_TARGET
{

	float depth = input.position.z / input.position.w;
	float dx = ddx(depth);
	float dy = ddy(depth);

	return float4(depth, (depth*depth) + .25f*(dx*dx + dy*dy), 0.0f, 0.0f);
}