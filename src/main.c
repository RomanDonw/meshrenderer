#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stb_image.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <ctype.h>
#include <time.h>

#include "vec2.h"
#include "vec3.h"
#include "mesh.h"
#include "glmesh.h"
#include "mat4.h"
#include "shaders.h"
#include "util.h"

static bool isshadercompilationsuccessful(GLuint shader);
static GLchar *getshadercompilelog(GLuint shader);
static char loadGLtexture(GLuint *texture, const char *file);

#define GLDEBUG() (printf("OpenGL error %u at %llu:%s in function %s\n", glGetError(), __LINE__, __FILE__, __func__))
#define PI 3.14159265358979323846
#define RAD(x) (((x) / 180.0) * PI)

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

static GLint u_proj = -1;
static GLuint fbtexcolor = 0, rbdepth = 0;
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

    if (fbtexcolor)
    {
        glBindTexture(GL_TEXTURE_2D, fbtexcolor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (rbdepth)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, rbdepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}


#define DEFAULT_SPEED 2.0
static float speed = DEFAULT_SPEED;
static void onscroll(GLFWwindow *w, double x, double y)
{
    float s = speed + y / 5;
    speed = s > 0 ? s : 0;
}

#define NOISETEX_WIDTH 256
#define NOISETEX_HEIGHT 256

static const char *memallocfailstr = "memory allocation failed";

int main(int argc, char *argv[])
{
    const char *meshpath = NULL, *texspath = NULL;
    bool downscale = false, flipv = false;
    {
        int p;
        while ((p = getopt(argc, argv, "m:dt:v")) != -1)
        {
            switch (p)
            {
                case 'm':
                    meshpath = optarg;
                    break;

                case 'd':
                    downscale = true;
                    break;

                case 't':
                    texspath = optarg;
                    break;

                case 'v':
                    flipv = true;
                    break;
            }
        }
    }

    if (!meshpath)
    {
        puts("specify path to mesh through -m parameter");
        return 1;
    }

    if (!texspath)
    {
        puts("specify path to textures folder through -t parameter");
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
    glfwSetScrollCallback(w, onscroll);

    // ==========================================================================================

    char *log;
    GLuint modelprog = glCreateProgram();
    if (shprog_addshader(modelprog, GL_VERTEX_SHADER, modelshaders.vertex, &log))
    {
        if (log) { printf("failed compiling vertex shader for model shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }
    if (shprog_addshader(modelprog, GL_FRAGMENT_SHADER, modelshaders.fragment, &log))
    {
        if (log) { printf("failed compiling fragment shader for model shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }
    if (shprog_link(modelprog, &log))
    {
        if (log) { printf("failed linking model shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }

    u_proj = glGetUniformLocation(modelprog, "proj");

    GLint u_view = glGetUniformLocation(modelprog, "view");
    GLint u_tex_albedo = glGetUniformLocation(modelprog, "albedo");
    GLint u_downscale = glGetUniformLocation(modelprog, "downscale");
    GLint u_flipv = glGetUniformLocation(modelprog, "flipv");
    
    glUniform1ui(u_downscale, downscale);
    glUniform1ui(u_flipv, flipv);

    // ==========================================================================================

    GLuint screenprog = glCreateProgram();
    if (shprog_addshader(screenprog, GL_VERTEX_SHADER, screenshaders.vertex, &log))
    {
        if (log) { printf("failed compiling vertex shader for screen shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }
    if (shprog_addshader(screenprog, GL_FRAGMENT_SHADER, screenshaders.fragment, &log))
    {
        if (log) { printf("failed compiling fragment shader for screen shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }
    if (shprog_link(screenprog, &log))
    {
        if (log) { printf("failed linking screen shader program:\n%s\n", log); free(log); }
        else puts(memallocfailstr);
        goto errorquit_glfw;
    }

    glUniform1i(glGetUniformLocation(screenprog, "screen"), 0);

    // ==========================================================================================

    Mesh *mesh = loadmesh(meshpath);
    if (!mesh) { puts("error loading mesh"); goto errorquit_glfw; }

    GLMesh *glmesh = glmesh_frommesh(mesh);
    freemesh(mesh);
    if (!glmesh) { puts("error creating (translating) OpenGL mesh from generic mesh"); goto errorquit_glfw; }

    // ==========================================================================================

    GLuint *texs = malloc(glmesh->surfacescount * sizeof(GLuint));
    if (!texs) { puts(memallocfailstr); glmesh_free(glmesh); goto errorquit_glfw; }
    memset(texs, 0, glmesh->surfacescount * sizeof(GLuint));

    {
        size_t texspathlen = strlen(texspath);
        size_t texpathsize = texspathlen + 256 + 4;
        char *texpath = malloc(texpathsize);
        if (!texpath) { puts(memallocfailstr); glmesh_free(glmesh); goto errorquit_glfw; }

        memcpy(texpath, texspath, texspathlen);

        for (size_t i = 0; i < glmesh->surfacescount; i++)
        {
            size_t namesize = glmesh->surfaces[i].namesize > 255 ? 255 : glmesh->surfaces[i].namesize;
            for (size_t j = 0; j < namesize; j++)
            {
                *(texpath + texspathlen + j) = /*tolower*/(*(glmesh->surfaces[i].name + j));
            }
            
            memcpy(texpath + texspathlen + namesize, ".png", 4);
            *(texpath + texspathlen + namesize + 4) = 0;

            if (loadGLtexture(texs + i, texpath)) printf("failed to load texture %s\n", texpath);
        }

        free(texpath);
    }

    // ==========================================================================================

    GLuint tex;
    if (loadGLtexture(&tex, "res/C3A2A_TRANS1.png"))
    {
        puts("failed loading texture res/C3A2A_TRANS1.png");
        free(texs);
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

    glBindVertexArray(0);

    #if 0
    struct
    {
        GLuint VAO, VBO, EBO;
    } ndcquad;
    {
        glGenVertexArrays(1, &ndcquad.VAO);
        glBindVertexArray(ndcquad.VAO);
        
        static const float verts[] =
        {
            -1, -1,
            -1, 1,
            1, -1,
            1, 1
        };
        glGenBuffers(1, &ndcquad.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, ndcquad.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);

        static const unsigned int inds[] =
        {
            0, 1, 3,
            0, 2, 3
        };
        glGenBuffers(1, &ndcquad.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ndcquad.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }
    #endif

    // ==========================================================================================

    float *noisetexdata = malloc(NOISETEX_WIDTH * NOISETEX_HEIGHT * sizeof(float));
    if (!noisetexdata) { puts(memallocfailstr); goto errorquit_glfw; }

    GLuint noisetex;
    glGenTextures(1, &noisetex);
    glBindTexture(GL_TEXTURE_2D, noisetex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, NOISETEX_WIDTH, NOISETEX_HEIGHT, 0, GL_RED, GL_FLOAT, noisetexdata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // ==========================================================================================

    glUniform1i(u_tex_albedo, 0);
    glUniform1i(glGetUniformLocation(modelprog, "noisetex"), 1);

    GLuint fb;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    glGenTextures(1, &fbtexcolor);
    glBindTexture(GL_TEXTURE_2D, fbtexcolor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbtexcolor, 0);

    glGenRenderbuffers(1, &rbdepth);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbdepth);

    puts(glCheckFramebufferStatus(GL_FRAMEBUFFER) ? "true" : "false");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ==========================================================================================

    glfwSetCursorPos(w, WIN_WIDTH / 2.0, WIN_HEIGHT / 2.0);
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    {
        glfwSetFramebufferSizeCallback(w, onresize);

        int width, height;
        glfwGetFramebufferSize(w, &width, &height);
        onresize(w, width, height);
    }

    glCullFace(GL_BACK);

    srand(time(NULL));
    glClearColor(0, 0, 0, 1);
    double lastframetime = 0;
    glfwSetTime(0);
    while (!glfwWindowShouldClose(w))
    {
        double currentframetime = glfwGetTime();
        double deltatime = currentframetime - lastframetime;
        lastframetime = currentframetime;
        
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
                else if (glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speedfactor = 0.3;
                else speedfactor = 1;

                float factor = 5 * deltatime * speedfactor * speed;

                static const vec3f worldup = {0, 1, 0};
                vec3f vec3;
                if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) { vec3f_muls(vec3, front, factor); vec3f_sub2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) { vec3f_muls(vec3, front, factor); vec3f_add2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) { vec3f_muls(vec3, right, factor); vec3f_sub2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) { vec3f_muls(vec3, right, factor); vec3f_add2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) { vec3f_muls(vec3, worldup, factor); vec3f_sub2(campos, vec3); }
                if (glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS) { vec3f_muls(vec3, worldup, factor); vec3f_add2(campos, vec3); }
            }

            mat4f_lookat(mat, campos, right, up, front);
            glUniformMatrix4fv(u_view, 1, GL_TRUE, mat);
        }

        // ==========================================================================================

        for (size_t j = 0; j < NOISETEX_HEIGHT; j++)
            for (size_t i = 0; i < NOISETEX_WIDTH; i++)
                noisetexdata[j * NOISETEX_WIDTH + i] = rand() / (float)RAND_MAX;
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, noisetex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NOISETEX_WIDTH, NOISETEX_HEIGHT, GL_RED, GL_FLOAT, noisetexdata);

        // ==========================================================================================

        //glBindFramebuffer(GL_FRAMEBUFFER, fb);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(modelprog);
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (size_t i = 0; i < glmesh->surfacescount; i++)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texs[i] ? texs[i] : tex);

            glBindVertexArray(glmesh->surfaces[i].VAO);
            glDrawElements(GL_TRIANGLES, glmesh->surfaces[i].triscount * 3, GL_UNSIGNED_INT, NULL);
        }

        // ======================================================

        #if 0
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(screenprog);
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbtexcolor);

        glBindVertexArray(ndcquad.VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        #endif

        // ======================================================

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

static char loadGLtexture(GLuint *texture, const char *file)
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