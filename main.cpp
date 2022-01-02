#include <iostream>
#include <string>

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

const char* pVS = "                                                           \n\
#version 330                                                                  \n\
                                                                              \n\
layout (location = 0) in vec3 Position;                                       \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    gl_Position = vec4(.5 * Position.x, .5 * Position.y, Position.z, 1.0);  \n\
}";

const char* pVF = "                                               \n\
#version 330                                                                  \n\
                                                                              \n\
out vec4 FragColor;                                                           \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    FragColor = vec4(1.0, 1.0, 0.0, 1.0);                                     \n\
}";

void add_shader(GLuint program, const char* text, GLenum type) {
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

    glAttachShader(program, shader);
}

void setup_shaders() {
    GLuint program = glCreateProgram();
    if (!program) {
        fatal("Unable to create program");
    }

    add_shader(program, pVS, GL_VERTEX_SHADER);
    add_shader(program, pVF, GL_FRAGMENT_SHADER);

    glLinkProgram(program);

    GLint result;
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
        Vector { 1.0f, -1.0f, 0.0f },
        Vector { 0.0f, 1.0f, 0.0f },
    };

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

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
