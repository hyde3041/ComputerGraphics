// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

struct Triangle {
    GLclampf Red;
    GLclampf Green;
    GLclampf Blue;

    GLfloat xPos;
    GLfloat yPos;
    GLfloat size;
    GLfloat rotation;
    int sizeDirection;

    GLfloat speed;
    int move;
    GLfloat downLength;
    // 스파이럴은 이 중심 주위를 돌면서 반지름을 조금씩 늘린다.
    GLfloat centerX;
    GLfloat centerY;
    GLfloat radius;
    GLfloat angle; // 중심 주위를 도는 각도(도 단위). rotation은 삼각형이 바라보는 각도다.
    std::vector<GLfloat> path;
};

// 생성 사분면 번호: 0 오른쪽 위, 1 왼쪽 위, 2 왼쪽 아래, 3 오른쪽 아래
// 클릭은 해당 번호의 삼각형을 교체하고, 애니메이션도 그 사분면 안에서 진행한다.
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

// 1: 튕기기, 2: 좌우 지그재그, 3: 상하 뾰족 지그재그, 4: 부드러운 스파이럴 이동
int moveMode{ 0 };

void makeSpiralPath(int index)
{
    triangle[index].path.clear();
    triangle[index].path.push_back(triangle[index].xPos);
    triangle[index].path.push_back(triangle[index].yPos);
    if (triangle[index].speed <= 0.0F) return;

    // 실제 위치는 바꾸지 않고, 벽에 닿기 전까지의 좌표를 미리 구한다.
    GLfloat angle = triangle[index].angle;
    GLfloat radius = triangle[index].radius;
    const GLfloat speed = triangle[index].speed;
    const GLfloat margin = triangle[index].size * 1.41421356F;
    const GLfloat minX = (index == 0 || index == 3) ? margin : -1.0F + margin;
    const GLfloat maxX = (index == 0 || index == 3) ? 1.0F - margin : -margin;
    const GLfloat minY = (index == 0 || index == 1) ? margin : -1.0F + margin;
    const GLfloat maxY = (index == 0 || index == 1) ? 1.0F - margin : -margin;

    while (true) {
        angle -= speed * 360.0F;
        radius += speed * 0.1F;
        const GLfloat radians = angle * 0.01745329252F;
        const GLfloat x = triangle[index].centerX + radius * std::cos(radians) * 0.75F;
        const GLfloat y = triangle[index].centerY + radius * std::sin(radians);
        if (x <= minX || x >= maxX || y <= minY || y >= maxY) break;
        triangle[index].path.push_back(x);
        triangle[index].path.push_back(y);
    }
}

// 생성하거나 이동 모드를 바꿀 때 사용할 시작값을 설정한다.
void resetMove(int index)
{
    // 네 삼각형의 속도는 다르다. 스파이럴에서는 각도와 반지름의 증가량에 사용한다.
    triangle[index].speed = 0.003F + index * 0.001F;
    triangle[index].rotation = 0.0F;
    triangle[index].downLength = 0.0F;
    triangle[index].path.clear();

    if (moveMode == 2) {
        triangle[index].move = (index % 2 == 0) ? 4 : 5;
    }
    else if (moveMode == 4) {
        // 지금 위치를 스파이럴의 중심으로 삼고 반지름 0부터 시작한다.
        triangle[index].centerX = triangle[index].xPos;
        triangle[index].centerY = triangle[index].yPos;
        triangle[index].radius = 0.0F;
        triangle[index].angle = -index * 90.0F;
        triangle[index].rotation = triangle[index].angle - 90.0F;
        makeSpiralPath(index);
    }
    else {
        // 0: 오른쪽 위, 1: 왼쪽 위, 2: 오른쪽 아래, 3: 왼쪽 아래
        const int startMove[4]{ 0, 1, 3, 2 };
        triangle[index].move = startMove[index];
    }
}

void changeMove(int mode)
{
    // 같은 숫자를 다시 누르면 이동을 멈춘다.
    if (moveMode == mode) {
        moveMode = 0;
        return;
    }
    moveMode = mode;
    for (int i = 0; i < 4; ++i) {
        resetMove(i);
    }
}

// 매 프레임 네 삼각형의 실제 이동을 모두 여기서 처리한다.
void moveTriangles()
{
    if (moveMode == 0) return;

    for (int i = 0; i < 4; ++i) {
        const GLfloat speed = triangle[i].speed;
        // 대각선으로 회전할 때는 꼭짓점이 더 멀리 뻗으므로 여유를 둔다.
        const bool diagonalRotation = moveMode == 1 || moveMode == 3 || moveMode == 4;
        const GLfloat margin = triangle[i].size * (diagonalRotation ? 1.41421356F : 1.0F);
        const GLfloat minX = (i == 0 || i == 3) ? margin : -1.0F + margin;
        const GLfloat maxX = (i == 0 || i == 3) ? 1.0F - margin : -margin;
        const GLfloat minY = (i == 0 || i == 1) ? margin : -1.0F + margin;
        const GLfloat maxY = (i == 0 || i == 1) ? 1.0F - margin : -margin;

        if (moveMode == 1 || moveMode == 3) {
            // 3번은 가로 이동을 줄여 상하로 뾰족하게 움직인다.
            const GLfloat xSpeed = (moveMode == 3) ? speed * 0.35F : speed;

            if (triangle[i].move == 0) {
                triangle[i].xPos += xSpeed;
                triangle[i].yPos += speed;
                const bool hitRight = triangle[i].xPos >= maxX;
                const bool hitTop = triangle[i].yPos >= maxY;
                if (hitRight && hitTop) triangle[i].move = 3;
                else if (hitRight) triangle[i].move = 1;
                else if (hitTop) triangle[i].move = 2;
            }
            else if (triangle[i].move == 1) {
                triangle[i].xPos -= xSpeed;
                triangle[i].yPos += speed;
                const bool hitLeft = triangle[i].xPos <= minX;
                const bool hitTop = triangle[i].yPos >= maxY;
                if (hitLeft && hitTop) triangle[i].move = 2;
                else if (hitLeft) triangle[i].move = 0;
                else if (hitTop) triangle[i].move = 3;
            }
            else if (triangle[i].move == 2) {
                triangle[i].xPos += xSpeed;
                triangle[i].yPos -= speed;
                const bool hitRight = triangle[i].xPos >= maxX;
                const bool hitBottom = triangle[i].yPos <= minY;
                if (hitRight && hitBottom) triangle[i].move = 1;
                else if (hitRight) triangle[i].move = 3;
                else if (hitBottom) triangle[i].move = 0;
            }
            else if (triangle[i].move == 3) {
                triangle[i].xPos -= xSpeed;
                triangle[i].yPos -= speed;
                const bool hitLeft = triangle[i].xPos <= minX;
                const bool hitBottom = triangle[i].yPos <= minY;
                if (hitLeft && hitBottom) triangle[i].move = 0;
                else if (hitLeft) triangle[i].move = 2;
                else if (hitBottom) triangle[i].move = 1;
            }
            // 사분면 경계를 넘긴 부분은 경계 위치에 맞춘다.
            if (triangle[i].xPos < minX) triangle[i].xPos = minX;
            if (triangle[i].xPos > maxX) triangle[i].xPos = maxX;
            if (triangle[i].yPos < minY) triangle[i].yPos = minY;
            if (triangle[i].yPos > maxY) triangle[i].yPos = maxY;

            // 3번은 가로 이동이 느려 45도보다 위아래에 가깝게 향한다.
            const GLfloat angle = std::atan2(xSpeed, speed) * 57.2957795F;
            if (triangle[i].move == 0) triangle[i].rotation = -angle;
            else if (triangle[i].move == 1) triangle[i].rotation = angle;
            else if (triangle[i].move == 2) triangle[i].rotation = angle - 180.0F;
            else triangle[i].rotation = 180.0F - angle;
        }
        else if (moveMode == 2) {
            // 1-4의 좌우 이동처럼 벽에서 아래로 짧게 이동한다.
            if (triangle[i].move == 4) {
                triangle[i].xPos += speed;
                if (triangle[i].xPos >= maxX) {
                    triangle[i].xPos = maxX;
                    triangle[i].move = 6;
                    triangle[i].downLength = 0.0F;
                }
            }
            else if (triangle[i].move == 5) {
                triangle[i].xPos -= speed;
                if (triangle[i].xPos <= minX) {
                    triangle[i].xPos = minX;
                    triangle[i].move = 7;
                    triangle[i].downLength = 0.0F;
                }
            }
            else if (triangle[i].move == 6 || triangle[i].move == 7) {
                const int nextMove = (triangle[i].move == 6) ? 5 : 4;
                triangle[i].yPos -= speed;
                triangle[i].downLength += speed;
                if (triangle[i].yPos <= minY) {
                    triangle[i].yPos = maxY;
                    triangle[i].downLength = 0.0F;
                    triangle[i].move = nextMove;
                }
                else if (triangle[i].downLength >= 0.1F) {
                    triangle[i].downLength = 0.0F;
                    triangle[i].move = nextMove;
                }
            }
            if (triangle[i].move == 4) triangle[i].rotation = -90.0F;
            else if (triangle[i].move == 5) triangle[i].rotation = 90.0F;
            else triangle[i].rotation = 180.0F;
        }
        else if (moveMode == 4) {
            // 1. 이동 전 위치를 저장한다. 나중에 진행 방향을 구할 때 사용한다.
            const GLfloat oldX = triangle[i].xPos;
            const GLfloat oldY = triangle[i].yPos;

            // 2. 시계 방향으로 조금 돌고, 반지름도 조금 키운다.
            // 한 바퀴(360도) 돌 때 반지름이 약 0.1 늘어난다.
            triangle[i].angle -= speed * 360.0F;
            triangle[i].radius += speed * 0.1F;

            // 3. 고정된 중심에서 새 각도와 반지름에 해당하는 위치로 옮긴다.
            // sin, cos는 라디안을 받으므로 각도를 변환한다.
            const GLfloat radians = triangle[i].angle * 0.01745329252F;
            // 0.75는 800x600 창에서 원이 가로로 늘어나 보이지 않게 보정하는 값.
            triangle[i].xPos = triangle[i].centerX + triangle[i].radius * std::cos(radians) * 0.75F;
            triangle[i].yPos = triangle[i].centerY + triangle[i].radius * std::sin(radians);

            // 4. 방금 움직인 방향으로 삼각형의 꼭짓점도 돌린다.
            const GLfloat dx = triangle[i].xPos - oldX;
            const GLfloat dy = triangle[i].yPos - oldY;
            triangle[i].rotation = std::atan2(dy, dx) * 57.2957795F - 90.0F;

            // 5. 벽에 닿으면 즉시 자기 사분면 중앙으로 돌아간다.
            if (triangle[i].xPos <= minX || triangle[i].xPos >= maxX ||
                triangle[i].yPos <= minY || triangle[i].yPos >= maxY) {
                triangle[i].xPos = (minX + maxX) / 2.0F;
                triangle[i].yPos = (minY + maxY) / 2.0F;

                // 중앙을 새 중심으로 잡고 반지름 0부터 다시 시작한다.
                triangle[i].speed = 0.003F + i * 0.001F;
                triangle[i].downLength = 0.0F;
                triangle[i].centerX = triangle[i].xPos;
                triangle[i].centerY = triangle[i].yPos;
                triangle[i].radius = 0.0F;
                triangle[i].angle = -i * 90.0F;
                triangle[i].rotation = triangle[i].angle - 90.0F;
                makeSpiralPath(i);
            }
        }
    }
}

void framebufferSize(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}



// 삼각형이 해당 사분면을 벗어나지 않는 최대 반너비와 반높이를 구한다.
GLfloat maxTriangleSize(int index)
{
    const GLfloat x = std::abs(triangle[index].xPos);
    const GLfloat y = std::abs(triangle[index].yPos);
    GLfloat maxSize = 0.3F;
    // 대각선으로 회전할 때 꼭짓점의 최대 거리(size * sqrt(2))를 고려한다.
    const bool diagonalRotation = moveMode == 1 || moveMode == 3 || moveMode == 4;
    const GLfloat scale = diagonalRotation ? 0.70710678F : 1.0F;

    if (x * scale < maxSize) maxSize = x * scale;
    if ((1.0F - x) * scale < maxSize) maxSize = (1.0F - x) * scale;
    if (y * scale < maxSize) maxSize = y * scale;
    if ((1.0F - y) * scale < maxSize) maxSize = (1.0F - y) * scale;
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
    resetMove(index);
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
    if (moveMode == 4) makeSpiralPath(index);
}

void drawVertices(const GLfloat* vertices, int count, GLenum primitive,
    GLclampf Red, GLclampf Green, GLclampf Blue)
{
    glUniform3f(colorLocation, Red, Green, Blue);
    // 곡선은 좌표가 많으므로 이번에 그릴 꼭짓점 수에 맞춰 버퍼를 채운다.
    glBufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(GLfloat), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(primitive, 0, count);
}

void drawMovePath(int index)
{
    if (moveMode == 0) return;

    if (moveMode == 4) {
        const int pathCount = static_cast<int>(triangle[index].path.size() / 2);
        if (pathCount >= 2) {
            drawVertices(triangle[index].path.data(), pathCount, GL_LINE_STRIP, 1.0F, 1.0F, 1.0F);
        }
        return;
    }

    const Triangle* Tri = &triangle[index];

    // 삼각형이 바라보는 방향으로 다음 방향 전환 지점까지 선을 그린다.
    const GLfloat radians = Tri->rotation * 0.01745329252F;
    const GLfloat dx = -std::sin(radians);
    const GLfloat dy = std::cos(radians);
    GLfloat length = 2.0F;
    if (moveMode == 2 && (Tri->move == 6 || Tri->move == 7)) {
        length = 0.1F - Tri->downLength;
    }

    // 실제 이동과 같은 경계에서 선을 자른다. 순간이동 경로는 잇지 않는다.
    const bool diagonalRotation = moveMode == 1 || moveMode == 3 || moveMode == 4;
    const GLfloat margin = Tri->size * (diagonalRotation ? 1.41421356F : 1.0F);
    const GLfloat minX = (index == 0 || index == 3) ? margin : -1.0F + margin;
    const GLfloat maxX = (index == 0 || index == 3) ? 1.0F - margin : -margin;
    const GLfloat minY = (index == 0 || index == 1) ? margin : -1.0F + margin;
    const GLfloat maxY = (index == 0 || index == 1) ? 1.0F - margin : -margin;

    // 가로 또는 세로 방향일 때 생기는 아주 작은 소수 오차는 무시한다.
    if (dx > 0.0001F) {
        const GLfloat wallLength = (maxX - Tri->xPos) / dx;
        if (wallLength < length) length = wallLength;
    }
    else if (dx < -0.0001F) {
        const GLfloat wallLength = (minX - Tri->xPos) / dx;
        if (wallLength < length) length = wallLength;
    }
    if (dy > 0.0001F) {
        const GLfloat wallLength = (maxY - Tri->yPos) / dy;
        if (wallLength < length) length = wallLength;
    }
    else if (dy < -0.0001F) {
        const GLfloat wallLength = (minY - Tri->yPos) / dy;
        if (wallLength < length) length = wallLength;
    }
    if (length <= 0.0F) return;

    const GLfloat vertices[] = {
        Tri->xPos, Tri->yPos,
        Tri->xPos + dx * length, Tri->yPos + dy * length
    };
    drawVertices(vertices, 2, GL_LINES, 1.0F, 1.0F, 1.0F);
}

void drawTriangle(Triangle* Tri)
{
    // 중심을 기준으로 세 꼭짓점을 회전시킨다.
    const GLfloat local[] = {
        -Tri->size, -Tri->size,
        Tri->size, -Tri->size,
        0.0F, Tri->size
    };
    const GLfloat radians = Tri->rotation * 0.01745329252F;
    const GLfloat cosine = std::cos(radians);
    const GLfloat sine = std::sin(radians);
    GLfloat vertices[6];
    for (int i = 0; i < 3; ++i) {
        const GLfloat x = local[i * 2];
        const GLfloat y = local[i * 2 + 1];
        vertices[i * 2] = Tri->xPos + x * cosine - y * sine;
        vertices[i * 2 + 1] = Tri->yPos + x * sine + y * cosine;
    }
    drawVertices(vertices, 3, polygonFill ? GL_TRIANGLES : GL_LINE_LOOP,
        Tri->Red, Tri->Green, Tri->Blue);
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
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4) {
        changeMove(key - GLFW_KEY_1 + 1);
    }
    else if (key == GLFW_KEY_A) {
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-9", nullptr, nullptr);
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
    colorLocation = glGetUniformLocation(shaderProgram, "drawColor");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // 좌표 저장 공간은 drawVertices에서 필요한 크기만큼 확보한다.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSize(window, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    resetTriangles();

    const GLfloat axes[] = { -1.0F, 0.0F, 1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 1.0F };
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        moveTriangles();
        glClear(GL_COLOR_BUFFER_BIT);
        drawVertices(axes, 4, GL_LINES, 0.6F, 0.6F, 0.6F);
        for (int i = 0; i < 4; ++i) {
            // 전체 스파이럴 경로는 뒤에, 움직이는 삼각형은 그 위에 그린다.
            if (moveMode == 4) drawMovePath(i);
            drawTriangle(&triangle[i]);
            if (moveMode != 4) drawMovePath(i);
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
