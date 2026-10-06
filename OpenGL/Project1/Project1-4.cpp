// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

struct Rectangle {
    GLclampf Red;
    GLclampf Green;
    GLclampf Blue;

    GLfloat xPos1;
    GLfloat yPos1;
    GLfloat xPos2;
    GLfloat yPos2;

    // 처음 클릭한 중심 위치를 저장한다.
    GLfloat startX{ 0.0F };
    GLfloat startY{ 0.0F };

    int move{ 0 };
    int size{ 0 };
    GLfloat downLenth{ 0.0F };
};


Rectangle rect[5]{};
int rectCount = 0;

bool move{ false };
bool moveZ{ false };
bool moveClock{ false };

bool moveSize{ false };
bool moveColor{ false };
double lastColorChangeTime{ 0.0 };

bool downLeft{ false };
bool upLeft{ true };
GLfloat mouseX{};
GLfloat mouseY{};


// 0.0~1.0 범위의 랜덤 색상을 만든다.
void setColor(GLclampf* Red, GLclampf* Green, GLclampf* Blue)
{
    *Red = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Green = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Blue = static_cast<GLclampf>(std::rand()) / RAND_MAX;
}

// min~max 범위의 랜덤 실수를 반환한다.
GLfloat randomFloat(GLfloat min, GLfloat max)
{
    const GLfloat randomValue =
        static_cast<GLfloat>(std::rand()) / RAND_MAX;

    return min + randomValue * (max - min);
}

// 마우스 좌표를 OpenGL 좌표인 -1.0~1.0으로 변환한다.
void getMousePos(GLFWwindow* window, GLfloat* x, GLfloat* y)
{
    double cursorX;
    double cursorY;

    int width;
    int height;

    glfwGetCursorPos(window, &cursorX, &cursorY);
    glfwGetWindowSize(window, &width, &height);

    if (width == 0 || height == 0) {
        return;
    }

    *x = static_cast<GLfloat>(
        cursorX / static_cast<double>(width) * 2.0 - 1.0
        );

    *y = static_cast<GLfloat>(
        1.0 - cursorY / static_cast<double>(height) * 2.0
        );
}

// 클릭한 위치를 중심으로 랜덤 사각형을 만든다.
void makeRect(GLfloat* x, GLfloat* y)
{
    // 최대 5개까지만 생성한다.
    if (rectCount >= 5) {
        return;
    }

    // 화면 가장자리에서는 생성하지 않는다.
    if (std::abs(*x) >= 0.8F || std::abs(*y) >= 0.8F) {
        return;
    }

    const GLfloat sizeX =
        randomFloat(0.1F, 1.0F - std::abs(*x));

    const GLfloat sizeY =
        randomFloat(0.1F, 1.0F - std::abs(*y));

    // 마우스로 클릭한 위치를 사각형의 중심으로 사용한다.
    const GLfloat centerX = *x;
    const GLfloat centerY = *y;

    rect[rectCount].xPos1 = centerX - sizeX / 2.0F;
    rect[rectCount].yPos1 = centerY - sizeY / 2.0F;
    rect[rectCount].xPos2 = centerX + sizeX / 2.0F;
    rect[rectCount].yPos2 = centerY + sizeY / 2.0F;
    rect[rectCount].startX = centerX;
    rect[rectCount].startY = centerY;

    setColor(
        &rect[rectCount].Red,
        &rect[rectCount].Green,
        &rect[rectCount].Blue
    );

    ++rectCount;
}

// 위치, 크기, 색상 애니메이션을 모두 멈춘다.
void stopAnimations()
{
    move = false;
    moveZ = false;
    moveClock = false;
    moveSize = false;
    moveColor = false;
}

void moveRect() {
    const GLfloat speed = 0.01F;

    if (moveColor) {
        const double currentTime = glfwGetTime();

        // 0.3초마다 각 사각형의 색상을 랜덤하게 바꾼다.
        if (currentTime - lastColorChangeTime >= 0.3) {
            for (int i = 0; i < rectCount; ++i) {
                setColor(&rect[i].Red, &rect[i].Green, &rect[i].Blue);
            }
            lastColorChangeTime = currentTime;
        }
    }

    if (moveSize) {
        for (int i = 0; i < rectCount; ++i) {

            if (rect[i].xPos2 - rect[i].xPos1 < speed ||
                rect[i].yPos2 - rect[i].yPos1 < speed) {
                rect[i].size = 1;
            }
            else if (rect[i].xPos2 - rect[i].xPos1 > 1.0F ||
                rect[i].yPos2 - rect[i].yPos1 > 1.0F) {
                rect[i].size = 0;
            }

            if (rect[i].size == 0) {
                rect[i].xPos1 += speed;
                rect[i].xPos2 -= speed;
                rect[i].yPos1 += speed;
                rect[i].yPos2 -= speed;
            }
            else  if (rect[i].size == 1) {
                rect[i].xPos1 -= speed;
                rect[i].xPos2 += speed;
                rect[i].yPos1 -= speed;
                rect[i].yPos2 += speed;
            }
        }
    }

    if (move) {
        // 생성된 사각형만 움직인다.
        for (int i = 0; i < rectCount; ++i) {
            // 0: 오른쪽 위
            if (rect[i].move == 0) {
                rect[i].xPos1 += speed;
                rect[i].yPos1 += speed;
                rect[i].xPos2 += speed;
                rect[i].yPos2 += speed;

                const bool hitRight = rect[i].xPos2 >= 1.0F;
                const bool hitTop = rect[i].yPos2 >= 1.0F;

                if (hitRight && hitTop) {
                    rect[i].move = 3;
                }
                else if (hitRight) {
                    rect[i].move = 1;
                }
                else if (hitTop) {
                    rect[i].move = 2;
                }
            }

            // 1: 왼쪽 위
            else if (rect[i].move == 1) {
                rect[i].xPos1 -= speed;
                rect[i].yPos1 += speed;
                rect[i].xPos2 -= speed;
                rect[i].yPos2 += speed;

                const bool hitLeft = rect[i].xPos1 <= -1.0F;
                const bool hitTop = rect[i].yPos2 >= 1.0F;

                if (hitLeft && hitTop) {
                    rect[i].move = 2;
                }
                else if (hitLeft) {
                    rect[i].move = 0;
                }
                else if (hitTop) {
                    rect[i].move = 3;
                }
            }

            // 2: 오른쪽 아래
            else if (rect[i].move == 2) {
                rect[i].xPos1 += speed;
                rect[i].yPos1 -= speed;
                rect[i].xPos2 += speed;
                rect[i].yPos2 -= speed;

                const bool hitRight = rect[i].xPos2 >= 1.0F;
                const bool hitBottom = rect[i].yPos1 <= -1.0F;

                if (hitRight && hitBottom) {
                    rect[i].move = 1;
                }
                else if (hitRight) {
                    rect[i].move = 3;
                }
                else if (hitBottom) {
                    rect[i].move = 0;
                }
            }

            // 3: 왼쪽 아래
            else if (rect[i].move == 3) {
                rect[i].xPos1 -= speed;
                rect[i].yPos1 -= speed;
                rect[i].xPos2 -= speed;
                rect[i].yPos2 -= speed;

                const bool hitLeft = rect[i].xPos1 <= -1.0F;
                const bool hitBottom = rect[i].yPos1 <= -1.0F;

                if (hitLeft && hitBottom) {
                    rect[i].move = 0;
                }
                else if (hitLeft) {
                    rect[i].move = 2;
                }
                else if (hitBottom) {
                    rect[i].move = 1;
                }
            }
        }
    }
    else if (moveZ) {
        for (int i = 0; i < rectCount; ++i) {
            // 4: 오른쪽으로 이동
            if (rect[i].move == 4) {
                rect[i].xPos1 += speed;
                rect[i].xPos2 += speed;

                if (rect[i].xPos2 >= 1.0F) {
                    rect[i].move = 6;
                    rect[i].downLenth = 0.0F;
                }
            }

            // 5: 왼쪽으로 이동
            else if (rect[i].move == 5) {
                rect[i].xPos1 -= speed;
                rect[i].xPos2 -= speed;

                if (rect[i].xPos1 <= -1.0F) {
                    rect[i].move = 7;
                    rect[i].downLenth = 0.0F;
                }
            }

            else if (rect[i].move == 6) {
                rect[i].yPos1 -= speed;
                rect[i].yPos2 -= speed;
                rect[i].downLenth += speed;

                if (rect[i].yPos1 <= -1.0F) {
                    const GLfloat height = rect[i].yPos2 - rect[i].yPos1;

                    rect[i].yPos2 = 1.0F;
                    rect[i].yPos1 = 1.0F - height;

                    rect[i].downLenth = 0.0F;
                    rect[i].move = 5;
                }
                else if (rect[i].downLenth >= 0.1F) {
                    rect[i].downLenth = 0.0F;
                    rect[i].move = 5;
                }
            }
            else if (rect[i].move == 7) {
                rect[i].yPos1 -= speed;
                rect[i].yPos2 -= speed;
                rect[i].downLenth += speed;

                if (rect[i].yPos1 <= -1.0F) {
                    const GLfloat height = rect[i].yPos2 - rect[i].yPos1;

                    rect[i].yPos2 = 1.0F;
                    rect[i].yPos1 = 1.0F - height;

                    rect[i].downLenth = 0.0F;
                    rect[i].move = 4;
                }
                else if (rect[i].downLenth >= 0.1F) {
                    rect[i].downLenth = 0.0F;
                    rect[i].move = 4;
                }
            }
        }
    }
    else if (moveClock) {
        for (int i = 0; i < rectCount; ++i) {
            // 8:오른쪽 이동
            if (rect[i].move == 8) {
                rect[i].xPos1 += speed;
                rect[i].xPos2 += speed;

                if (rect[i].xPos2 >= 1.0F) {
                    rect[i].move = 9;
                }
            }
            else if (rect[i].move == 9) {
                rect[i].yPos1 -= speed;
                rect[i].yPos2 -= speed;

                if (rect[i].yPos1 <= -1.0F) {
                    rect[i].move = 10;
                }
            }
            else if (rect[i].move == 10) {
                rect[i].xPos1 -= speed;
                rect[i].xPos2 -= speed;

                if (rect[i].xPos1 <= -1.0F) {
                    rect[i].move = 11;
                }
            }
            else if (rect[i].move == 11) {
                rect[i].yPos1 += speed;
                rect[i].yPos2 += speed;

                if (rect[i].yPos2 >= 1.0F) {
                    rect[i].move = 8;
                }
            }
        }
    }


}

// 사각형 하나를 그린다.
void drawRect(Rectangle* Rect)
{
    glColor3f(Rect->Red, Rect->Green, Rect->Blue);

    glRectf(
        Rect->xPos1,
        Rect->yPos1,
        Rect->xPos2,
        Rect->yPos2
    );
}

// 키를 처음 눌렀을 때만 처리한다. 길게 눌러도 토글이 반복되지 않는다.
void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_1) {
        move = !move;
        moveZ = false;
        moveClock = false;
        for (int i = 0; i < 5; ++i) {
            rect[i].move = rand() % 4;
        }
    }
    else if (key == GLFW_KEY_2) {
        move = false;
        moveZ = !moveZ;
        moveClock = false;
        for (int i = 0; i < 5; ++i) {
            rect[i].move = (rand() % 2) + 4;
        }
    }
    else if (key == GLFW_KEY_3) {
        move = false;
        moveZ = false;
        moveClock = !moveClock;
        for (int i = 0; i < 5; ++i) {
            rect[i].move = (rand() % 2) + 8;
        }
    }
    else if (key == GLFW_KEY_4) {
        moveSize = !moveSize;
        for (int i = 0; i < 5; ++i) {
            rect[i].size = rand() % 2;
        }
    }
    else if (key == GLFW_KEY_5) {
        moveColor = !moveColor;
        lastColorChangeTime = glfwGetTime();
    }
    else if (key == GLFW_KEY_S) {
        stopAnimations();
    }
    else if (key == GLFW_KEY_M) {
        // 복귀한 위치에 머무르도록 위치 이동을 멈춘다.
        move = false;
        moveZ = false;
        moveClock = false;

        for (int i = 0; i < rectCount; ++i) {
            // 현재 크기를 유지하며 처음 클릭한 중심 위치로 돌아간다.
            const GLfloat halfWidth = (rect[i].xPos2 - rect[i].xPos1) / 2.0F;
            const GLfloat halfHeight = (rect[i].yPos2 - rect[i].yPos1) / 2.0F;

            rect[i].xPos1 = rect[i].startX - halfWidth;
            rect[i].xPos2 = rect[i].startX + halfWidth;
            rect[i].yPos1 = rect[i].startY - halfHeight;
            rect[i].yPos2 = rect[i].startY + halfHeight;
            rect[i].downLenth = 0.0F;
        }
    }
    else if (key == GLFW_KEY_R) {
        stopAnimations();
        for (int i = 0; i < 5; ++i) {
            rect[i] = Rectangle{};
        }
        rectCount = 0;
        lastColorChangeTime = glfwGetTime();
        downLeft = false;
        // 마우스를 누르고 있었다면 놓은 뒤 다시 클릭해야 생성한다.
        upLeft = false;
    }
    else if (key == GLFW_KEY_Q) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

int main()
{
    // 실행할 때마다 다른 난수가 나오도록 초기화한다.
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "GLFW initialization failed.\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_COMPAT_PROFILE
    );

    GLFWwindow* window = glfwCreateWindow(
        800,
        600,
        "Project1-4",
        nullptr,
        nullptr
    );

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

    glViewport(0, 0, 800, 600);

    // 창 크기가 바뀌면 그리기 영역도 변경한다.
    glfwSetFramebufferSizeCallback(
        window,
        [](GLFWwindow*, int width, int height) {
            glViewport(0, 0, width, height);
        }
    );

    // 배경색은 검정색이다.
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    // 키 콜백은 반복문에 들어가기 전에 한 번만 등록한다.
    glfwSetKeyCallback(window, keyCallback);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (upLeft) {
                downLeft = true;
                upLeft = false;

                getMousePos(window, &mouseX, &mouseY);
                makeRect(&mouseX, &mouseY);
            }
        }
        else {
            downLeft = false;
            upLeft = true;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        moveRect();
        // 지금까지 생성된 모든 사각형을 그린다.
        for (int i = 0; i < rectCount; ++i) {
            drawRect(&rect[i]);
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
