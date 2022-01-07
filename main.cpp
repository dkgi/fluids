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

enum class Axis { X, Y, Z };

struct Matrix4f {
    Matrix4f() {}

    Matrix4f(const float data[4][4]) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                this->data[i][j] = data[i][j];
            }
        }
    }

    static Matrix4f identity() {
        float data[4][4] = {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };
        return Matrix4f(data);
    }

    static Matrix4f translation(float x, float y, float z) {
        Matrix4f result = Matrix4f::identity();
        result.data[0][3] = x;
        result.data[1][3] = y;
        result.data[2][3] = z;
        return result;
    }

    static Matrix4f rotation(float angle, Axis axis) {
        Matrix4f result = Matrix4f::identity();
        switch(axis) {
            case Axis::X:
                result.data[1][1] = cos(angle);
                result.data[1][2] = -sin(angle);
                result.data[2][1] = sin(angle);
                result.data[2][2] = cos(angle);
                break;
            case Axis::Y:
                result.data[0][0] = cos(angle);
                result.data[0][2] = -sin(angle);
                result.data[2][0] = sin(angle);
                result.data[2][2] = cos(angle);
                break;
            case Axis::Z:
                result.data[0][0] = cos(angle);
                result.data[0][1] = -sin(angle);
                result.data[1][0] = sin(angle);
                result.data[1][1] = cos(angle);
                break;
        }
        return result;
    }
    
    static Matrix4f perspective(int width, int height, float near, float far, float fov) {
        float ratio = (float)width / height;
        float range = far - near;
        float tanFov = tan(fov / 2.0f);

        Matrix4f result;
        result.data[0][0] = 1.0f / (tanFov * ratio);
        result.data[1][1] = 1.0f / tanFov;
        result.data[2][2] = (-near - far) / range;
        result.data[2][3] = 2.0f * far * near / range;
        result.data[3][2] = 1.0f;
        return result;
    }

    Matrix4f multiply(const Matrix4f& other) const {
        Matrix4f result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                float product = 0.0f;
                for (int k = 0; k < 4; k++) {
                    product += this->data[i][k] * other.data[k][j];
                }
                result.data[i][j] = product;
            }
        }
        return result;
    }

    float data[4][4] = {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
    };
};

std::ostream& operator<<(std::ostream& out, const Matrix4f& matrix) {
    out << "{";
    for (int i = 0; i < 4; i++) {
        out << "{";
        for (int j = 0; j < 4; j++) {
            out << matrix.data[i][j];
            if (j != 3) {
                out << ", ";
            }
        }
        if (i != 3) {
            out << "}, ";
        };
    }
    return out << "}";
}

struct Camera {
    float position[3] = {-.5f, -.5f, 1.0f};
    float fov = 1.5;  // ~90 degrees
    float near = 0.1;
    float far = 200.0;

    void move(const State& state, double delta) {
        double rate = 1.0;
        if (state.key_pressed[GLFW_KEY_LEFT]) {
            position[0] += rate * delta;
        }
        if (state.key_pressed[GLFW_KEY_RIGHT]) {
            position[0] -= rate * delta;
        }
        if (state.key_pressed[GLFW_KEY_DOWN]) {
            position[2] += rate * delta;
        }
        if (state.key_pressed[GLFW_KEY_UP]) {
            position[2] -= rate * delta;
        }
    }

    Matrix4f getTransform(int width, int height) const {
        return Matrix4f::identity()
            .multiply(Matrix4f::perspective(width, height, near, far, fov))
            .multiply(
                Matrix4f::translation(
                    position[0],
                    position[1],
                    position[2]));
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

    float vertices[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
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
