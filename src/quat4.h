#ifndef QUAT4_H
#define QUAT4_H

#include <string.h>
#include <math.h>

#include "vec3.h"

typedef float quat4f[4]; // [s, i, j, k]

static inline void quat4f_zero(quat4f q) { memset(q, 0, sizeof(quat4f)); }
static inline void quat4f_copy(float * restrict dst, const float * restrict src) { memcpy(dst, src, sizeof(quat4f)); }
static inline float quat4f_dot(const float *a, const float *b)
{ return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]; }

static inline float quat4f_len(const float *quat)
{ return sqrtf(quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2] + quat[3] * quat[3]); }

static inline void quat4f_add(float *out, const float *a, const float *b)
{
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
    out[2] = a[2] + b[2];
    out[3] = a[3] + b[3];
}

static inline void quat4f_add2(float *out, const float *in)
{
    out[0] += in[0];
    out[1] += in[1];
    out[2] += in[2];
    out[3] += in[3];
}

static inline void quat4f_sub(float *out, const float *a, const float *b)
{
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
    out[2] = a[2] - b[2];
    out[3] = a[3] - b[3];
}

static inline void quat4f_sub2(float *out, const float *in)
{
    out[0] -= in[0];
    out[1] -= in[1];
    out[2] -= in[2];
    out[3] -= in[3];
}

static inline void quat4f_sub2r(float *out, const float *in)
{
    out[0] = in[0] - out[0];
    out[1] = in[1] - out[1];
    out[2] = in[2] - out[2];
    out[3] = in[3] - out[3];
}

static inline void quat4f_muls(float *out, const float *in, float scalar)
{
    out[0] = in[0] * scalar;
    out[1] = in[1] * scalar;
    out[2] = in[2] * scalar;
    out[3] = in[3] * scalar;
}

static inline void quat4f_muls2(float *out, float scalar)
{
    out[0] *= scalar;
    out[1] *= scalar;
    out[2] *= scalar;
    out[3] *= scalar;
}

static inline void quat4f_divs(float *out, const float *in, float scalar)
{
    out[0] = in[0] / scalar;
    out[1] = in[1] / scalar;
    out[2] = in[2] / scalar;
    out[3] = in[3] / scalar;
}

static inline void quat4f_divs2(float *out, float scalar)
{
    out[0] /= scalar;
    out[1] /= scalar;
    out[2] /= scalar;
    out[3] /= scalar;
}

static inline void quat4f_divs2r(float *out, float scalar)
{
    out[0] = scalar / out[0];
    out[1] = scalar / out[1];
    out[2] = scalar / out[2];
    out[3] = scalar / out[3];
}

static inline void quat4f_mulq(float * restrict out, const float * restrict a, const float * restrict b)
{
    out[0] = a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3];
    out[1] = a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2];
    out[2] = a[0] * b[2] + a[2] * b[0] + a[3] * b[1] - a[1] * b[3];
    out[3] = a[0] * b[3] + a[3] * b[0] + a[1] * b[2] - a[2] * b[1];
}

static inline void quat4f_mulq2(float *out, const float *in)
{
    quat4f ret;
    quat4f_mulq(ret, out, in);
    quat4f_copy(out, ret);
}

static inline void quat4f_mulq2r(float *out, const float *in)
{
    quat4f ret;
    quat4f_mulq(ret, in, out);
    quat4f_copy(out, ret);
}

static inline void quat4f_conj(float *out, const float *in)
{
    out[0] = in[0];
    vec3f_neg(out + 1, in + 1);
}

static inline void quat4f_conj2(float *out) { vec3f_neg2(out + 1); }

static inline void quat4f_norm(float *out, const float *in)
{
    float l = quat4f_len(in);
    out[0] = in[0] / l;
    out[1] = in[1] / l;
    out[2] = in[2] / l;
    out[3] = in[3] / l;
}

static inline void quat4f_norm2(float *out)
{
    float l = quat4f_len(out);
    out[0] = out[0] / l;
    out[1] = out[1] / l;
    out[2] = out[2] / l;
    out[3] = out[3] / l;
}

static inline void quat4f_rotate(quat4f out, const vec3f axis, float angle)
{
    out[0] = cos(angle * 0.5);
    vec3f_muls(out + 1, axis, sinf(angle * 0.5));
}

static inline void quat4f_inv(float *out, const float *in)
{
    float lsqr = in[0] * in[0] + in[1] * in[1] + in[2] * in[2] + in[3] * in[3];
    quat4f_conj(out, in);
    quat4f_divs2(out, lsqr);
}

static inline void quat4f_inv2(float *out)
{
    float lsqr = out[0] * out[0] + out[1] * out[1] + out[2] * out[2] + out[3] * out[3];
    quat4f_conj2(out);
    quat4f_divs2(out, lsqr);
}

#endif