#include "shaders.h"

const char *vertexshadersource =
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
        "vec3 pos = downscale ? vpos * vec3(0.0254) : vpos;\n"
        "gl_Position = proj * view * vec4(pos.x, pos.y, pos.z, 1);\n"
    "}"
;

const char *fragmentshadersource =
    "#version 330 core\n"

    "in vec2 uv;\n"
    "out vec4 FragColor;\n"

    "uniform sampler2D albedo;"

    "void main(void)\n"
    "{\n"
        //"FragColor = vec4(1, 1, 1, 1);\n"
        "FragColor = texture(albedo, uv);\n"
    "}"
;
