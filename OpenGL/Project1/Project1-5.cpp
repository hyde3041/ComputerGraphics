// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <ctime>
#include <iostream>

struct Rectangle {
    GLclampf Red{};
    GLclampf Green{};
    GLclampf Blue{};

    GLfloat xPos1{};
    GLfloat yPos1{};
    GLfloat xPos2{};
    GLfloat yPos2{};

    bool exist{ true };
    bool canErase{ true };
};

// 처음 40개와 오른쪽 클릭으로 추가하는 10개를 저장한다.
Rectangle rect[50]{};
Rectangle eraser{};
int rectCount{};
int addCount{};

const GLfloat rectSize = 0.08F;
GLfloat eraserSize = rectSize * 2.0F;

bool eraserOn{ false };
bool leftFlag{ false };
bool rightFlag{ false };

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

void setPos(Rectangle* Rect, GLfloat x, GLfloat y, GLfloat size)
{
    Rect->xPos1 = x - size / 2.0F;
    Rect->yPos1 = y - size / 2.0F;
    Rect->xPos2 = x + size / 2.0F;
    Rect->yPos2 = y + size / 2.0F;
}

bool getMousePos(GLFWwindow* window, GLfloat* x, GLfloat* y)
{
    double mouseX;
    double mouseY;
    int width;
    int height;

    glfwGetCursorPos(window, &mouseX, &mouseY);
    glfwGetWindowSize(window, &width, &height);

    if (width == 0 || height == 0) {
        return false;
    }

    *x = static_cast<GLfloat>(mouseX / width * 2.0 - 1.0);
    *y = static_cast<GLfloat>(1.0 - mouseY / height * 2.0);
    return true;
}

void resetRect()
{
    rectCount = 20 + std::rand() % 21;
    addCount = 0;
    eraserOn = false;
    eraserSize = rectSize * 2.0F;
    eraser = Rectangle{};

    for (int i = 0; i < rectCount; ++i) {
        rect[i] = Rectangle{};
        GLfloat x = randomFloat(-1.0F + rectSize / 2.0F, 1.0F - rectSize / 2.0F);
        GLfloat y = randomFloat(-1.0F + rectSize / 2.0F, 1.0F - rectSize / 2.0F);

        setPos(&rect[i], x, y, rectSize);
        setColor(&rect[i].Red, &rect[i].Green, &rect[i].Blue);
    }
}

void changeEraserSize(GLfloat size)
{
    GLfloat centerX = (eraser.xPos1 + eraser.xPos2) / 2.0F;
    GLfloat centerY = (eraser.yPos1 + eraser.yPos2) / 2.0F;

    eraserSize += size;
    // 계속 줄이더라도 지우개의 크기가 0이 되지 않게 한다.
    if (eraserSize < rectSize / 2.0F) {
        eraserSize = rectSize / 2.0F;
    }
    setPos(&eraser, centerX, centerY, eraserSize);
}

bool overlapRect(Rectangle* first, Rectangle* second)
{
    if (first->xPos2 < second->xPos1 || first->xPos1 > second->xPos2) {
        return false;
    }
    if (first->yPos2 < second->yPos1 || first->yPos1 > second->yPos2) {
        return false;
    }
    return true;
}

void startEraser(GLfloat x, GLfloat y)
{
    eraser = Rectangle{};
    eraserSize = rectSize * 2.0F;
    setPos(&eraser, x, y, eraserSize);
    eraserOn = true;
}

void stopEraser()
{
    eraserOn = false;
    // 좌표와 색상은 그대로 두고, 숨겼던 사각형을 다시 보이게 한다.
    for (int i = 0; i < rectCount; ++i) {
        rect[i].exist = true;
        rect[i].canErase = true;
    }
}

void eraseRect()
{
    for (int i = 0; i < rectCount; ++i) {
        if (rect[i].exist == false) {
            continue;
        }

        bool overlap = overlapRect(&eraser, &rect[i]);
        // 지우개 안에 새로 만든 사각형은 밖으로 나갔다가 다시 닿을 때 지운다.
        if (rect[i].canErase == false) {
            if (overlap == false) {
                rect[i].canErase = true;
            }
            continue;//이건왜?
        }

        if (overlap) {
            rect[i].exist = false;
            eraser.Red = rect[i].Red;
            eraser.Green = rect[i].Green;
            eraser.Blue = rect[i].Blue;
            changeEraserSize(0.02F);
        }
    }
}

void makeRect(GLfloat x, GLfloat y)
{
    if (addCount >= 10) {
        return;
    }

    rect[rectCount] = Rectangle{};
    setPos(&rect[rectCount], x, y, rectSize);
    setColor(&rect[rectCount].Red, &rect[rectCount].Green, &rect[rectCount].Blue);

    if (eraserOn) {
        changeEraserSize(-0.02F);
        rect[rectCount].canErase = !overlapRect(&eraser, &rect[rectCount]);
    }
    ++rectCount;
    ++addCount;
}

void drawRect(Rectangle* Rect)
{
    glColor3f(Rect->Red, Rect->Green, Rect->Blue);
    glRectf(Rect->xPos1, Rect->yPos1, Rect->xPos2, Rect->yPos2);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_R) {
        resetRect();
        // 리셋할 때 누르고 있던 버튼은 놓은 뒤 다시 눌러야 처리한다.
        leftFlag = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        rightFlag = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
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
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-5", nullptr, nullptr);
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
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
        glViewport(0, 0, width, height);
    });
    glfwSetKeyCallback(window, keyCallback);
    glClearColor(1.0F, 1.0F, 1.0F, 1.0F);

    resetRect();

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();

        GLfloat mouseX{};
        GLfloat mouseY{};
        bool mouseValid = getMousePos(window, &mouseX, &mouseY);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (mouseValid) {
                if (leftFlag == false) {
                    startEraser(mouseX, mouseY);
                    leftFlag = true;
                }
                if (eraserOn) {
                    setPos(&eraser, mouseX, mouseY, eraserSize);
                    eraseRect();
                }
            }
        }
        else {
            if (eraserOn) {
                stopEraser();
            }
            leftFlag = false;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            if (rightFlag == false && mouseValid) {
                makeRect(mouseX, mouseY);
                rightFlag = true;
            }
        }
        else {
            rightFlag = false;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        for (int i = 0; i < rectCount; ++i) {
            if (rect[i].exist) {
                drawRect(&rect[i]);
            }
        }
        if (eraserOn) {
            drawRect(&eraser);
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
