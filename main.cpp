#include <iostream>
#include <string>
#include <cmath>

#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>

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

const char* kVertexShader = R"(
#version 330
layout (location = 0) in vec3 position;
uniform mat4 world;
out vec4 Color;
void main() {
    gl_Position = world * vec4(position, 1.0);
    Color = vec4(clamp(position, 0.0, 1.0), 1.0);
}
)";

const char* kFragmentShader = R"(
#version 330
in vec4 Color;
out vec4 FragColor;
void main() {
    FragColor = Color;
}
)";

GLuint create_shader(const char* text, GLenum type) {
    auto shader = glCreateShader(type);
    if (!shader) {
        fatal("Unable to create shader");
    }

    const GLchar* sources[] = {text};
    const GLint lengths[] = {(GLint)strlen(text)};

    glShaderSource(shader, 1, sources, lengths);
    glCompileShader(shader);

    GLint result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (!result) {
        GLchar buffer[1024];
        glGetShaderInfoLog(shader, sizeof(buffer), nullptr, buffer);
        fatal(std::string("Unable to compile shader:\n") + buffer);
    }

    return shader;
}

GLuint setup_shaders() {
    GLuint program = glCreateProgram();
    if (!program) {
        fatal("Unable to create program");
    }

    glAttachShader(program, create_shader(kVertexShader, GL_VERTEX_SHADER));
    glAttachShader(program, create_shader(kFragmentShader, GL_FRAGMENT_SHADER));

    GLint result;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (!result) {
        fatal("Unable to link shaders");
    }

    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
    if (!result) {
        GLchar buffer[1024];
        glGetProgramInfoLog(program, sizeof(buffer), nullptr, buffer);
        fatal(std::string("Unable to validate shader:\n") + buffer);
    }

    glUseProgram(program);
    return program;
}

int main(int argc, const char* argv[]) {
    if (!glfwInit()) {
        fatal("Unable to initalize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

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
        Vector { 0.0f, -1.0f, 1.0f },
        Vector { 1.0f, -1.0f, 0.0f },
        Vector { 0.0f, 1.0f, 0.0f },
    };

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(2, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint IBO;
    GLuint indices[] = {0, 3, 1, 1, 3, 2, 2, 3, 0, 0, 2, 1};
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    GLuint shaders = setup_shaders();

    auto world = glGetUniformLocation(shaders, "world");
    float transformation[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    glUniformMatrix4fv(world, 1, GL_TRUE, &transformation[0][0]);


    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        auto time = glfwGetTime();
        transformation[0][0] = cos(time);
        transformation[0][2] = -sin(time);
        transformation[2][0] = sin(time);
        transformation[2][2] = cos(time);
        glUniformMatrix4fv(world, 1, GL_TRUE, &transformation[0][0]);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
