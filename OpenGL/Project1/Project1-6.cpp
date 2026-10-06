// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cstdio>

struct Rectangle {
    GLclampf Red;
    GLclampf Green;
    GLclampf Blue;

    GLfloat xPos1;
    GLfloat yPos1;
    GLfloat xPos2;
    GLfloat yPos2;

    int move;
    bool bright;
};

Rectangle rect[10]{};
Rectangle piece[80]{};
int rectCount{};
int pieceCount{};
// 0: 상하좌우, 1: 대각선, 2: 같은 방향, 3: 여덟 방향
int mode{};

// 0: 왼쪽, 1: 오른쪽, 2: 위, 3: 아래
// 4: 왼쪽 위, 5: 오른쪽 위, 6: 왼쪽 아래, 7: 오른쪽 아래
const GLfloat moveX[8] = {
    -1.0F, 1.0F, 0.0F, 0.0F, -0.7071F, 0.7071F, -0.7071F, 0.7071F
};
const GLfloat moveY[8] = {
    0.0F, 0.0F, 1.0F, -1.0F, 0.7071F, 0.7071F, -0.7071F, -0.7071F
};

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

// 처음 실행하거나 r을 누르면 5~10개의 사각형을 만든다.
void makeRect()
{
    rectCount = std::rand() % 6 + 5;
    pieceCount = 0;

    for (int i = 0; i < rectCount; ++i) {
        const GLfloat sizeX = randomFloat(0.25F, 0.5F);
        const GLfloat sizeY = randomFloat(0.25F, 0.5F);
        const GLfloat centerX = randomFloat(-0.9F + sizeX / 2.0F, 0.9F - sizeX / 2.0F);
        const GLfloat centerY = randomFloat(-0.9F + sizeY / 2.0F, 0.9F - sizeY / 2.0F);

        rect[i].xPos1 = centerX - sizeX / 2.0F;
        rect[i].yPos1 = centerY - sizeY / 2.0F;
        rect[i].xPos2 = centerX + sizeX / 2.0F;
        rect[i].yPos2 = centerY + sizeY / 2.0F;
        setColor(&rect[i].Red, &rect[i].Green, &rect[i].Blue);
    }
}

void getMousePos(GLFWwindow* window, GLfloat* x, GLfloat* y)
{
    double cursorX;
    double cursorY;
    int width;
    int height;

    glfwGetCursorPos(window, &cursorX, &cursorY);
    glfwGetWindowSize(window, &width, &height);

    if (width <= 0 || height <= 0) {
        // 창이 최소화된 상태에서는 사각형을 선택하지 않는다.
        *x = 2.0F;
        *y = 2.0F;
        return;
    }

    *x = static_cast<GLfloat>(cursorX / width * 2.0 - 1.0);
    *y = static_cast<GLfloat>(1.0 - cursorY / height * 2.0);
}

int findRect(GLfloat x, GLfloat y)
{
    // 겹친 경우 마지막에 그린 사각형부터 찾는다.
    for (int i = rectCount - 1; i >= 0; --i) {
        if (x >= rect[i].xPos1 && x <= rect[i].xPos2 &&
            y >= rect[i].yPos1 && y <= rect[i].yPos2) {
            return i;
        }
    }

    return -1;
}

void deleteRect(int index)
{
    for (int i = index; i < rectCount - 1; ++i) {
        rect[i] = rect[i + 1];
    }
    --rectCount;
}

void deletePiece(int index)
{
    for (int i = index; i < pieceCount - 1; ++i) {
        piece[i] = piece[i + 1];
    }
    --pieceCount;
}

void splitRect(int index)
{
    if (index < 0 || index >= rectCount) {
        return;
    }

    // 숫자 키로 선택한 모드에 따라 네 조각 또는 여덟 조각으로 나눈다.
    const int columns = (mode == 3) ? 4 : 2;
    const int splitCount = columns * 2;

    if (pieceCount + splitCount > 80) {
        return;
    }

    const Rectangle oldRect = rect[index];
    const GLfloat sizeX = (oldRect.xPos2 - oldRect.xPos1) / columns;
    const GLfloat sizeY = (oldRect.yPos2 - oldRect.yPos1) / 2.0F;
    const int sameMove = std::rand() % 8;
    const bool bright = std::rand() % 2 == 0;
    // 왼쪽 아래부터 행 단위로 생성한 조각이 바깥 방향으로 움직인다.
    const int straightMove[4] = { 0, 3, 2, 1 };
    const int diagonalMove[4] = { 6, 7, 4, 5 };
    const int eightMove[8] = { 0, 6, 3, 7, 4, 2, 5, 1 };

    for (int i = 0; i < splitCount; ++i) {
        piece[pieceCount] = oldRect;

        // 기본은 2행 2열의 네 조각, 여덟 방향은 2행 4열의 여덟 조각이다.
        piece[pieceCount].xPos1 = oldRect.xPos1 + (i % columns) * sizeX;
        piece[pieceCount].yPos1 = oldRect.yPos1 + (i / columns) * sizeY;
        piece[pieceCount].xPos2 = piece[pieceCount].xPos1 + sizeX;
        piece[pieceCount].yPos2 = piece[pieceCount].yPos1 + sizeY;
        piece[pieceCount].bright = bright;

        if (mode == 0) {
            piece[pieceCount].move = straightMove[i];
        }
        else if (mode == 1) {
            piece[pieceCount].move = diagonalMove[i];
        }
        else if (mode == 2) {
            piece[pieceCount].move = sameMove;
        }
        else {
            piece[pieceCount].move = eightMove[i];
        }

        ++pieceCount;
    }

    // 원래 사각형을 지우고 나누어진 조각만 남긴다.
    deleteRect(index);
    std::cout << "Animation " << mode + 1 << ": " << splitCount << " pieces\n";
}

void moveRect()
{
    const GLfloat speed = 0.002F;  // 한 번에 이동할 거리
    const GLfloat shrink = 0.0002F; // 각 변을 안쪽으로 줄일 거리

    for (int i = 0; i < pieceCount; ++i) {
        const int direction = piece[i].move;
        const GLfloat dx = moveX[direction] * speed;
        const GLfloat dy = moveY[direction] * speed;

        // 두 좌표를 같은 방향으로 옮기면서, 양쪽 변을 안쪽으로 줄인다.
        piece[i].xPos1 += dx + shrink;
        piece[i].xPos2 += dx - shrink;
        piece[i].yPos1 += dy + shrink;
        piece[i].yPos2 += dy - shrink;

        // 가로나 세로가 충분히 작아지면 조각을 지운다.
        if (piece[i].xPos2 - piece[i].xPos1 <= 0.005F ||
            piece[i].yPos2 - piece[i].yPos1 <= 0.005F) {
            deletePiece(i);
            --i; // 앞으로 당겨진 조각도 이번 반복에서 처리한다.
            continue;
        }

        // 밝아질 때는 0.001씩 더하고, 어두워질 때는 0.001씩 뺀다.
        const GLclampf colorChange = piece[i].bright ? 0.001F : -0.001F;
        piece[i].Red += colorChange;
        piece[i].Green += colorChange;
        piece[i].Blue += colorChange;

        // RGB 값은 0~1 범위를 유지한다.
        if (piece[i].Red > 1.0F) piece[i].Red = 1.0F;
        if (piece[i].Red < 0.0F) piece[i].Red = 0.0F;
        if (piece[i].Green > 1.0F) piece[i].Green = 1.0F;
        if (piece[i].Green < 0.0F) piece[i].Green = 0.0F;
        if (piece[i].Blue > 1.0F) piece[i].Blue = 1.0F;
        if (piece[i].Blue < 0.0F) piece[i].Blue = 0.0F;
    }
}

void drawRect(Rectangle* Rect)
{
    glColor3f(Rect->Red, Rect->Green, Rect->Blue);
    glRectf(Rect->xPos1, Rect->yPos1, Rect->xPos2, Rect->yPos2);
}

void mouseCallback(GLFWwindow* window, int button, int action, int)
{
    // 누르는 순간만 처리하여 길게 눌러도 한 번만 나눈다.
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
        return;
    }

    GLfloat mouseX;
    GLfloat mouseY;
    getMousePos(window, &mouseX, &mouseY);
    splitRect(findRect(mouseX, mouseY));
}

void updateTitle(GLFWwindow* window)
{
    const char* modeName[4] = { "Up/Down/Left/Right", "Diagonals", "Same direction", "8 directions" };
    char title[160]{};
    std::snprintf(title, sizeof(title),
        "Project1-6 | Mode %d: %s | 1-4 / Numpad 1-4: Mode | R: Reset | Q: Quit",
        mode + 1, modeName[mode]);
    glfwSetWindowTitle(window, title);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) {
        return;
    }

    // 키패드와 키보드 위쪽 숫자 키를 모두 사용할 수 있다.
    if (key >= GLFW_KEY_KP_1 && key <= GLFW_KEY_KP_4) {
        mode = key - GLFW_KEY_KP_1;
        updateTitle(window);
    }
    else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4) {
        mode = key - GLFW_KEY_1;
        updateTitle(window);
    }
    else if (key == GLFW_KEY_R) {
        makeRect();
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
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-6", nullptr, nullptr);
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

    glfwSwapInterval(1);
    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetWindowAspectRatio(window, 4, 3);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    makeRect();
    updateTitle(window);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        moveRect();
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < rectCount; ++i) {
            drawRect(&rect[i]);
        }
        for (int i = 0; i < pieceCount; ++i) {
            drawRect(&piece[i]);
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
