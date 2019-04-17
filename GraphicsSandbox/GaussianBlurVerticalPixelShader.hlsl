#define SAMPLES 10
#define PI 3.14159265359
#define E 2.71828182846

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D BlurInputTexture     	 : register(t0);

SamplerState BasicSampler	: register(s0);

static const float lhand = (1 / sqrt(2 * PI * (.02*.02)));

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 color = float4(0.f, 0.f, 0.f, 0.f);
	float sum = 0;
	float2 uv;
	float offset;
	float gauss;

	for (float index = 0; index < SAMPLES; index++)
	{
		offset = ((index / (SAMPLES - 1.f)) - .5f)*.004f; // Blur size = .07
		uv = input.uv + float2(0.f, offset);
		gauss = lhand * pow(E, -((offset * offset) / (2 * (.02*.02)))); // M a t h (stdDev = .02)

		sum += gauss;
		color += gauss * BlurInputTexture.Sample(BasicSampler, uv);
	}

	color = color / sum; // Divide by num samples
	return color;
}