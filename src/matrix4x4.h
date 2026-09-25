#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#include <string.h>

#include "vec3.h"
#include "vec4.h"

#define MAT4IDX(col, row) (row * 4 + col)
typedef float mat4f[16];
typedef double mat4d[16];

void mat4f_mulm(mat4f result, const mat4f a, const mat4f b);

static inline void mat4f_zero(mat4f mat) { memset(mat, 0, sizeof(mat4f)); }
void mat4f_idt(mat4f mat);

void mat4f_translate(mat4f mat, const vec3f vec);
void mat4f_rotatex(mat4f mat, float angle);
void mat4f_rotatey(mat4f mat, float angle);
void mat4f_rotatez(mat4f mat, float angle);
void mat4f_scale(mat4f mat, const vec3f vec);

void mat4f_mulm2(mat4f dst, const mat4f src);
void mat4f_mulv(vec4f result, const mat4f mat, const vec4f vec);

#endif