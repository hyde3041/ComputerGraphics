#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

GLuint shaderProgram{};
GLuint VAO{};
GLuint VBO{};
GLint transformLocation{ -1 };
int windowWidth{ 800 };
int windowHeight{ 600 };

int objectType{ 0 }; // 0: 육면체, 1: 사각뿔
bool depthTest{ true };
bool wireFrame{ false };
GLfloat xPos{};
GLfloat yPos{};
GLfloat xRotation{ 30.0f };
GLfloat yRotation{ 30.0f };
int xDirection{}; 
int yDirection{};

const GLfloat cubeVertices[8][6] = {
    {-0.35f, -0.35f,  0.35f, 1.0f, 0.0f, 0.0f},
    { 0.35f, -0.35f,  0.35f, 0.0f, 1.0f, 0.0f},
    { 0.35f,  0.35f,  0.35f, 0.0f, 0.0f, 1.0f},
    {-0.35f,  0.35f,  0.35f, 1.0f, 1.0f, 0.0f},
    {-0.35f, -0.35f, -0.35f, 1.0f, 0.0f, 1.0f},
    { 0.35f, -0.35f, -0.35f, 0.0f, 1.0f, 1.0f},
    { 0.35f,  0.35f, -0.35f, 1.0f, 0.5f, 0.0f},
    {-0.35f,  0.35f, -0.35f, 1.0f, 1.0f, 1.0f}
};

const GLfloat pyramidVertices[5][6] = {
    {-0.35f, -0.35f,  0.35f, 1.0f, 0.0f, 0.0f},
    { 0.35f, -0.35f,  0.35f, 0.0f, 1.0f, 0.0f},
    { 0.35f, -0.35f, -0.35f, 0.0f, 0.0f, 1.0f},
    {-0.35f, -0.35f, -0.35f, 1.0f, 1.0f, 0.0f},
    { 0.0f,   0.4f,   0.0f, 1.0f, 0.0f, 1.0f}
};

const char* vertexSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
uniform mat4 transform;
out vec3 vertexColor;
void main()
{
    gl_Position = transform * vec4(position, 1.0);
    vertexColor = color;
}
)";

const char* fragmentSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 fragmentColor;
void main()
{
    fragmentColor = vec4(vertexColor, 1.0);
}
)";

// 육면체: Y축이 높이이고, 앞면은 +Z쪽이다.
const int cubeFaces[6][4] = {
    {3, 2, 6, 7}, // 0: 윗면   (Y = +0.35)
    {4, 5, 1, 0}, // 1: 아랫면 (Y = -0.35)
    {4, 0, 3, 7}, // 2: 왼쪽   (X = -0.35)
    {1, 5, 6, 2}, // 3: 오른쪽 (X = +0.35)
    {0, 1, 2, 3}, // 4: 앞면   (Z = +0.35)
    {5, 4, 7, 6}  // 5: 뒷면   (Z = -0.35)
};

// 사각뿔도 Y축이 높이인 기준
const int pyramidFaces[5][4] = {
    {0, 1, 4, 0}, // 0: +Z쪽 옆면
    {1, 2, 4, 0}, // 1: +X쪽 옆면
    {2, 3, 4, 0}, // 2: -Z쪽 옆면
    {3, 0, 4, 0}, // 3: -X쪽 옆면
    {3, 2, 1, 0}  // 4: 바닥 (Y = -0.35)
};

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        char log[1024]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compilation failed:\n" << log << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createShader()
{
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertex == 0) return 0;
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragment == 0) {
        glDeleteShader(vertex);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint success{};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char log[1024]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Shader linking failed:\n" << log << '\n';
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

void framebufferSize(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
    if (width > 0 && height > 0) {
        windowWidth = width;
        windowHeight = height;
    }
}

glm::mat4 getViewProjection()
{
    glm::mat4 view = glm::lookAt(glm::vec3(-1.0f, 0.8f, 2.5f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    GLfloat aspect = static_cast<GLfloat>(windowWidth) / windowHeight;
    glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, 0.1f, 10.0f);
    return projection * view;
}

void initBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 36 * 6 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
}

void drawVertices(const GLfloat* vertices, int count, GLenum mode)
{
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * 6 * sizeof(GLfloat), vertices);
    glDrawArrays(mode, 0, count);
}

void drawAxes(const glm::mat4& viewProjection)
{
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));
    const GLfloat axes[]{
        -0.9f, 0.0f, 0.0f, 1.0f, 0.2f, 0.2f,
         0.9f, 0.0f, 0.0f, 1.0f, 0.2f, 0.2f,
         0.0f,-0.9f, 0.0f, 0.2f, 1.0f, 0.2f,
         0.0f, 0.9f, 0.0f, 0.2f, 1.0f, 0.2f,
         0.0f, 0.0f,-0.9f, 0.3f, 0.5f, 1.0f,
         0.0f, 0.0f, 0.9f, 0.3f, 0.5f, 1.0f
    };
    drawVertices(axes, 6, GL_LINES);
}

void drawShape(const glm::mat4& viewProjection)
{
    // 이동은 전체 도형에 적용하고, 회전은 도형 자체의 X축과 Y축을 기준으로 한다.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xPos, yPos, 0.0f));
    model = glm::rotate(model, glm::radians(yRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(xRotation), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));

    const int order[6]{ 0, 1, 2, 0, 2, 3 };
    int faceCount = objectType == 0 ? 6 : 5;
    glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
    for (int i = 0; i < faceCount; ++i) {
        GLfloat vertices[6 * 6]{};
        // 사각뿔 옆면은 삼각형 1개, 사각형 면은 삼각형 2개로 그린다.
        int count = (objectType == 1 && i < 4) ? 3 : 6;
        for (int j = 0; j < count; ++j) {
            int index = objectType == 0 ? cubeFaces[i][order[j]] : pyramidFaces[i][order[j]];
            for (int k = 0; k < 6; ++k) {
                vertices[j * 6 + k] = objectType == 0 ? cubeVertices[index][k] : pyramidVertices[index][k];
            }
        }
        drawVertices(vertices, count, GL_TRIANGLES);
    }
}

void moveShape()
{
    xRotation += xDirection * 0.5f;
    yRotation += yDirection * 0.5f;
    if (xRotation >= 360.0f) xRotation -= 360.0f;
    if (xRotation <= -360.0f) xRotation += 360.0f;
    if (yRotation >= 360.0f) yRotation -= 360.0f;
    if (yRotation <= -360.0f) yRotation += 360.0f;
}

void resetMove()
{
    xPos = 0.0f;
    yPos = 0.0f;
    xRotation = 30.0f;
    yRotation = 30.0f;
    xDirection = 0;
    yDirection = 0;
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    // 방향키는 누르고 있는 동안 반복 입력으로 계속 이동한다.
    if (key == GLFW_KEY_LEFT) {
        xPos -= 0.05f;
        return;
    }
    else if (key == GLFW_KEY_RIGHT) {
        xPos += 0.05f;
        return;
    }
    else if (key == GLFW_KEY_UP) {
        yPos += 0.05f;
        return;
    }
    else if (key == GLFW_KEY_DOWN) {
        yPos -= 0.05f;
        return;
    }
    // 도형 선택과 회전 설정은 처음 누른 순간에만 바꾼다.
    if (action != GLFW_PRESS) return;
    bool shift = (mods & GLFW_MOD_SHIFT) != 0; // Shift를 누르면 대문자 명령이다.

    if (key == GLFW_KEY_C) {
        objectType = 0;
    }
    else if (key == GLFW_KEY_P) {
        objectType = 1;
    }
    else if (key == GLFW_KEY_H) {
        depthTest = !depthTest;
    }
    else if (key == GLFW_KEY_W) {
        wireFrame = !shift; // w: 와이어, W: 솔리드
    }
    else if (key == GLFW_KEY_X) {
        int direction = shift ? -1 : 1;
        xDirection = xDirection == direction ? 0 : direction;
    }
    else if (key == GLFW_KEY_Y) {
        int direction = shift ? -1 : 1;
        yDirection = yDirection == direction ? 0 : direction;
    }
    else if (key == GLFW_KEY_S) {
        resetMove();
    }
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void drawScene()
{
    if (depthTest) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 viewProjection = getViewProjection();
    drawAxes(viewProjection);
    drawShape(viewProjection);
}

int main()
{
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "GLFW initialization failed.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 창을 만들 때 깊이값을 기록할 24비트 깊이 버퍼를 요청한다.
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-14", nullptr, nullptr);
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
    glUseProgram(shaderProgram);
    transformLocation = glGetUniformLocation(shaderProgram, "transform");
    initBuffer();
    // 은면 제거에 사용할 깊이 검사를 켠다. 겹치는 면 중 가까운 면을 남긴다.
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glfwSwapInterval(1);
    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSize(window, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetKeyCallback(window, keyCallback);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        moveShape();
        drawScene();
        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
