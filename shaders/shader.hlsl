float4 VS(float2 inPos : POSITION) : SV_POSITION 
{
	return float4(inPos, 0.0f, 1.0f);
}

float4 PS() : SV_TARGET 
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}