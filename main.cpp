#include <iostream>
#include <string>

#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>

#include "matrix.h"

void warning(const std::string& message) {
    std::cerr << "WARNING: " << message << std::endl;
    std::exit(1);
}

void fatal(const std::string& message) {
    std::cerr << "FATAL: " << message << std::endl;
    std::exit(1);
}

struct State {
    bool terminated = false;
    bool key_pressed[1024] = {};
} state;

void on_key(GLFWwindow* window, int key, int /* scancode */, int action, int /* mods */) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        state.terminated = true;
    }
    if (action == GLFW_PRESS) {
        state.key_pressed[key] = true;
    }
    if (action == GLFW_RELEASE) {
        state.key_pressed[key] = false;
    }
}

void on_error(int /* error */, const char* description) {
    warning(description);
}

struct Camera {
    float position[3] = {0.0f, 0.0f, 1.0f};
    float rotation[3] = {0.0f, 0.0f, 0.0f};
    float fov = 1.5;  // ~90 degrees
    float near = 0.1;
    float far = 200.0;

    void move(const State& state, double delta) {
        double rate = 1.0;
        if (state.key_pressed[GLFW_KEY_Q]) {
            position[2] += rate * delta;
        }
        if (state.key_pressed[GLFW_KEY_E]) {
            position[2] -= rate * delta;
        }

        if (state.key_pressed[GLFW_KEY_W]) {
            rotation[0] -= rate * delta;
        }
        if (state.key_pressed[GLFW_KEY_S]) {
            rotation[0] += rate * delta;
        }
        if (state.key_pressed[GLFW_KEY_D]) {
            rotation[1] -= rate * delta;
        }
        if (state.key_pressed[GLFW_KEY_A]) {
            rotation[1] += rate * delta;
        }
    }

    Matrix4f getTransform(int width, int height) const {
        return Matrix4f::identity()
            .multiply(Matrix4f::perspective(width, height, near, far, fov))
            .multiply(
                Matrix4f::translation(
                    position[0],
                    position[1],
                    position[2]))
            .multiply(Matrix4f::rotation(rotation[0], Axis::X))
            .multiply(Matrix4f::rotation(rotation[1], Axis::Y))
            .multiply(Matrix4f::rotation(rotation[2], Axis::Z));
    }
} camera;

const char* kVertexShader = R"(
#version 330
layout (location = 0) in vec3 position;
uniform mat4 gTransform;
out vec4 Color;
void main() {
    gl_Position = gTransform * vec4(position, 1.0);
    Color = vec4(1.0, 1.0, 1.0, 1.0);
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

    // TODO: factor out
    float vertices[] = {
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.2f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.5f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.5f,
        0.0f, 1.0f, 0.5f,
        0.5f, 0.0f, 0.5f,
        0.5f, 1.0f, 0.5f,
        1.0f, 0.0f, 0.5f,
        1.0f, 1.0f, 0.5f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.5f, 0.0f, 1.0f,
        0.5f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLuint IBO;
    GLuint indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    GLuint shaders = setup_shaders();

    auto gTransform = glGetUniformLocation(shaders, "gTransform");

    double lastTime = 0.0;
    while (!state.terminated) {
        double time = glfwGetTime();
        double delta = time - lastTime;
        lastTime = time;

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.move(state, delta);
        Matrix4f transform = camera.getTransform(width, height);

        glUniformMatrix4fv(gTransform, 1, GL_TRUE, &transform.data[0][0]);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glDrawElements(GL_LINES, 12, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
