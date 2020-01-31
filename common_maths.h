#ifndef COMMON_MATHS_H
#define COMMON_MATHS_H

#define PI (float)3.14159265358979323846  
#define PI_OVER_TWO (float)1.57079632679489661923

#pragma pack(push, 1)
struct vec2 {
	union {
		struct {
			float x;
			float y;
		};
		struct {
			float width;
			float height;
		};
	};
};
#pragma pack(pop)

typedef vec2 rect;

inline vec2 operator*(vec2 a, float b)
{
	vec2 result;
	result.x = a.x * b;
	result.y = a.y * b;
	return(result);
}

inline vec2 operator-(vec2 a, vec2 b)
{
	vec2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return(result);
}

inline vec2 operator-(vec2 a, float b)
{
	vec2 result;
	result.x = a.x - b;
	result.y = a.y - b;
	return(result);
}

inline vec2 operator+(vec2 a, float b)
{
	vec2 result;
	result.x = a.x + b;
	result.y = a.y + b;
	return(result);
}

inline vec2 operator+(vec2 a, vec2 b)
{
	vec2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return(result);
}

inline vec2 operator*(vec2 a, vec2 b)
{
	vec2 result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	return(result);
}

inline vec2 operator/(vec2 a, float b)
{
	vec2 result;
	result.x = a.x / b;
	result.y = a.y / b;
	return(result);
}

internal vec2 RadianToVectorSlow(float r)
{
	vec2 result = {cosf(PI_OVER_TWO-r), sinf(PI_OVER_TWO-r)};
	return(result);
}

internal float V2Mag(vec2 a)
{
	float result;
	result = sqrtf((a.x*a.x) + (a.y*a.y));
	return(result);
}

internal vec2 V2Normalise(vec2 a)
{
	vec2 result = {};
	if (a.x && a.y) {
		result = a / V2Mag(a);
	}
	return(result);
}

internal vec2 MoveAtAngle(vec2 pos, float rot, float speed)
{
	vec2 result = pos - (RadianToVectorSlow(rot) * speed);
	return(result);
}

inline float fClamp(float val, float lower, float upper)
{
	float result = val;
	if (val > upper) result = upper;
	else if (val < lower) result = lower;
	return(result);
}

inline vec2 RadToVec2(float r)
{
	vec2 result;
	result.x = cosf(PI_OVER_TWO - r);
	result.y = sinf(PI_OVER_TWO - r);
	return(result);
}

#ifndef WIN32_D3D_CPP
#include "win32_d3d.hpp"
#endif

internal u32 RandomNumber(void)
{
	persist u32 seed;
	if (!seed) seed = (u32)Win32GetTicks();
	return((seed = (seed * 0x41C64E6D + 12345)));
}

internal float RandomFloat(void)
{
	float result = 0.0f;
	u32 randInt = (u32)RandomNumber() % 256;
	result = (float)randInt / 256.0f;
	return(result);
}

#endif
