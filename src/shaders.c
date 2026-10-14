#include "shaders.h"

const char *vertexshadersource =
    "#version 330 core\n"

    "layout (location = 0) in vec3 vpos;\n"
    "layout (location = 1) in vec2 vuv;\n"

    "out vec2 uv;\n"

    "uniform mat4 proj;\n"

    "void main(void)\n"
    "{"
        "uv = vuv;\n"
        "gl_Position = proj * vec4(vpos + vec3(0, 0, -5), 1);\n"
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
