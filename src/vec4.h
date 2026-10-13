#ifndef VEC4_H
#define VEC4_H

#include <string.h>

typedef float vec4f[3];

static inline void vec4f_copy(float * restrict dst, const float * restrict src) { memcpy(dst, src, sizeof(vec4f)); }

#endif