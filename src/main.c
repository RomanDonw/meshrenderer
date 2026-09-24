#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stb_image.h>
#include <string.h>

#include "shaders.h"

static void onresize(GLFWwindow *window, int width, int height)
{ glViewport(0, 0, width, height); }
static bool isshadercompilationsuccessful(GLuint shader);
static GLchar *getshadercompilelog(GLuint shader);

#define GLDEBUG() (printf("OpenGL error %u at %llu:%s in function %s\n", glGetError(), __LINE__, __FILE__, __func__))

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

    // ==========================================================================================

    float *meshvertices, *meshUVs;
    uint32_t *meshindices;
    uint32_t meshverticescount, meshUVscount, meshindicescount;
    {
        FILE *meshfile = fopen("res/map.ucmesh", "rb");
        if (!meshfile) { puts("unable to open map mesh file"); goto errorquit_glfw; }

        char sig[6];
        if (!fread(sig, sizeof(sig), 1, meshfile) || ferror(meshfile) || feof(meshfile)) goto readmesherror_generic;
        if (memcmp(sig, "UCMESH", 6)) { puts("invalid signature"); goto readmesherror_generic; }

        uint16_t version;
        if (!fread(&version, sizeof(version), 1, meshfile) || ferror(meshfile) || feof(meshfile)) goto readmesherror_generic;
        if (version != 67) { puts("incorrect format version (must be 67)"); goto readmesherror_generic; }

        uint32_t vertexcount;
        if (!fread(&vertexcount, sizeof(vertexcount), 1, meshfile) || ferror(meshfile) || feof(meshfile)) goto readmesherror_generic;
        uint32_t tricount;
        if (!fread(&tricount, sizeof(tricount), 1, meshfile) || ferror(meshfile) || feof(meshfile)) goto readmesherror_generic;

        meshvertices = malloc(vertexcount * 3 * sizeof(float));
        meshUVs = malloc(vertexcount * 2 * sizeof(float));
        meshindices = malloc(tricount)
        if (!mesh)

        fclose(meshfile);
        goto readmeshfilesuccess;
        readmesherror_generic:
            puts("error parsing mesh");
            fclose(meshfile);
        goto errorquit_glfw;
        readmeshfilesuccess:
    }

    /*
    GLuint VAO, VBO_vertex, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_vertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    */

    // ==========================================================================================

    // ==========================================================================================

    //glUniform1i(glGetUniformLocation(prog, "font"), 0);
    //glBindTexture(GL_TEXTURE_1D, font);
    //glActiveTexture(GL_TEXTURE0);

    glClearColor(0, 0, 0, 1);
    while (!glfwWindowShouldClose(w))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        //glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, NULL);

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
