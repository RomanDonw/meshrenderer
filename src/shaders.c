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
    "out vec4 FragmentColor;\n"

    "uniform sampler2D albedo;\n"

    "uniform bool flipv;\n"

    "void main(void)\n"
    "{\n"
        "FragmentColor = texture(albedo, vec2(uv.x, flipv ? 1 - fract(uv.y) : uv.y));\n"
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
    "uniform sampler2D noise;\n"

    "#define LUMA_R 0.299\n"
    "#define LUMA_G 0.587\n"
    "#define LUMA_B 0.114\n"

    "#define COEFF_U 0.492\n"
    "#define COEFF_V 0.877\n"

    "#define CH_DELTAB_MIN -0.436\n"
    "#define CH_DELTAB_MAX 0.436\n"

    "#define CH_DELTAR_MIN -0.615\n"
    "#define CH_DELTAR_MAX 0.615\n"

    "uniform float time;\n"

    "#define PI 3.14159265358979323846\n"

    "void main(void)\n"
    "{\n"
        "float linesz = 0.01, padding = 0.5, speed = 1.0 / 2.0;\n"
        "linesz = clamp(linesz, 0, 1);\n"
        "float dist = abs((fract(-time * speed) * (1 + (linesz + padding) * 2) - linesz - padding) - uv.y);\n"
        "float factor = min(linesz, dist) / linesz;\n"

        "float whnoiselvl = 0.05;\n"
        "float n0 = texture(noise, fract(uv) * (1.0 / 3.0) + vec2(0, 1 / 3)).r;\n"
        "vec4 f = mix(vec4(n0, n0, n0, 1), texture(screen, uv) + vec4(vec3(n0), 0) * whnoiselvl, factor);\n"

        "float cy = dot(f.rgb, vec3(LUMA_R, LUMA_G, LUMA_B));\n"
        "float cu = COEFF_U * (f.b - cy);\n"
        "float cv = COEFF_V * (f.r - cy);\n"

        "float n1 = texture(noise, fract(uv) * (1.0 / 3.0)).r * 2 - 1;\n"
        "float n2 = texture(noise, fract(uv) * (1.0 / 3.0) + (1.0 / 3.0)).r * 2 - 1;\n"
        "float n3 = texture(noise, fract(uv) * (1.0 / 3.0) + (2.0 / 3.0)).r * 2 - 1;\n"
        "cy += n1 * 0.1;\n"
        "cu += n2 * 0.07;\n"
        "cv += n3 * 0.07;\n"

        "cy = clamp(cy, 0, 1);\n"
        "cu = clamp(cu, CH_DELTAB_MIN, CH_DELTAB_MAX);\n"
        "cv = clamp(cv, CH_DELTAR_MIN, CH_DELTAR_MAX);\n"

        "float tb = cu / COEFF_U + cy;\n"
        "float tr = cv / COEFF_V + cy;\n"
        "float tg = (cy - LUMA_R * tr - LUMA_B * tb) / LUMA_G;\n"

        "FragmentColor = vec4(tr, tg, tb, f.a);\n"
    "}"
};