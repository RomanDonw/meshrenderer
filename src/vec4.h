#ifndef VEC4_H
#define VEC4_H

#include <string.h>

typedef float vec4f[4];

static inline void vec4f_copy(float * restrict dst, const float * restrict src) { memcpy(dst, src, sizeof(vec4f)); }
static inline float vec4f_len(const vec4f vec) { return sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]); }
static inline float vec4f_dot(const vec4f a, const vec4f b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]; }

static inline void vec4f_add(float *out, const float *a, const float *b)
{
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
    out[2] = a[2] + b[2];
    out[3] = a[3] + b[3];
}

static inline void vec4f_add2(float *out, const float *in)
{
    out[0] += in[0];
    out[1] += in[1];
    out[2] += in[2];
    out[3] += in[3];
}

static inline void vec4f_sub(float *out, const float *a, const float *b)
{
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
    out[2] = a[2] - b[2];
    out[3] = a[3] - b[3];
}

static inline void vec4f_sub2(float *out, const float *in)
{
    out[0] -= in[0];
    out[1] -= in[1];
    out[2] -= in[2];
    out[3] -= in[3];
}

static inline void vec4f_sub2r(float *out, const float *in)
{
    out[0] = in[0] - out[0];
    out[1] = in[1] - out[1];
    out[2] = in[2] - out[2];
    out[3] = in[3] - out[3];
}

static inline void vec4f_muls(float *out, const float *in, float scalar)
{
    out[0] = in[0] * scalar;
    out[1] = in[1] * scalar;
    out[2] = in[2] * scalar;
    out[3] = in[3] * scalar;
}

static inline void vec4f_muls2(float *out, float scalar)
{
    out[0] *= scalar;
    out[1] *= scalar;
    out[2] *= scalar;
    out[3] *= scalar;
}

static inline void vec4f_divs(float *out, const float *in, float scalar)
{
    out[0] = in[0] / scalar;
    out[1] = in[1] / scalar;
    out[2] = in[2] / scalar;
    out[3] = in[3] / scalar;
}

static inline void vec4f_divs2(float *out, float scalar)
{
    out[0] /= scalar;
    out[1] /= scalar;
    out[2] /= scalar;
    out[3] /= scalar;
}

static inline void vec4f_divs2r(float *out, float scalar)
{
    out[0] = scalar / out[0];
    out[1] = scalar / out[1];
    out[2] = scalar / out[2];
    out[3] = scalar / out[3];
}

#endif