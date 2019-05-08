cbuffer vs_constant_buffer
{
	float ar;
};

float4 VS(float2 inPos : POSITION) : SV_POSITION 
{
	float2x2 aspectRatioMatrix = {
		ar, 0.0f, 
		0.0f, 1.0f, 
	};
	float2 p = mul(inPos, aspectRatioMatrix);
	return float4(p, 0.0f, 1.0f);
}

float4 PS() : SV_TARGET 
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}