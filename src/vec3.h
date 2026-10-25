#ifndef VEC3_H
#define VEC3_H

#include <string.h>
#include <math.h>

typedef float vec3f[3];

static inline void vec3f_zero(vec3f vec) { memset(vec, 0, sizeof(vec3f)); }
static inline void vec3f_copy(float * restrict dst, const float * restrict src) { memcpy(dst, src, sizeof(vec3f)); }
static inline float vec3f_dot(const float *a, const float *b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

static inline void vec3f_add(float *dst, const float *a, const float *b)
{
    dst[0] = a[0] + b[0];
    dst[1] = a[1] + b[1];
    dst[2] = a[2] + b[2];
}

static inline void vec3f_add2(float *dst, const float *src)
{
    dst[0] += src[0];
    dst[1] += src[1];
    dst[2] += src[2];
}

static inline void vec3f_sub(float *dst, const float *a, const float *b)
{
    dst[0] = a[0] - b[0];
    dst[1] = a[1] - b[1];
    dst[2] = a[2] - b[2];
}

static inline void vec3f_sub2(float *dst, const float *src)
{
    dst[0] -= src[0];
    dst[1] -= src[1];
    dst[2] -= src[2];
}

static inline void vec3f_sub2r(float *dst, const float *src)
{
    dst[0] = src[0] - dst[0];
    dst[1] = src[1] - dst[1];
    dst[2] = src[2] - dst[2];
}

static inline void vec3f_norm(float *dst, const float *src)
{
    float len = sqrtf(src[0] * src[0] + src[1] * src[1] + src[2] * src[2]);
    dst[0] = src[0] / len;
    dst[1] = src[1] / len;
    dst[2] = src[2] / len;
}

static inline void vec3f_norm2(float *vec)
{
    float len = sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    vec[0] /= len;
    vec[1] /= len;
    vec[2] /= len;
}

static inline void vec3f_mulv(float *dst, const float *a, const float *b)
{
    dst[0] = a[0] * b[0];
    dst[1] = a[1] * b[1];
    dst[2] = a[2] * b[2];
}

static inline void vec3f_mulv2(float *dst, const float *src)
{
    dst[0] *= src[0];
    dst[1] *= src[1];
    dst[2] *= src[2];
}

static inline void vec3f_muls(float *dst, const float *src, float scalar)
{
    dst[0] = src[0] * scalar;
    dst[1] = src[1] * scalar;
    dst[2] = src[2] * scalar;
}

static inline void vec3f_muls2(float *dst, float scalar)
{
    dst[0] *= scalar;
    dst[1] *= scalar;
    dst[2] *= scalar;
}

static inline void vec3f_divv(float *dst, const float *a, const float *b)
{
    dst[0] = a[0] / b[0];
    dst[1] = a[1] / b[1];
    dst[2] = a[2] / b[2];
}

static inline void vec3f_divv2(float *dst, const float *src)
{
    dst[0] /= src[0];
    dst[1] /= src[1];
    dst[2] /= src[2];
}

static inline void vec3f_divs(float *dst, const float *src, float scalar)
{
    dst[0] = src[0] / scalar;
    dst[1] = src[1] / scalar;
    dst[2] = src[2] / scalar;
}

static inline void vec3f_divs2(float *dst, float scalar)
{
    dst[0] /= scalar;
    dst[1] /= scalar;
    dst[2] /= scalar;
}

static inline void vec3f_neg(float *dst, const float *src)
{
    dst[0] = -src[0];
    dst[1] = -src[1];
    dst[2] = -src[2];
}

static inline void vec3f_neg2(float *dst)
{
    dst[0] = -dst[0];
    dst[1] = -dst[1];
    dst[2] = -dst[2];
}

#endif