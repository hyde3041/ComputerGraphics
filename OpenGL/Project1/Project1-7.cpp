// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <ctime>
#include <iostream>

struct Shape {
    GLclampf Red{};
    GLclampf Green{};
    GLclampf Blue{};

    // 0: 점, 1: 선, 2: 삼각형, 3: 사각형
    int type{};
    int vertexCount{};
    GLfloat xPos[4]{};
    GLfloat yPos[4]{};
};

Shape shape[50]{};
int shapeCount{};
int selectShape{ -1 };
bool selectAll{ false };
int windowWidth{ 800 };
int windowHeight{ 600 };

GLuint shaderProgram{};
GLuint VAO{};
GLuint VBO{};
GLint colorLocation{};

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

GLfloat randomFloat(GLfloat min, GLfloat max)
{
    return min + static_cast<GLfloat>(std::rand()) / RAND_MAX * (max - min);
}

void setColor(GLclampf* Red, GLclampf* Green, GLclampf* Blue)
{
    *Red = randomFloat(0.0F, 1.0F);
    *Green = randomFloat(0.0F, 1.0F);
    *Blue = randomFloat(0.0F, 1.0F);
}

void makeShape(int type)
{
    if (shapeCount >= 50 || type < 0 || type > 3) return;

    shape[shapeCount] = Shape{};
    shape[shapeCount].type = type;
    GLfloat sizeX = randomFloat(0.08F, 0.3F);
    GLfloat sizeY = randomFloat(0.08F, 0.3F);
    if (type == 0) {
        // 점은 작은 사각형으로 표시한다.
        sizeX = 0.01F;
        sizeY = sizeX;
    }

    GLfloat x = randomFloat(-1.0F + sizeX / 2.0F, 1.0F - sizeX / 2.0F);
    GLfloat y = randomFloat(-1.0F + sizeY / 2.0F, 1.0F - sizeY / 2.0F);

    if (type == 1) {
        shape[shapeCount].vertexCount = 2;
        shape[shapeCount].xPos[0] = x - sizeX / 2.0F;
        shape[shapeCount].xPos[1] = x + sizeX / 2.0F;
        GLfloat direction = std::rand() % 2 == 0 ? 1.0F : -1.0F;
        shape[shapeCount].yPos[0] = y - sizeY / 2.0F * direction;
        shape[shapeCount].yPos[1] = y + sizeY / 2.0F * direction;
    }
    else if (type == 2) {
        shape[shapeCount].vertexCount = 3;
        shape[shapeCount].xPos[0] = x;
        shape[shapeCount].yPos[0] = y + sizeY / 2.0F;
        shape[shapeCount].xPos[1] = x - sizeX / 2.0F;
        shape[shapeCount].yPos[1] = y - sizeY / 2.0F;
        shape[shapeCount].xPos[2] = x + sizeX / 2.0F;
        shape[shapeCount].yPos[2] = y - sizeY / 2.0F;
    }
    else {
        shape[shapeCount].vertexCount = 4;
        shape[shapeCount].xPos[0] = shape[shapeCount].xPos[3] = x - sizeX / 2.0F;
        shape[shapeCount].xPos[1] = shape[shapeCount].xPos[2] = x + sizeX / 2.0F;
        shape[shapeCount].yPos[0] = shape[shapeCount].yPos[1] = y - sizeY / 2.0F;
        shape[shapeCount].yPos[2] = shape[shapeCount].yPos[3] = y + sizeY / 2.0F;
    }
    setColor(&shape[shapeCount].Red, &shape[shapeCount].Green, &shape[shapeCount].Blue);
    ++shapeCount;
}

void getBounds(Shape* target, GLfloat* minX, GLfloat* minY, GLfloat* maxX, GLfloat* maxY)
{
    *minX = *maxX = target->xPos[0];
    *minY = *maxY = target->yPos[0];
    for (int i = 1; i < target->vertexCount; ++i) {
        if (target->xPos[i] < *minX) *minX = target->xPos[i];
        if (target->xPos[i] > *maxX) *maxX = target->xPos[i];
        if (target->yPos[i] < *minY) *minY = target->yPos[i];
        if (target->yPos[i] > *maxY) *maxY = target->yPos[i];
    }
}

int findShape(GLfloat x, GLfloat y)
{
    // 나중에 그린 도형부터 검사해서 가장 위에 있는 도형을 선택한다.
    for (int i = shapeCount - 1; i >= 0; --i) {
        GLfloat minX, minY, maxX, maxY;
        getBounds(&shape[i], &minX, &minY, &maxX, &maxY);
        if (x >= minX && x <= maxX && y >= minY && y <= maxY) {
            return i;
        }
    }
    return -1;
}

void moveShape(int index, GLfloat dx, GLfloat dy)
{
    if (index < 0 || index >= shapeCount) return;
    GLfloat minX, minY, maxX, maxY;
    getBounds(&shape[index], &minX, &minY, &maxX, &maxY);
    // 도형 전체가 화면 안에 남도록 이동량을 제한한다.
    if (minX + dx < -1.0F) dx = -1.0F - minX;
    if (maxX + dx > 1.0F) dx = 1.0F - maxX;
    if (minY + dy < -1.0F) dy = -1.0F - minY;
    if (maxY + dy > 1.0F) dy = 1.0F - maxY;

    for (int i = 0; i < shape[index].vertexCount; ++i) {
        shape[index].xPos[i] += dx;
        shape[index].yPos[i] += dy;
    }
}

void moveSelected(GLfloat dx, GLfloat dy)
{
    if (selectAll) {
        for (int i = 0; i < shapeCount; ++i) 
            moveShape(i, dx, dy);
    }
    else {
        moveShape(selectShape, dx, dy);
    }
}

void drawVertices(GLfloat* vertices, int count, GLenum mode, GLclampf Red, GLclampf Green, GLclampf Blue)
{
    glUniform3f(colorLocation, Red, Green, Blue);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * count * 2, vertices);
    glDrawArrays(mode, 0, count);
}

void drawShape(Shape* target)
{
    GLfloat vertices[12]{};
    int count = target->vertexCount;
    // 사각형은 0-1-2와 0-2-3의 두 삼각형으로 그린다.
    int order[6]{ 0, 1, 2, 0, 2, 3 };
    if (target->vertexCount == 4) count = 6;
    for (int i = 0; i < count; ++i) {
        int index = target->vertexCount == 4 ? order[i] : i;
        vertices[i * 2] = target->xPos[index];
        vertices[i * 2 + 1] = target->yPos[index];
    }
    GLenum mode = target->type == 1 ? GL_LINES : GL_TRIANGLES;
    drawVertices(vertices, count, mode, target->Red, target->Green, target->Blue);
}

void drawSelection(Shape* target)
{
    GLfloat minX, minY, maxX, maxY;
    getBounds(target, &minX, &minY, &maxX, &maxY);
    GLfloat vertices[8]{ minX - 0.008F, minY - 0.008F,
        maxX + 0.008F, minY - 0.008F, maxX + 0.008F, maxY + 0.008F,
        minX - 0.008F, maxY + 0.008F };
    drawVertices(vertices, 4, GL_LINE_LOOP, 0.0F, 0.0F, 0.0F);
}

void mouseCallback(GLFWwindow* window, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0) return;
    GLfloat x = static_cast<GLfloat>(mouseX / windowWidth * 2.0 - 1.0);
    GLfloat y = static_cast<GLfloat>(1.0 - mouseY / windowHeight * 2.0);
    selectAll = false;
    selectShape = findShape(x, y);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_P) makeShape(0);
        else if (key == GLFW_KEY_E) makeShape(1);
        else if (key == GLFW_KEY_T) makeShape(2);
        else if (key == GLFW_KEY_R) makeShape(3);
        else if (key == GLFW_KEY_C) {
            shapeCount = 0;
            selectShape = -1;
            selectAll = false;
        }
        else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    const GLfloat speed = 0.02F;
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4) {
        selectAll = true;
        selectShape = -1;
        if (key == GLFW_KEY_1) moveSelected(-speed, 0.0F);
        else if (key == GLFW_KEY_2) moveSelected(speed, 0.0F);
        else if (key == GLFW_KEY_3) moveSelected(0.0F, speed);
        else if (key == GLFW_KEY_4) moveSelected(0.0F, -speed);
    }
    else if (key == GLFW_KEY_W) moveSelected(0.0F, speed);
    else if (key == GLFW_KEY_A) moveSelected(-speed, 0.0F);
    else if (key == GLFW_KEY_S) moveSelected(0.0F, -speed);
    else if (key == GLFW_KEY_D) moveSelected(speed, 0.0F);
    // I/J/K/L: 왼쪽 위 / 오른쪽 위 / 왼쪽 아래 / 오른쪽 아래
    else if (key == GLFW_KEY_I) moveSelected(-speed, speed);
    else if (key == GLFW_KEY_J) moveSelected(speed, speed);
    else if (key == GLFW_KEY_K) moveSelected(-speed, -speed);
    else if (key == GLFW_KEY_L) moveSelected(speed, -speed);
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-7", nullptr, nullptr);
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
    colorLocation = glGetUniformLocation(shaderProgram, "drawColor");
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(0);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int newWidth, int newHeight) {
        glViewport(0, 0, newWidth, newHeight);
    });
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSwapInterval(1);
    glClearColor(1.0F, 1.0F, 1.0F, 1.0F);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        for (int i = 0; i < shapeCount; ++i) drawShape(&shape[i]);
        for (int i = 0; i < shapeCount; ++i) {
            if (selectAll || selectShape == i) drawSelection(&shape[i]);
        }
        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
