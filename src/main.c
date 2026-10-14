#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stb_image.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "vec2.h"
#include "vec3.h"
#include "mesh.h"
#include "glmesh.h"
#include "mat4.h"
#include "shaders.h"

static bool isshadercompilationsuccessful(GLuint shader);
static GLchar *getshadercompilelog(GLuint shader);
static char loadGLTexture(GLuint *texture, const char *file);

#define GLDEBUG() (printf("OpenGL error %u at %llu:%s in function %s\n", glGetError(), __LINE__, __FILE__, __func__))
#define PI 3.14159265358979323846
#define RAD(x) (((x) / 180.0) * PI)

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

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


#define DEFAULT_SPEED 1.0
static float speed = DEFAULT_SPEED;
static void onscroll(GLFWwindow *w, double x, double y)
{
    float s = speed + y / 5;
    speed = s > 0 ? s : 0;
}

int main(int argc, char *argv[])
{
    const char *meshpath = NULL;
    bool downscale = false;
    {
        int p;
        while ((p = getopt(argc, argv, "m:d")) != -1)
        {
            switch (p)
            {
                case 'm':
                    meshpath = optarg;
                    break;

                case 'd':
                    downscale = true;
                    break;
            }
        }
    }

    if (!meshpath)
    {
        puts("specify path to mesh through -m parameter");
        return 1;
    }

    // ==========================================================================================

    if (!glfwInit()) { puts("failed to initialize GLFW"); return 1; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *w;
    if (!(w = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Mesh renderer", NULL, NULL))) { puts("error creating window"); goto errorquit_glfw; }
    glfwMakeContextCurrent(w);

    if (!gladLoadGL(glfwGetProcAddress)) { puts("error loading OpenGL context through GLAD"); goto errorquit_glfw;  }

    glfwSetFramebufferSizeCallback(w, onresize);
    glfwSetScrollCallback(w, onscroll);

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
    onresize(w, WIN_WIDTH, WIN_HEIGHT);

    GLint u_view = glGetUniformLocation(prog, "view");
    GLint u_tex_albedo = glGetUniformLocation(prog, "albedo");
    GLint u_downscale = glGetUniformLocation(prog, "downscale");
    
    glUniform1ui(u_downscale, downscale);

    // ==========================================================================================

    Mesh *mesh = loadmesh(meshpath);
    if (!mesh) { puts("error loading mesh"); goto errorquit_glfw; }

    GLMesh *glmesh = glmesh_frommesh(mesh);
    freemesh(mesh);
    if (!glmesh) { puts("error creating (translating) OpenGL mesh from generic mesh"); goto errorquit_glfw; }

    GLuint tex;
    if (loadGLTexture(&tex, "res/C3A2A_TRANS1.png"))
    {
        puts("failed loading texture res/C3A2A_TRANS1.png");
        glmesh_free(glmesh);
        goto errorquit_glfw;
    }

    // ==========================================================================================

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

    glUniform1i(u_tex_albedo, 0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glActiveTexture(GL_TEXTURE0);

    glfwSetCursorPos(w, WIN_WIDTH / 2.0, WIN_HEIGHT / 2.0);
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 1);
    double lastframetime = 0;
    glfwSetTime(0);
    while (!glfwWindowShouldClose(w))
    {
        double currentframetime = glfwGetTime();
        double deltatime = currentframetime - lastframetime;
        lastframetime = currentframetime;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        {
            static vec3f campos = {0, 0, 0};
            static vec2f camrot = {0, 0};

            {
                static double lastx = WIN_WIDTH / 2.0, lasty = WIN_HEIGHT / 2.0;
                double x, y;
                glfwGetCursorPos(w, &x, &y);
                double deltax = x - lastx;
                double deltay = y - lasty;
                lastx = x;
                lasty = y;

                camrot[1] += deltax * 0.005;
                camrot[0] += deltay * 0.005;

                camrot[1] = fmodf(camrot[1], 2 * PI);
                camrot[0] = fmodf(camrot[0], 2 * PI);

                if (camrot[0] > RAD(80)) camrot[0] = RAD(80);
                else if (camrot[0] < -RAD(80)) camrot[0] = -RAD(80);
            }

            mat4f mat;
            {
                mat4f_rotatey(mat, camrot[1]);
                mat4f t;
                mat4f_rotatex(t, camrot[0]);
                mat4f_mulm2(mat, t);
            }

            vec4f front = {0, 0, -1, 0}, right = {1, 0, 0, 0}, up = {0, 1, 0, 0};
            mat4f_mulv2(front, mat);
            mat4f_mulv2(right, mat);
            mat4f_mulv2(up, mat);

            vec3f_norm2(front);
            vec3f_norm2(right);
            vec3f_norm2(up);

            {
                if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) speed = DEFAULT_SPEED;

                float speedfactor;
                if (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speedfactor = 3;
                else if (glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(w, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) speedfactor = 0.3;
                else speedfactor = 1;

                float factor = 10 * deltatime * speedfactor * speed;

                vec3f vec3;
                if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) { vec3f_muls(vec3, front, factor);  vec3f_sub2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) { vec3f_muls(vec3, front, factor);  vec3f_add2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) { vec3f_muls(vec3, right, factor);  vec3f_sub2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) { vec3f_muls(vec3, right, factor);  vec3f_add2(campos, vec3); }
            }
            
            mat4f_lookat(mat, campos, right, up, front);
            glUniformMatrix4fv(u_view, 1, GL_TRUE, mat);
        }
        
        for (size_t i = 0; i < glmesh->surfacescount; i++)
        {
            glBindVertexArray(glmesh->surfaces[i].VAO);
            glDrawElements(GL_TRIANGLES, glmesh->surfaces[i].triscount * 3, GL_UNSIGNED_INT, NULL);
        }
        
        skipdraw:
        glfwSwapBuffers(w);
        glfwPollEvents();
    }

    glmesh_free(glmesh);

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

static char loadGLTexture(GLuint *texture, const char *file)
{
    int x, y, ch;
    void *data = stbi_load(file, &x, &y, &ch, 0);
    if (!data) return 1;

    if (x <= 0 || y <= 0 || ch > 4 || ch < 3) { stbi_image_free(data); return 1; }

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexImage2D(GL_TEXTURE_2D, 0, ch == 4 ? GL_RGBA8 : GL_RGB8, x, y, 0, ch == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
}