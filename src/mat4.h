#ifndef MAT4_H
#define MAT4_H

#include <string.h>

#include "vec3.h"
#include "vec4.h"

typedef float mat4f[16];

static inline void mat4f_zero(mat4f mat) { memset(mat, 0, sizeof(mat4f)); }
static inline void mat4f_copy(float * restrict dst, const float * restrict src) { memcpy(dst, src, sizeof(mat4f)); }
void mat4f_idt(mat4f mat);

void mat4f_translate(float * restrict mat, const float * restrict vec3);
void mat4f_rotatex(mat4f mat, float angle);
void mat4f_rotatey(mat4f mat, float angle);
void mat4f_rotatez(mat4f mat, float angle);
void mat4f_scale(float * restrict mat, const float * restrict vec3);

void mat4f_mulm(float * restrict result, const float * restrict a, const float * restrict b);
void mat4f_mulm2(mat4f dst, const mat4f src); // dst = dst * src
void mat4f_mulm2r(mat4f dst, const mat4f src); // dst = src * dst
void mat4f_mulv(float * restrict dstvec4, const mat4f mat, const float * restrict srcvec4);
void mat4f_mulv2(vec4f result, const mat4f mat);

void mat4f_lookat(float * restrict mat, const float * restrict pos3, const float * restrict right3,
    const float * restrict up3, const float * restrict front3);
void mat4f_perspective(mat4f mat, float halfFOVy, float aspectratio, float near, float far);

#endif