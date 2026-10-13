#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stb_image.h>
#include <string.h>

#include "mesh.h"
#include "mat4.h"
#include "shaders.h"

static bool isshadercompilationsuccessful(GLuint shader);
static GLchar *getshadercompilelog(GLuint shader);

#define GLDEBUG() (printf("OpenGL error %u at %llu:%s in function %s\n", glGetError(), __LINE__, __FILE__, __func__))
#define PI 3.14159265358979323846
#define RAD(x) (((x) / 180.0) * PI)

static GLint u_proj;
static void onresize(GLFWwindow *window, int width, int height)
{
    int w = width > 0 ? width : 1;
    int h = height > 0 ? height : 1;
    glViewport(0, 0, w, h);

    if (u_proj >= 0)
    {
        mat4f projm;
        mat4f_perspective(projm, RAD(45), (float)width / (float)height, 0.01, 1000);
        glUniformMatrix4fv(u_proj, 1, GL_TRUE, projm);
    }
}

int main(int argc, char *argv[])
{
    // ==========================================================================================

    if (!glfwInit()) { puts("failed to initialize GLFW"); return 1; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *w;
    if (!(w = glfwCreateWindow(800, 600, "Mesh renderer", NULL, NULL))) { puts("error creating window"); goto errorquit_glfw; }
    glfwMakeContextCurrent(w);

    if (!gladLoadGL(glfwGetProcAddress)) { puts("error loading OpenGL context through GLAD"); goto errorquit_glfw;  }
    glfwSetFramebufferSizeCallback(w, onresize);

    // ==========================================================================================
    
    GLuint prog;
    {
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertexshadersource, NULL);
        glCompileShader(vs);
        if (!isshadercompilationsuccessful(vs))
        {
            char *log = getshadercompilelog(vs);
            printf("error compiling vertex shader:\n%s\n", log);
            free(log);
            goto errorquit_glfw;
        }

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragmentshadersource, NULL);
        glCompileShader(fs);
        if (!isshadercompilationsuccessful(fs))
        {
            char *log = getshadercompilelog(fs);
            printf("error compiling fragment shader:\n%s\n", log);
            free(log);
            goto errorquit_glfw;
        }

        prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);
        glUseProgram(prog);

        GLint result;
        glGetProgramiv(prog, GL_LINK_STATUS, &result);
        if (!result)
        {
            GLint length;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &length);
            if (length > 0)
            {
                char *log = malloc(length);
                if (log)
                {
                    glGetProgramInfoLog(prog, length, NULL, log);
                    printf("error linking shader program:\n%s\n", log);
                    free(log);
                }
                goto errorquit_glfw;
            }
        }

    }

    u_proj = glGetUniformLocation(prog, "proj");

    // ==========================================================================================

    Mesh *mesh = loadmesh("res/test.mesh");
    if (!mesh) { puts("error loading mesh"); goto errorquit_glfw; }
    freemesh(mesh);
    
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    static const float vertices[] =
    {
        -0.5, -0.5, -5,
        0, 0.5, -5,
        0.5, -0.5, -5
    };
    static const int indices[] =
    {
        0, 1, 2
    };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // ==========================================================================================

    glClearColor(0, 0, 0, 1);
    while (!glfwWindowShouldClose(w))
    {
        glClear(GL_COLOR_BUFFER_BIT);


        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);

        skipdraw:
        glfwSwapBuffers(w);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

    errorquit_glfw:
        glfwTerminate();
    return 1;
}

static bool isshadercompilationsuccessful(GLuint shader)
{
    GLint result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    return result;
}

static char *getshadercompilelog(GLuint shader)
{
    GLint length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length <= 0) return NULL;

    char *ret = malloc(length);
    glGetShaderInfoLog(shader, length, NULL, ret);
    return ret;
}
