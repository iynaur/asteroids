cbuffer vs_constant_buffer
{
	float ar;
	float scale;
};

float4 VS(float2 inPos : POSITION) : SV_POSITION 
{
	float2 pos = inPos * (1 / scale);
	float2x2 aspectRatioMatrix = {
		ar, 0.0f, 
		0.0f, 1.0f, 
	};
	float2 transformedPos = mul(pos, aspectRatioMatrix);
	return float4(transformedPos, 0.0f, 1.0f);
}

float4 PS() : SV_TARGET 
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}