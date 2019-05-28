cbuffer vs_constant_buffer
{
	float2 pos;
	float2 cameraPos;
	float2 distort;
	float2 localOffset;
	float ar;
	float scale;
	float r;
};

float4 VS(float2 inPos : POSITION) : SV_POSITION 
{
	float2x2 rotTransform = {
		cos(r), -sin(r),
		sin(r), cos(r)
	};
	float2 p = inPos;
 p += localOffset;
	p *= distort;
	p = mul(p, rotTransform) - (pos - cameraPos);
	
	float sf = 1/scale;
	float2x2 aspectTransform = {
		ar*sf, 0,
		0, sf
	};
	float2 finalPos = mul(aspectTransform, p);
	return float4(finalPos, 0.0f, 1.0f);
}

float4 PS() : SV_TARGET 
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}