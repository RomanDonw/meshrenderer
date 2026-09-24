#include "matrix4x4.h"

#include <math.h>

void mat4f_idt(mat4f mat)
{
    mat4f_zero(mat);
    mat[MAT4IDX(0, 0)] = mat[MAT4IDX(1, 1)] = mat[MAT4IDX(2, 2)] = mat[MAT4IDX(3, 3)] = 1;
}

void mat4f_translate(mat4f mat, const vec3f vec)
{
    mat4f_idt(mat);
    mat[MAT4IDX(3, 0)] = vec[0];
    mat[MAT4IDX(3, 1)] = vec[1];
    mat[MAT4IDX(3, 2)] = vec[2];
}

void mat4f_scale(mat4f mat, const vec3f vec)
{
    mat[MAT4IDX(0, 0)] = vec[0];
    mat[MAT4IDX(1, 1)] = vec[1];
    mat[MAT4IDX(2, 2)] = vec[2];
    mat[MAT4IDX(3, 3)] = 1;
}

#define FLOATROTFUNCBASE(funcname, onecellx, onecelly, coscell1x, coscell1y, coscell2x, coscell2y, sincellx, sincelly, negsincellx, negsincelly) \
    void funcname(mat4f mat, float angle) \
    {\
        mat4f_zero(mat);\
        mat[MAT4IDX(3, 3)] = mat[MAT4IDX(onecellx, onecelly)] = 1;\
        mat[MAT4IDX(coscell1x, coscell1y)] = cosf(angle);\
        mat[MAT4IDX(coscell2x, coscell2y)] = mat[MAT4IDX(coscell1x, coscell1y)];\
        mat[MAT4IDX(sincellx, sincelly)] = sinf(angle);\
        mat[MAT4IDX(negsincellx, negsincelly)] = -mat[MAT4IDX(sincellx, sincelly)];\
    }

FLOATROTFUNCBASE(mat4f_rotatex, 0, 0, 1, 1, 2, 2, 1, 2, 2, 1)
FLOATROTFUNCBASE(mat4f_rotatey, 1, 1, 0, 0, 2, 2, 2, 1, 1, 2)
FLOATROTFUNCBASE(mat4f_rotatez, 2, 2, 0, 0, 1, 1, 0, 1, 1, 0)

#define MULELHELP1(r, a1, a2, a3, a4, b1, b2, b3, b4) (result[r] = a[a1] * b[b1] + a[a2] * b[b2] + a[a3] * b[b3] + a[a4] * b[b4])
#define MULELHELP2(r1, r2, r3, r4) \
    MULELHELP1(r1, r1, r2, r3, r4, MAT4IDX());

void mat4f_mulm(mat4f result, const mat4f a, const mat4f b)
{
    
}
void mat4f_mulv(vec4f result, const mat4f mat, const vec4f vec);
