// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cstdio>

struct Shape {
    GLclampf Red{};
    GLclampf Green{};
    GLclampf Blue{};
    // 0: 사각형, 1: 정삼각형, 2: 직각삼각형
    int type{};
    int vertexCount{};
    GLfloat xPos{};
    GLfloat yPos{};
    GLfloat xVertex[4]{};
    GLfloat yVertex[4]{};
    int slot{ -1 };
    bool locked{ false };
};

struct Slot {
    Shape shape;
    int board{};
    int piece{ -1 };
};

struct Board {
    int firstSlot{};
    int slotCount{};
    bool complete{ false };
};

Shape shape[40]{};
Slot slot[30]{};
Board board[5]{};

int shapeCount{};
int slotCount{};
int selectShape{ -1 };
int oldSlot{ -1 };

GLfloat oldX{}, oldY{};
GLfloat dragX{}, dragY{};


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

// 800x600에서 실제 길이가 같도록 x축 길이에 3/4을 곱한다.
Shape makeShape(int type, GLfloat x, GLfloat y, GLfloat width, GLfloat height, int turn)
{
    Shape result{};
    result.type = type;
    result.xPos = x;
    result.yPos = y;
    result.vertexCount = (type == 0) ? 4 : 3;

    GLfloat px[4]{ -width / 2, width / 2, width / 2, -width / 2 };
    GLfloat py[4]{ -height / 2, -height / 2, height / 2, height / 2 };
    if (type == 1) {
        height = width * std::sqrt(3.0F) / 2.0F;
        px[2] = 0.0F;
        py[0] = py[1] = -height / 2.0F;
        py[2] = height / 2.0F;
    }
    else if (type == 2) {
        px[2] = -width / 2.0F;
    }

    for (int i = 0; i < result.vertexCount; ++i) {
        for (int j = 0; j < turn; ++j) {
            const GLfloat temp = px[i];
            px[i] = -py[i];
            py[i] = temp;
        }
        result.xVertex[i] = px[i] * 0.75F;
        result.yVertex[i] = py[i];
    }
    return result;
}

void addSlot(int index, int type, GLfloat x, GLfloat y, GLfloat width, GLfloat height, int turn = 0)
{
    slot[slotCount] = Slot{};
    slot[slotCount].shape = makeShape(type, x, y, width, height, turn);
    slot[slotCount].board = index;
    ++board[index].slotCount;
    ++slotCount;
}

void makeBoards()
{
    slotCount = 0;
    for (int i = 0; i < 5; ++i) {
        board[i] = Board{};
    }

    // 1. 작은 사각형 네 개로 큰 사각형을 만든다.
    board[0].firstSlot = slotCount;
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            addSlot(0, 0, 0.25F + (col - 0.5F) * 0.087F, 0.68F + (row - 0.5F) * 0.116F, 0.1F, 0.1F);
        }
    }

    // 2. 안쪽을 바라보는 정삼각형 네 개로 바람개비를 만든다.
    board[1].firstSlot = slotCount;
    const GLfloat offset = 0.16F * std::sqrt(3.0F) / 4.0F + 0.014F;
    addSlot(1, 1, 0.74F, 0.68F + offset, 0.16F, 0.0F, 2);
    addSlot(1, 1, 0.74F, 0.68F - offset, 0.16F, 0.0F, 0);
    addSlot(1, 1, 0.74F - offset * 0.75F, 0.68F, 0.16F, 0.0F, 3);
    addSlot(1, 1, 0.74F + offset * 0.75F, 0.68F, 0.16F, 0.0F, 1);

    // 3. 직각삼각형 두 개로 세로 직사각형을 만든다.
    board[2].firstSlot = slotCount;
    addSlot(2, 2, 0.25F, 0.05F, 0.2F, 0.34F, 0);
    addSlot(2, 2, 0.25F, 0.05F, 0.2F, 0.34F, 2);

    // 4. 집은 큰 사각형 네 개와 폭을 맞춘 정삼각형 지붕으로 만든다.
    board[3].firstSlot = slotCount;
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            addSlot(3, 0, 0.74F + (col - 0.5F) * 0.105F,
                -0.035F + (row - 0.5F) * 0.14F, 0.14F, 0.14F);
        }
    }
    addSlot(3, 1, 0.74F, 0.105F + 0.28F * std::sqrt(3.0F) / 4.0F, 0.28F, 0.0F);

    // 5. 작은 사각형 세 개로 두 단 계단을 만든다.
    board[4].firstSlot = slotCount;
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2 - row; ++col) {
            addSlot(4, 0, 0.40F + (col + row) * 0.06F,
                -0.75F + row * 0.08F, 0.08F, 0.08F);
        }
    }
}

void getBounds(const Shape* target, GLfloat* minX, GLfloat* minY, GLfloat* maxX, GLfloat* maxY)
{
    *minX = *maxX = target->xPos + target->xVertex[0];
    *minY = *maxY = target->yPos + target->yVertex[0];
    for (int i = 1; i < target->vertexCount; ++i) {
        const GLfloat x = target->xPos + target->xVertex[i];
        const GLfloat y = target->yPos + target->yVertex[i];
        if (x < *minX) *minX = x;
        if (x > *maxX) *maxX = x;
        if (y < *minY) *minY = y;
        if (y > *maxY) *maxY = y;
    }
}

bool overlapShape(const Shape* first, const Shape* second)
{
    GLfloat x1, y1, x2, y2, a1, b1, a2, b2;
    getBounds(first, &x1, &y1, &x2, &y2);
    getBounds(second, &a1, &b1, &a2, &b2);
    return x1 < a2 + 0.008F && x2 > a1 - 0.008F &&
        y1 < b2 + 0.008F && y2 > b1 - 0.008F;
}

void resetShapes()
{
    makeBoards();
    selectShape = -1;
    oldSlot = -1;

    // 모든 모양판을 완성할 조각을 먼저 만들고 여분을 랜덤하게 추가한다.
    shapeCount = slotCount + 2 + std::rand() % 9;
    for (int i = 0; i < shapeCount; ++i) {
        const int index = (i < slotCount) ? i : std::rand() % slotCount;
        shape[i] = slot[index].shape;
        shape[i].xPos = shape[i].yPos = 0.0F;
        setColor(&shape[i].Red, &shape[i].Green, &shape[i].Blue);
        shape[i].Red = 0.2F + shape[i].Red * 0.65F;
        shape[i].Green = 0.2F + shape[i].Green * 0.65F;
        shape[i].Blue = 0.2F + shape[i].Blue * 0.65F;

        GLfloat minX, minY, maxX, maxY;
        getBounds(&shape[i], &minX, &minY, &maxX, &maxY);
        // 왼쪽 영역에 놓으며 가능한 한 서로 겹치지 않는 위치를 찾는다.
        for (int attempt = 0; attempt < 500; ++attempt) {
            shape[i].xPos = randomFloat(-0.97F - minX, -0.04F - maxX);
            shape[i].yPos = randomFloat(-0.93F - minY, 0.93F - maxY);
            bool overlap = false;
            for (int j = 0; j < i; ++j) {
                if (overlapShape(&shape[i], &shape[j])) {
                    overlap = true;
                    break;
                }
            }
            if (!overlap) break;
        }
    }
}

bool containsPoint(const Shape* target, GLfloat x, GLfloat y)
{
    bool positive = false;
    bool negative = false;
    x -= target->xPos;
    y -= target->yPos;
    for (int i = 0; i < target->vertexCount; ++i) {
        const int next = (i + 1) % target->vertexCount;
        const GLfloat cross = (target->xVertex[next] - target->xVertex[i]) *
            (y - target->yVertex[i]) - (target->yVertex[next] - target->yVertex[i]) *
            (x - target->xVertex[i]);
        if (cross > 0.000001F) positive = true;
        if (cross < -0.000001F) negative = true;
    }
    return !(positive && negative);
}

int findShape(GLfloat x, GLfloat y)
{
    for (int i = shapeCount - 1; i >= 0; --i) {
        if (!shape[i].locked && containsPoint(&shape[i], x, y)) return i;
    }
    return -1;
}

bool sameShape(const Shape* first, const Shape* second)
{
    if (first->type != second->type || first->vertexCount != second->vertexCount) return false;
    // 꼭짓점의 저장 순서는 달라도 위치, 크기, 방향이 같으면 같은 조각이다.
    for (int i = 0; i < first->vertexCount; ++i) {
        bool found = false;
        for (int j = 0; j < second->vertexCount; ++j) {
            if (std::abs(first->xVertex[i] - second->xVertex[j]) < 0.0001F &&
                std::abs(first->yVertex[i] - second->yVertex[j]) < 0.0001F) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

void beginDrag(GLfloat x, GLfloat y)
{
    selectShape = findShape(x, y);
    if (selectShape == -1) return;
    Shape* selected = &shape[selectShape];
    oldX = selected->xPos;
    oldY = selected->yPos;
    oldSlot = selected->slot;
    dragX = x - selected->xPos;
    dragY = y - selected->yPos;
    if (oldSlot != -1) {
        slot[oldSlot].piece = -1;
        selected->slot = -1;
    }
}

void moveShape(GLfloat x, GLfloat y)
{
    if (selectShape == -1) return;
    Shape* selected = &shape[selectShape];
    selected->xPos = x - dragX;
    selected->yPos = y - dragY;
    GLfloat minX, minY, maxX, maxY;
    getBounds(selected, &minX, &minY, &maxX, &maxY);
    if (minX < -1.0F) selected->xPos += -1.0F - minX;
    if (maxX > 1.0F) selected->xPos -= maxX - 1.0F;
    if (minY < -1.0F) selected->yPos += -1.0F - minY;
    if (maxY > 1.0F) selected->yPos -= maxY - 1.0F;
}

void checkComplete()
{
    for (int i = 0; i < 5; ++i) {
        bool complete = true;
        for (int j = board[i].firstSlot; j < board[i].firstSlot + board[i].slotCount; ++j) {
            if (slot[j].piece == -1) complete = false;
        }
        board[i].complete = complete;
        if (complete) {
            for (int j = board[i].firstSlot; j < board[i].firstSlot + board[i].slotCount; ++j) {
                shape[slot[j].piece].locked = true;
            }
        }
    }
}

void finishDrag()
{
    if (selectShape == -1) return;
    Shape* selected = &shape[selectShape];
    int nearest = -1;
    GLfloat distance = 0.08F * 0.08F;
    for (int i = 0; i < slotCount; ++i) {
        if (slot[i].piece != -1 || !sameShape(selected, &slot[i].shape)) continue;
        const GLfloat dx = (selected->xPos - slot[i].shape.xPos) / 0.75F;
        const GLfloat dy = selected->yPos - slot[i].shape.yPos;
        const GLfloat nextDistance = dx * dx + dy * dy;
        if (nextDistance < distance) {
            distance = nextDistance;
            nearest = i;
        }
    }

    if (nearest != -1) {
        selected->xPos = slot[nearest].shape.xPos;
        selected->yPos = slot[nearest].shape.yPos;
        selected->slot = nearest;
        slot[nearest].piece = selectShape;
    }
    else {
        // 맞는 칸에 놓지 못하면 드래그를 시작한 자리로 되돌린다.
        selected->xPos = oldX;
        selected->yPos = oldY;
        selected->slot = oldSlot;
        if (oldSlot != -1) slot[oldSlot].piece = selectShape;
    }
    selectShape = -1;
    oldSlot = -1;
    checkComplete();
}

void updateTitle(GLFWwindow* window)
{
    int count = 0;
    for (int i = 0; i < 5; ++i) {
        if (board[i].complete) ++count;
    }
    char title[128]{};
    std::snprintf(title, sizeof(title),
        "Project1-10 | Completed %d/5 | Drag: Left mouse | R: Reset | Q: Quit", count);
    glfwSetWindowTitle(window, title);
}

bool getMousePos(GLFWwindow* window, GLfloat* x, GLfloat* y)
{
    int width, height;
    double mouseX, mouseY;
    glfwGetWindowSize(window, &width, &height);
    if (width <= 0 || height <= 0) return false;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    *x = static_cast<GLfloat>(mouseX / width * 2.0 - 1.0);
    *y = static_cast<GLfloat>(1.0 - mouseY / height * 2.0);
    return true;
}

void mouseCallback(GLFWwindow* window, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    GLfloat x, y;
    if (!getMousePos(window, &x, &y)) return;
    if (action == GLFW_PRESS) {
        beginDrag(x, y);
    }
    else if (action == GLFW_RELEASE) {
        moveShape(x, y);
        finishDrag();
        updateTitle(window);
    }
}

void cursorCallback(GLFWwindow* window, double, double)
{
    GLfloat x, y;
    if (getMousePos(window, &x, &y)) moveShape(x, y);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_R) {
        resetShapes();
        updateTitle(window);
    }
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    
}


void drawVertices(const GLfloat* vertices, int count, GLenum primitive,
    GLclampf Red, GLclampf Green, GLclampf Blue)
{
    glUseProgram(shaderProgram);
    glUniform3f(colorLocation, Red, Green, Blue);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(GLfloat), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(primitive, 0, count);
}

void drawShape(const Shape* target, bool filled, GLclampf Red, GLclampf Green, GLclampf Blue)
{
    GLfloat vertices[12]{};
    const int order[6]{ 0, 1, 2, 0, 2, 3 };
    const int count = (filled && target->vertexCount == 4) ? 6 : target->vertexCount;
    for (int i = 0; i < count; ++i) {
        const int index = (filled && target->vertexCount == 4) ? order[i] : i;
        vertices[i * 2] = target->xPos + target->xVertex[index];
        vertices[i * 2 + 1] = target->yPos + target->yVertex[index];
    }
    drawVertices(vertices, count, filled ? GL_TRIANGLES : GL_LINE_LOOP, Red, Green, Blue);
}

void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    const GLfloat divider[]{ 0.0F, -1.0F, 0.0F, 1.0F };
    drawVertices(divider, 2, GL_LINES, 0.5F, 0.5F, 0.5F);
    for (int i = 0; i < 5; ++i) {
        GLfloat minX = 1.0F, minY = 1.0F, maxX = -1.0F, maxY = -1.0F;
        for (int j = board[i].firstSlot; j < board[i].firstSlot + board[i].slotCount; ++j) {
            GLfloat x1, y1, x2, y2;
            getBounds(&slot[j].shape, &x1, &y1, &x2, &y2);
            if (x1 < minX) minX = x1;
            if (y1 < minY) minY = y1;
            if (x2 > maxX) maxX = x2;
            if (y2 > maxY) maxY = y2;
            drawShape(&slot[j].shape, true, 0.90F, 0.92F, 0.95F);
            drawShape(&slot[j].shape, false, 0.28F, 0.38F, 0.50F);
        }
        const GLfloat bounds[]{
            minX - 0.015F, minY - 0.02F, maxX + 0.015F, minY - 0.02F,
            maxX + 0.015F, maxY + 0.02F, minX - 0.015F, maxY + 0.02F
        };
        drawVertices(bounds, 4, GL_LINE_LOOP, board[i].complete ? 0.1F : 0.75F,
            board[i].complete ? 0.7F : 0.78F, board[i].complete ? 0.3F : 0.82F);
    }
    for (int i = 0; i < shapeCount; ++i) {
        if (i == selectShape) continue;
        drawShape(&shape[i], true, shape[i].Red, shape[i].Green, shape[i].Blue);
        drawShape(&shape[i], false, 0.2F, 0.2F, 0.2F);
    }
    if (selectShape != -1) {
        drawShape(&shape[selectShape], true, shape[selectShape].Red, shape[selectShape].Green, shape[selectShape].Blue);
        drawShape(&shape[selectShape], false, 0.0F, 0.0F, 0.0F);
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-10", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Window creation failed.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowAspectRatio(window, 4, 3);
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
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);
    glClearColor(0.97F, 0.97F, 0.98F, 1.0F);
    resetShapes();
    updateTitle(window);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        drawScene();
        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
