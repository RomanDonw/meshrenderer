#ifndef QUAT4_H
#define QUAT4_H

#include <string.h>
#include <math.h>

#include "vec3.h"
#include "vec4.h"

typedef float quat4f[4]; // [s, i, j, k]

static inline void quat4f_zero(quat4f q) { memset(q, 0, sizeof(quat4f)); }
static inline void quat4f_copy(float * restrict dst, const float * restrict src) { memcpy(dst, src, sizeof(quat4f)); }
static inline float quat4f_len(const float *quat) { return vec4f_len(quat); }
static inline float quat4f_dot(const float *a, const float *b) { return vec4f_dot(a, b); }

static inline void quat4f_add(float *out, const float *a, const float *b) { vec4f_add(out, a, b); }
static inline void quat4f_add2(float *out, const float *in) { vec4f_add2(out, in); }
static inline void quat4f_sub(float *out, const float *a, const float *b) { vec4f_sub(out, a, b); }
static inline void quat4f_sub2(float *out, const float *in) { vec4f_sub2(out, in); }
static inline void quat4f_sub2r(float *out, const float *in) { vec4f_sub2r(out, in); }
static inline void quat4f_muls(float *out, const float *in, float scalar) { vec4f_muls(out, in, scalar); }
static inline void quat4f_muls2(float *out, float scalar) { vec4f_muls2(out, scalar); }
static inline void quat4f_divs(float *out, const float *in, float scalar) { vec4f_divs(out, in, scalar); }
static inline void quat4f_divs2(float *out, float scalar) { vec4f_divs2(out, scalar); }
static inline void quat4f_divs2r(float *out, float scalar) { vec4f_divs2r(out, scalar); }
static inline void quat4f_norm(float *out, const float *in) { vec4f_norm(out, in); }
static inline void quat4f_norm2(float *out) { vec4f_norm2(out); }

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

static inline void quat4f_mulv(float * restrict outvec3, const float * restrict quat, const float * restrict invec3)
{
    vec3f t;
    vec3f_cross(t, quat + 1, invec3);
    vec3f_muls2(t, 2);

    vec3f_cross(outvec3, quat + 1, t);
    vec3f_muls2(t, *quat);
    vec3f_add2(outvec3, t);
    
    vec3f_add2(outvec3, invec3);
}

static inline void quat4f_mulv2(float *vec3, const float *quat)
{
    vec3f ret;
    quat4f_mulv(ret, quat, vec3);
    vec3f_copy(vec3, quat);
}

// suffixes: t - translate, r - rotate, s - scale.
// all this functions assumes that quaternion was normalized.
    
#define QUAT4_TOMAT_BASE(scalex, scaley, scalez) \
    float i2 = quat[1] * quat[1];\
    float j2 = quat[2] * quat[2];\
    float k2 = quat[3] * quat[3];\
    \
    float ij = quat[1] * quat[2];\
    float jk = quat[2] * quat[3];\
    float ki = quat[3] * quat[1];\
    \
    float si = quat[0] * quat[1];\
    float sj = quat[0] * quat[2];\
    float sk = quat[0] * quat[3];\
    \
    vec3f_zero(mat + 12);\
    mat[15] = 1;\
    \
    mat[0] = 1 - 2 * (j2 + k2) * scalex;\
    mat[1] = 2 * (ij + sk) * scalex;\
    mat[2] = 2 * (ki - sj) * scalex;\
    \
    mat[4] = 2 * (ij - sk) * scaley;\
    mat[5] = 1 - 2 * (i2 + k2) * scaley;\
    mat[6] = 2 * (jk + si) * scaley;\
    \
    mat[8] = 2 * (ki + sj) * scalez;\
    mat[9] = 2 * (jk - si) * scalez;\
    mat[10] = 1 - 2 * (i2 + j2) * scalez

static inline void quat4f_toRmat(float * restrict mat, const float * restrict quat)
{
    QUAT4_TOMAT_BASE(1, 1, 1);
    mat[3] = mat[7] = mat[11] = 0;
}

static inline void quat4f_toRSmat(float * restrict mat, const float * restrict quat, const float * restrict scale)
{
    QUAT4_TOMAT_BASE(scale[0], scale[1], scale[2]);
    mat[3] = mat[7] = mat[11] = 0;
}

static inline void quat4f_toTRmat(float * restrict mat, const float * restrict translate, const float * restrict quat)
{
    QUAT4_TOMAT_BASE(1, 1, 1);
    mat[3] = translate[0];
    mat[7] = translate[1];
    mat[11] = translate[2];
}

static inline void quat4f_toTRSmat(float * restrict mat, const float * restrict translate, const float * restrict quat, const float * restrict scale)
{
    QUAT4_TOMAT_BASE(scale[0], scale[1], scale[2]);
    mat[3] = translate[0];
    mat[7] = translate[1];
    mat[11] = translate[2];
}

#undef QUAT4_TOMAT_BASE

#endif