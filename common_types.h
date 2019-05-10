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

#endif