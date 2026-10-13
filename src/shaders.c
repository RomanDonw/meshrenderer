#include "shaders.h"

const char *vertexshadersource =
    "#version 330 core\n"

    "layout (location = 0) in vec3 vpos;\n"

    "uniform mat4 proj;\n"

    "void main(void)\n"
    "{"
        "gl_Position = proj * vec4(vpos, 1);\n"
    "}"
;

const char *fragmentshadersource =
    "#version 330 core\n"

    "out vec4 FragColor;\n"

    "void main(void)\n"
    "{\n"
        "FragColor = vec4(1, 1, 1, 1);\n"
    "}"
;
