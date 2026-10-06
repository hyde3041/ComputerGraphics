// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

struct Triangle {
    GLclampf Red;
    GLclampf Green;
    GLclampf Blue;

    GLfloat xPos;
    GLfloat yPos;
    GLfloat size;
    int sizeDirection;
};

// 0: 오른쪽 위, 1: 왼쪽 위, 2: 왼쪽 아래, 3: 오른쪽 아래
Triangle triangle[4]{};
bool polygonFill{ true };

GLuint shaderProgram{};
GLint colorLocation{ -1 };
GLuint vao{};
GLuint vbo{};

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        char message[1024]{};
        glGetShaderInfoLog(shader, 1024, nullptr, message);
        std::cerr << "Shader compilation failed:\n" << message << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createShader()
{
    const char* vertexSource = R"(
#version 330 core
layout (location = 0) in vec2 position;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
}
)";

    const char* fragmentSource = R"(
#version 330 core
uniform vec3 drawColor;
out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(drawColor, 1.0);
}
)";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) {
            glDeleteShader(vertexShader);
        }
        if (fragmentShader != 0) {
            glDeleteShader(fragmentShader);
        }
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char message[1024]{};
        glGetProgramInfoLog(program, 1024, nullptr, message);
        std::cerr << "Shader linking failed:\n" << message << '\n';
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

void setColor(GLclampf* Red, GLclampf* Green, GLclampf* Blue)
{
    *Red = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Green = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Blue = static_cast<GLclampf>(std::rand()) / RAND_MAX;
}

GLfloat randomFloat(GLfloat min, GLfloat max)
{
    return min + static_cast<GLfloat>(std::rand()) / RAND_MAX * (max - min);
}

bool getMousePos(GLFWwindow* window, GLfloat* x, GLfloat* y)
{
    double cursorX;
    double cursorY;
    int width;
    int height;

    glfwGetCursorPos(window, &cursorX, &cursorY);
    glfwGetWindowSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        return false;
    }

    *x = static_cast<GLfloat>(cursorX / width * 2.0 - 1.0);
    *y = static_cast<GLfloat>(1.0 - cursorY / height * 2.0);
    return *x >= -1.0F && *x <= 1.0F && *y >= -1.0F && *y <= 1.0F;
}

int findQuadrant(GLfloat x, GLfloat y)
{
    if (y >= 0.0F) {
        return x >= 0.0F ? 0 : 1;
    }
    return x >= 0.0F ? 3 : 2;
}

// 삼각형이 해당 사분면을 벗어나지 않는 최대 반너비와 반높이를 구한다.
GLfloat maxTriangleSize(int index)
{
    const GLfloat x = std::abs(triangle[index].xPos);
    const GLfloat y = std::abs(triangle[index].yPos);
    GLfloat maxSize = 0.3F;

    if (x < maxSize) {
        maxSize = x;
    }
    if (1.0F - x < maxSize) {
        maxSize = 1.0F - x;
    }
    if (y < maxSize) {
        maxSize = y;
    }
    if (1.0F - y < maxSize) {
        maxSize = 1.0F - y;
    }
    return maxSize;
}

void makeTriangle(int index, GLfloat x, GLfloat y)
{
    const GLfloat minX = (index == 0 || index == 3) ? 0.0F : -1.0F;
    const GLfloat minY = (index == 0 || index == 1) ? 0.0F : -1.0F;

    // 축이나 바깥 테두리에서 0.03 이내를 클릭하면 바꾸지 않는다.
    if (x < minX + 0.03F || x > minX + 0.97F ||
        y < minY + 0.03F || y > minY + 0.97F) {
        return;
    }

    triangle[index].xPos = x;
    triangle[index].yPos = y;
    triangle[index].size = randomFloat(0.08F, 0.2F);
    const GLfloat maxSize = maxTriangleSize(index);
    if (triangle[index].size > maxSize) {
        triangle[index].size = maxSize;
    }
    triangle[index].sizeDirection = triangle[index].size >= maxSize ? -1 : 1;

    setColor(&triangle[index].Red, &triangle[index].Green, &triangle[index].Blue);
}

void resetTriangles()
{
    for (int i = 0; i < 4; ++i) {
        triangle[i] = Triangle{};
    }
    makeTriangle(0, 0.5F, 0.5F);
    makeTriangle(1, -0.5F, 0.5F);
    makeTriangle(2, -0.5F, -0.5F);
    makeTriangle(3, 0.5F, -0.5F);
}

void resizeTriangle(int index)
{
    const GLfloat minSize = 0.02F;
    const GLfloat maxSize = maxTriangleSize(index);

    // 오른쪽 클릭 한 번에 0.02씩 바꾸고, 최소/최대 크기에서 방향을 바꾼다.
    triangle[index].size += triangle[index].sizeDirection * 0.02F;
    if (triangle[index].size >= maxSize) {
        triangle[index].size = maxSize;
        triangle[index].sizeDirection = -1;
    }
    else if (triangle[index].size <= minSize) {
        triangle[index].size = minSize;
        triangle[index].sizeDirection = 1;
    }
}

void drawVertices(const GLfloat* vertices, int count, GLenum primitive, GLclampf Red, GLclampf Green, GLclampf Blue)
{
    glUniform3f(colorLocation, Red, Green, Blue);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * 2 * sizeof(GLfloat), vertices);
    glDrawArrays(primitive, 0, count);
}

void drawTriangle(Triangle* Tri)
{
    // 밑변의 양 끝과 위쪽 꼭짓점으로 이등변삼각형을 만든다.
    const GLfloat vertices[] = {
        Tri->xPos - Tri->size, Tri->yPos - Tri->size,
        Tri->xPos + Tri->size, Tri->yPos - Tri->size,
        Tri->xPos, Tri->yPos + Tri->size
    };
    drawVertices(vertices, 3, polygonFill ? GL_TRIANGLES : GL_LINE_LOOP, Tri->Red, Tri->Green, Tri->Blue);
}

void mouseCallback(GLFWwindow* window, int button, int action, int)
{
    if (action != GLFW_PRESS) {
        return;
    }

    GLfloat mouseX;
    GLfloat mouseY;
    if (!getMousePos(window, &mouseX, &mouseY)) {
        return;
    }
    const int index = findQuadrant(mouseX, mouseY);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        makeTriangle(index, mouseX, mouseY);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        resizeTriangle(index);
    }
}

void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) {
        return;
    }
    if (key == GLFW_KEY_A) {
        polygonFill = true;
    }
    else if (key == GLFW_KEY_B) {
        polygonFill = false;
    }
    else if (key == GLFW_KEY_C) {
        resetTriangles();
    }
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void framebufferSize(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "GLFW initialization failed.\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-8", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Window creation failed.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    shaderProgram = createShader();
    if (shaderProgram == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    //1.사용할꺼 선택
    glUseProgram(shaderProgram);
    colorLocation = glGetUniformLocation(shaderProgram, "drawColor");
    //2. 버퍼생성
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    //3. 사용할버퍼 선택
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // 4. 축 4개 꼭짓점이 가장 크므로 GLfloat 8개 분량을 미리 확보한다.
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    resetTriangles();

    const GLfloat axes[] = { -1.0F, 0.0F, 1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 1.0F };
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        drawVertices(axes, 4, GL_LINES, 0.6F, 0.6F, 0.6F);
        for (int i = 0; i < 4; ++i) {
            drawTriangle(&triangle[i]);
        }
        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
