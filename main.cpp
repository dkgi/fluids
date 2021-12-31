#include <iostream>
#include <string>

#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>

void warning(const std::string& message) {
    std::cerr << "WARNING: " << message << std::endl;
    std::exit(1);
}

void fatal(const std::string& message) {
    std::cerr << "FATAL: " << message << std::endl;
    std::exit(1);
}

void on_key(GLFWwindow* window, int key, int /* scancode */, int action, int /* mods */) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void on_error(int /* error */, const char* description) {
    warning(description);
}

struct Vector {
    float x, y, z;
};

const char* pVS = "                                                           \n\
#version 330                                                                  \n\
                                                                              \n\
layout (location = 0) in vec3 Position;                                       \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    gl_Position = vec4(0.5 * Position.x, 0.5 * Position.y, Position.z, 1.0);  \n\
}";

const char* kFragmentShader = "                                               \n\
#version 330                                                                  \n\
                                                                              \n\
out vec4 FragColor;                                                           \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);                                     \n\
}";

void add_shader() {

}

void setup_shaders() {
    GLuint program = glCreateProgram();
    if (!program) {
        fatal("Unable to create program");
    }

    glLinkProgram(program);

    GLint result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (!result) {
        fatal("Unable to link shaders");
    }

    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
    if (!result) {
        fatal("Unable to validate shaders");
    }

    glUseProgram(program);
}

int main(int argc, const char* argv[]) {
    if (!glfwInit()) {
        fatal("Unable to initalize GLFW");
    }

    auto window = glfwCreateWindow(500, 500, "Fluids", nullptr, nullptr);
    if (!window) {
        fatal("Unable to create window");
    }

    glfwSetKeyCallback(window, on_key);
    glfwSetErrorCallback(on_error);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    Vector vertices[] = {
        Vector { -1.0f, -1.0f, 0.0f },
        Vector { 1.0f, -1.0f, 0.0f },
        Vector { 0.0f, 1.0f, 0.0f },
    };

    GLuint VBO;
    glGenBuffers(2, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // ??: necessary before drawing?
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    setup_shaders();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices));

        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
