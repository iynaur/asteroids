cbuffer vs_constant_buffer
{
	float2 pos;
	float ar;
	float scale;
	float r;
};

float4 VS(float2 inPos : POSITION) : SV_POSITION 
{
	float2x2 rotMatrix = {
		cos(r), -sin(r),
		sin(r), cos(r)
	};
	
	float2 tpos = mul(inPos, rotMatrix) - pos;
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