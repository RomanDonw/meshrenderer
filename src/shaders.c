#include "shaders.h"

const shadersourcepair modelshaders =
{
    .vertex = ""
    "#version 330 core\n"

    "layout (location = 0) in vec3 vpos;\n"
    "layout (location = 1) in vec2 vuv;\n"

    "out vec2 uv;\n"

    "uniform mat4 proj;\n"
    "uniform mat4 view;\n"

    "uniform bool downscale;\n"

    "void main(void)\n"
    "{"
        "uv = vuv;\n"
        "gl_Position = proj * view * vec4(downscale ? vpos * vec3(0.0254) : vpos, 1);\n"
    "}",

    // ========================================

    .fragment = ""
    "#version 330 core\n"

    "in vec2 uv;\n"
    "out vec4 FragColor;\n"

    "uniform sampler2D albedo;\n"
    "uniform sampler2D noisetex;"
    
    "uniform bool flipv;\n"

    "#define LUMA_R 0.299\n"
    "#define LUMA_G 0.587\n"
    "#define LUMA_B 0.114\n"

    "#define COEFF_U 0.492\n"
    "#define COEFF_V 0.877\n"

    "void main(void)\n"
    "{\n"
        "vec4 pix = texture(albedo, vec2(uv.x, flipv ? 1 - fract(uv.y) : uv.y));\n"
        
        "float cy = dot(pix.rgb, vec3(LUMA_R, LUMA_G, LUMA_B));\n"
        "float cu = COEFF_U * (pix.b - cy);\n"
        "float cv = COEFF_V * (pix.r - cy);\n"

        "float n = texture(noisetex, fract(uv) * (1.0 / 3.0)).r * 2 - 1;\n"
        "float n2 = texture(noisetex, fract(uv) * (1.0 / 3.0) + (1.0 / 3.0)).r * 2 - 1;\n"
        "float n3 = texture(noisetex, fract(uv) * (1.0 / 3.0) + (2.0 / 3.0)).r * 2 - 1;\n"
        "cy += n * 0.1;\n"
        "cu += n2 * 0.07;\n"
        "cv += n3 * 0.07;\n"

        "float tb = cu / COEFF_U + cy;\n"
        "float tr = cv / COEFF_V + cy;\n"
        "float tg = (cy - LUMA_R * tr - LUMA_B * tb) / LUMA_G;\n"

        "FragColor = vec4(tr, tg, tb, pix.a);\n"
    "}"
};

const shadersourcepair screenshaders =
{
    .vertex = ""
    "#version 330 core\n"
    
    "layout (location = 0) in vec2 pos;\n"

    "out vec2 uv;\n"

    "void main(void)\n"
    "{\n"
        "gl_Position = vec4(pos, 0, 1);\n"
        "uv = (pos + vec2(1)) * vec2(0.5);\n"
    "}",

    // ========================================

    .fragment = ""
    "#version 330 core\n"

    "in vec2 uv;"
    "out vec4 FragmentColor;\n"

    "uniform sampler2D screen;\n"
    
    "void main(void)\n"
    "{\n"
        "vec4 f = texture(screen, uv);\n"
        "FragmentColor = f * vec4(0.3);\n"
    "}"
};