cbuffer vs_constant_buffer
{
	float2 pos;
	float ar;
	float scale;
	float r;
	float2 distort;
	float2 localOffset;
};

float4 VS(float2 inPos : POSITION) : SV_POSITION 
{
	float2 tpos = inPos;
	
	tpos.x += localOffset.y; //TODO: why does x correspond to y in local offset
	tpos.y += localOffset.x;
	
	tpos.x *= distort.x;
	tpos.y *= distort.y;
	
	float2x2 rotMatrix = {
		cos(r), -sin(r),
		sin(r), cos(r)
	};
	
	tpos = mul(tpos, rotMatrix) - pos;
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