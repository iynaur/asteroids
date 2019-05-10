cbuffer vs_constant_buffer
{
	float ar;
	float scale;
	float2 pos;
};

float4 VS(float2 inPos : POSITION) : SV_POSITION 
{
	float2 tpos = inPos - pos;
	float scaleFactor = (1 / scale);
	float2x2 aspectRatioMatrix = {
		ar * scaleFactor, 0.0f, 
		0.0f, scaleFactor, 
	};
	float2 transformedPos = mul(tpos, aspectRatioMatrix);
	return float4(transformedPos, 0.0f, 1.0f);
}

float4 PS() : SV_TARGET 
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}