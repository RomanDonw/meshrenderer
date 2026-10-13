#include "mat4.h"

#include <math.h>

void mat4f_idt(mat4f mat)
{
    mat4f_zero(mat);
    mat[0] = mat[5] = mat[10] = mat[15] = 1;
}

void mat4f_translate(float * restrict mat, const float * restrict vec3)
{
    mat4f_idt(mat);
    mat[3] = vec3[0];
    mat[7] = vec3[1];
    mat[11] = vec3[2];
}

void mat4f_scale(float * restrict mat, const float * restrict vec3)
{
    mat4f_zero(mat);
    mat[0] = vec3[0];
    mat[5] = vec3[1];
    mat[10] = vec3[2];
    mat[15] = 1;
}

#define FLOATROTFUNCBASE(funcname, oneidx, cos1idx, cos2idx, possinidx, negsinidx) \
    void funcname(mat4f mat, float angle) \
    {\
        mat4f_zero(mat);\
        mat[15] = mat[oneidx] = 1;\
        mat[cos1idx] = mat[cos2idx] = cosf(angle);\
        float s = sinf(angle);\
        mat[possinidx] = s;\
        mat[negsinidx] = -s;\
    }

FLOATROTFUNCBASE(mat4f_rotatex, 0, 5, 10, 9, 6)
FLOATROTFUNCBASE(mat4f_rotatey, 5, 0, 10, 2, 8)
FLOATROTFUNCBASE(mat4f_rotatez, 10, 0, 5, 4, 1)

#define MULMHELP1(r, a1, a2, a3, a4, b1, b2, b3, b4) (result[r] = a[a1] * b[b1] + a[a2] * b[b2] + a[a3] * b[b3] + a[a4] * b[b4])
#define MULMHELP2(r1, r2, r3, r4) \
    MULMHELP1(r1, r1, r2, r3, r4, 0, 4, 8, 12);\
    MULMHELP1(r2, r1, r2, r3, r4, 1, 5, 9, 13);\
    MULMHELP1(r3, r1, r2, r3, r4, 2, 6, 10, 14);\
    MULMHELP1(r4, r1, r2, r3, r4, 3, 7, 11, 15);

void mat4f_mulm(float * restrict result, const float * restrict a, const float * restrict b)
{
    MULMHELP2(0, 1, 2, 3)
    MULMHELP2(4, 5, 6, 7)
    MULMHELP2(8, 9, 10, 11)
    MULMHELP2(12, 13, 14, 15)
}

void mat4f_mulm2(mat4f dst, const mat4f src)
{
    mat4f ret;
    mat4f_mulm(ret, dst, src);
    mat4f_copy(dst, ret);
}

void mat4f_mulm2r(mat4f dst, const mat4f src)
{
    mat4f ret;
    mat4f_mulm(ret, src, dst);
    mat4f_copy(dst, ret);
}

#define MULVHELP(idx, m1, m2, m3, m4) \
    dstvec4[idx] = mat[m1] * srcvec4[0] + mat[m2] * srcvec4[1] + mat[m3] * srcvec4[2] + mat[m4] * srcvec4[3];

void mat4f_mulv(float * restrict dstvec4, const mat4f mat, const float * restrict srcvec4)
{
    MULVHELP(0, 0, 1, 2, 3)
    MULVHELP(1, 4, 5, 6, 7)
    MULVHELP(2, 8, 9, 10, 11)
    MULVHELP(3, 12, 13, 14, 15)
}

void mat4f_mulv2(vec4f result, const mat4f mat)
{
    vec4f ret;
    mat4f_mulv(ret, mat, result);
    vec4f_copy(result, ret);
}

void mat4f_lookat(float * restrict mat, const float * restrict pos3, const float * restrict right3,
    const float * restrict up3, const float * restrict front3)
{
    vec3f_copy(mat, right3);
    vec3f_copy(mat + 4, up3);
    vec3f_copy(mat + 8, front3);
    vec3f_zero(mat + 12);
    
    mat[3] = -vec3f_dot(pos3, right3);
    mat[7] = -vec3f_dot(pos3, up3);
    mat[11] = -vec3f_dot(pos3, front3);
    mat[15] = 1;
}

void mat4f_perspective(mat4f mat, float halfFOVy, float aspectratio, float near, float far)
{
    mat4f_zero(mat);
    
    float tanfov = tanf(halfFOVy);
    float delta = far - near;
    mat[0] = 1 / (aspectratio * tanfov);
    mat[5] = 1 / tanfov;
    mat[10] = -(far + near) / delta;
    mat[11] = -(2 * far * near) / delta;
    mat[14] = -1;
}