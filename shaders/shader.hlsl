cbuffer vs_constant_buffer
{
	float4x4 aspectRatioMatrix;
};

float4 VS(float4 inPos : POSITION) : SV_POSITION 
{
	float4 p = mul(inPos, aspectRatioMatrix);
	return p;
}

float4 PS() : SV_TARGET 
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}