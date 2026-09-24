#include "shaders.h"

const char *vertexshadersource =
    "#version 330 core\n"

    "layout (location = 0) in vec3 vpos;\n"
    "layout (location = 1) in vec2 vuv;\n"

    "out vec2 uv;\n"

    "void main(void)\n"
    "{"
        "gl_Position = vec4(vpos.xy, vpos.z - 10, 1);\n"
        "uv = vuv;\n"
    "}"
;

const char *fragmentshadersource =
    "#version 330 core\n"

    "in vec2 uv;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D diffuse;\n"

    "void main(void)\n"
    "{\n"
        "FragColor = texture(diffuse, uv);\n"
    "}"
;
