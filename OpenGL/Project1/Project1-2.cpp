#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdlib>

typedef class Rectangle {
public:
    GLclampf Red;
    GLclampf Green;
    GLclampf Blue;

    GLfloat xPos1;
    GLfloat yPos1;
    GLfloat xPos2;
    GLfloat yPos2;

    int index{ -1 };
    bool exist{ false };
}Rectangle;

int width;
int height;

Rectangle backGround[4];
Rectangle smallRect[20];

int count[4]{};
int selectRect{ -1 };

bool keyFlag[4]{};
bool mouseFlag{ false };
bool plusFlag{ false };
bool minusFlag{ false };
bool cFlag{ false };
bool rFlag{ false };

void framebufferSize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void setColor(GLclampf* Red, GLclampf* Green, GLclampf* Blue) {
    *Red = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Green = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Blue = static_cast<GLclampf>(std::rand()) / RAND_MAX;
}

void setPos(int Pos, GLfloat Size) {
    if (count[Pos] >= 5) {
        return;
    }

    int index = Pos * 5 + count[Pos];

    GLfloat centerX;
    GLfloat centerY;

    if (Pos == 0) {
        centerX = 0.5f;
        centerY = 0.5f;
    }
    else if (Pos == 1) {
        centerX = -0.5f;
        centerY = 0.5f;
    }
    else if (Pos == 2) {
        centerX = -0.5f;
        centerY = -0.5f;
    }
    else {
        centerX = 0.5f;
        centerY = -0.5f;
    }

    smallRect[index].xPos1 = centerX - Size / 2.0f;
    smallRect[index].yPos1 = centerY - Size / 2.0f;
    smallRect[index].xPos2 = centerX + Size / 2.0f;
    smallRect[index].yPos2 = centerY + Size / 2.0f;

    setColor(&smallRect[index].Red, &smallRect[index].Green, &smallRect[index].Blue);

    smallRect[index].index = index;
    smallRect[index].exist = true;

    ++count[Pos];
}

void firstRect() {
    backGround[0].xPos1 = 0;
    backGround[0].yPos1 = 0;
    backGround[0].xPos2 = 1;
    backGround[0].yPos2 = 1;

    backGround[1].xPos1 = -1;
    backGround[1].yPos1 = 0;
    backGround[1].xPos2 = 0;
    backGround[1].yPos2 = 1;

    backGround[2].xPos1 = -1;
    backGround[2].yPos1 = -1;
    backGround[2].xPos2 = 0;
    backGround[2].yPos2 = 0;

    backGround[3].xPos1 = 0;
    backGround[3].yPos1 = -1;
    backGround[3].xPos2 = 1;
    backGround[3].yPos2 = 0;

    for (int i = 0; i < 4; ++i) {
        setColor(&backGround[i].Red, &backGround[i].Green, &backGround[i].Blue);
    }
}

void changeSize(int index, GLfloat size) {
    if (index == -1) {
        return;
    }

    GLfloat newX1 = smallRect[index].xPos1 - size;
    GLfloat newY1 = smallRect[index].yPos1 - size;
    GLfloat newX2 = smallRect[index].xPos2 + size;
    GLfloat newY2 = smallRect[index].yPos2 + size;

    if (newX2 - newX1 <= 0.1f || newY2 - newY1 <= 0.1f) {
        return;
    }

    if (newX2 - newX1 >= 1.0f || newY2 - newY1 >= 1.0f) {
        return;
    }

    smallRect[index].xPos1 = newX1;
    smallRect[index].yPos1 = newY1;
    smallRect[index].xPos2 = newX2;
    smallRect[index].yPos2 = newY2;
}

void resetRect() {
    for (int i = 0; i < 20; ++i) {
        smallRect[i] = Rectangle();
    }

    for (int i = 0; i < 4; ++i) {
        count[i] = 0;
        setColor(&backGround[i].Red, &backGround[i].Green, &backGround[i].Blue);
    }

    selectRect = -1;
}

void drawRect(Rectangle* Rect) {
    glColor3f(Rect->Red, Rect->Green, Rect->Blue);
    glRectf(Rect->xPos1, Rect->yPos1, Rect->xPos2, Rect->yPos2);

    if (selectRect != -1 && Rect->index == selectRect) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(4.0f);

        glBegin(GL_LINE_LOOP);
        glVertex2f(Rect->xPos1, Rect->yPos1);
        glVertex2f(Rect->xPos2, Rect->yPos1);
        glVertex2f(Rect->xPos2, Rect->yPos2);
        glVertex2f(Rect->xPos1, Rect->yPos2);
        glEnd();
    }
}

void selectRectangle(GLFWwindow* window) {
    double mouseX;
    double mouseY;

    glfwGetCursorPos(window, &mouseX, &mouseY);
    glfwGetWindowSize(window, &width, &height);

    GLfloat x = static_cast<GLfloat>(mouseX / width * 2.0 - 1.0);
    GLfloat y = static_cast<GLfloat>(1.0 - mouseY / height * 2.0);

    selectRect = -1;

    for (int i = 19; i >= 0; --i) {
        if (smallRect[i].exist == false) {
            continue;
        }

        if (x >= smallRect[i].xPos1 && x <= smallRect[i].xPos2 &&
            y >= smallRect[i].yPos1 && y <= smallRect[i].yPos2) {
            selectRect = smallRect[i].index;
            break;
        }
    }
}

int main()
{
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "GLFW initialization failed.\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-2", nullptr, nullptr);

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
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    firstRect();

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        int key[4]{
            GLFW_KEY_1,
            GLFW_KEY_2,
            GLFW_KEY_3,
            GLFW_KEY_4
        };

        for (int i = 0; i < 4; ++i) {
            if (glfwGetKey(window, key[i]) == GLFW_PRESS) {
                if (keyFlag[i] == false) {
                    GLfloat size = 0.2f + static_cast<GLfloat>(std::rand()) / RAND_MAX * 0.3f;
                    setPos(i, size);
                    keyFlag[i] = true;
                }
            }
            else {
                keyFlag[i] = false;
            }
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (mouseFlag == false) {
                selectRectangle(window);
                mouseFlag = true;
            }
        }
        else {
            mouseFlag = false;
        }
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
            if (plusFlag == false) {
                changeSize(selectRect, 0.05f);
                plusFlag = true;
            }
        }
        else {
            plusFlag = false;
        }

        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
            if (minusFlag == false) {
                changeSize(selectRect, -0.05f);
                minusFlag = true;
            }
        }
        else {
            minusFlag = false;
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            if (cFlag == false) {
                if (selectRect != -1) {
                    setColor(&smallRect[selectRect].Red,
                        &smallRect[selectRect].Green,
                        &smallRect[selectRect].Blue);
                }
                cFlag = true;
            }
        }
        else {
            cFlag = false;
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            if (rFlag == false) {
                resetRect();
                rFlag = true;
            }
        }
        else {
            rFlag = false;
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glfwGetWindowSize(window, &width, &height);

        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < 4; ++i) {
            drawRect(&backGround[i]);
        }

        for (int i = 0; i < 20; ++i) {
            if (smallRect[i].exist) {
                drawRect(&smallRect[i]);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}