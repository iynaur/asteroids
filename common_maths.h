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

internal vec2 RadianToVectorSlow(float r)
{
	vec2 result = {cosf(PI_OVER_TWO-r), sinf(PI_OVER_TWO-r)};
	return(result);
}

internal vec2 MoveAtAngle(vec2 pos, float rot, float speed)
{
	vec2 r = RadianToVectorSlow(rot);
	vec2 result = pos - (r * speed);
	return(result);
}

#endif
