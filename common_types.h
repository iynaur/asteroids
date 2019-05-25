#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#define internal static
#define global static
#define persist static

typedef int unsigned u32;
typedef u32 b32;
typedef long unsigned u64;

#define true (b32)1
#define false (b32)0

internal float RandomFloat(void)
{
	float result = 0.0f;
	u32 randInt = (u32)rand() % 256;
	result = (float)randInt / 256.0f;
	return(result);
}

#endif