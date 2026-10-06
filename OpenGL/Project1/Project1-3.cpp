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
}Rectangle;

int width;
int height;

Rectangle rect[20];

int rectCount{};
int selectRect{ -1 };

bool aFlag{ false };
bool leftFlag{ false };
bool rightFlag{ false };
bool dragging{ false };

GLfloat dragX{};
GLfloat dragY{};

void setColor(GLclampf* Red, GLclampf* Green, GLclampf* Blue) {
    *Red = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Green = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Blue = static_cast<GLclampf>(std::rand()) / RAND_MAX;
}

GLfloat randomFloat(GLfloat min, GLfloat max) {
    return min + static_cast<GLfloat>(std::rand()) / RAND_MAX * (max - min);
}

void makeRect() {
    if (rectCount >= 10) {
        return;
    }

    GLfloat sizeX = randomFloat(0.2f, 0.5f);
    GLfloat sizeY = randomFloat(0.2f, 0.5f);

    GLfloat centerX = randomFloat(-1.0f + sizeX / 2.0f, 1.0f - sizeX / 2.0f);
    GLfloat centerY = randomFloat(-1.0f + sizeY / 2.0f, 1.0f - sizeY / 2.0f);

    rect[rectCount].xPos1 = centerX - sizeX / 2.0f;
    rect[rectCount].yPos1 = centerY - sizeY / 2.0f;
    rect[rectCount].xPos2 = centerX + sizeX / 2.0f;
    rect[rectCount].yPos2 = centerY + sizeY / 2.0f;

    setColor(&rect[rectCount].Red, &rect[rectCount].Green, &rect[rectCount].Blue);

    ++rectCount;
}

void drawRect(Rectangle* Rect) {
    glColor3f(Rect->Red, Rect->Green, Rect->Blue);
    glRectf(Rect->xPos1, Rect->yPos1, Rect->xPos2, Rect->yPos2);
}

void getMousePos(GLFWwindow* window, GLfloat* x, GLfloat* y) {
    double mouseX;
    double mouseY;

    glfwGetCursorPos(window, &mouseX, &mouseY);
    glfwGetWindowSize(window, &width, &height);

    *x = static_cast<GLfloat>(mouseX / width * 2.0 - 1.0);
    *y = static_cast<GLfloat>(1.0 - mouseY / height * 2.0);
}

int findRect(GLfloat x, GLfloat y) {
    for (int i = rectCount - 1; i >= 0; --i) {
        if (x >= rect[i].xPos1 && x <= rect[i].xPos2 &&
            y >= rect[i].yPos1 && y <= rect[i].yPos2) {
            return i;
        }
    }

    return -1;
}

void moveRect(GLFWwindow* window) {
    if (selectRect == -1) {
        return;
    }

    GLfloat mouseX;
    GLfloat mouseY;

    getMousePos(window, &mouseX, &mouseY);

    GLfloat sizeX = rect[selectRect].xPos2 - rect[selectRect].xPos1;
    GLfloat sizeY = rect[selectRect].yPos2 - rect[selectRect].yPos1;

    GLfloat centerX = mouseX - dragX;
    GLfloat centerY = mouseY - dragY;

    if (centerX - sizeX / 2.0f < -1.0f) {
        centerX = -1.0f + sizeX / 2.0f;
    }
    if (centerX + sizeX / 2.0f > 1.0f) {
        centerX = 1.0f - sizeX / 2.0f;
    }
    if (centerY - sizeY / 2.0f < -1.0f) {
        centerY = -1.0f + sizeY / 2.0f;
    }
    if (centerY + sizeY / 2.0f > 1.0f) {
        centerY = 1.0f - sizeY / 2.0f;
    }

    rect[selectRect].xPos1 = centerX - sizeX / 2.0f;
    rect[selectRect].yPos1 = centerY - sizeY / 2.0f;
    rect[selectRect].xPos2 = centerX + sizeX / 2.0f;
    rect[selectRect].yPos2 = centerY + sizeY / 2.0f;
}

bool overlapRect(int first, int second) {
    if (rect[first].xPos2 < rect[second].xPos1) {
        return false;
    }
    if (rect[first].xPos1 > rect[second].xPos2) {
        return false;
    }
    if (rect[first].yPos2 < rect[second].yPos1) {
        return false;
    }
    if (rect[first].yPos1 > rect[second].yPos2) {
        return false;
    }

    return true;
}

void deleteRect(int index) {
    for (int i = index; i < rectCount - 1; ++i) {
        rect[i] = rect[i + 1];
    }

    --rectCount;
}

void mergeRect() {
    if (selectRect == -1) {
        return;
    }

    for (int i = rectCount - 1; i >= 0; --i) {
        if (i == selectRect) {
            continue;
        }

        if (overlapRect(selectRect, i)) {
            GLfloat minX = rect[selectRect].xPos1;
            GLfloat minY = rect[selectRect].yPos1;
            GLfloat maxX = rect[selectRect].xPos2;
            GLfloat maxY = rect[selectRect].yPos2;

            if (rect[i].xPos1 < minX) {
                minX = rect[i].xPos1;
            }
            if (rect[i].yPos1 < minY) {
                minY = rect[i].yPos1;
            }
            if (rect[i].xPos2 > maxX) {
                maxX = rect[i].xPos2;
            }
            if (rect[i].yPos2 > maxY) {
                maxY = rect[i].yPos2;
            }

            int first = selectRect;
            int second = i;

            if (first > second) {
                deleteRect(first);
                deleteRect(second);
            }
            else {
                deleteRect(second);
                deleteRect(first);
            }

            rect[rectCount].xPos1 = minX;
            rect[rectCount].yPos1 = minY;
            rect[rectCount].xPos2 = maxX;
            rect[rectCount].yPos2 = maxY;

            setColor(&rect[rectCount].Red, &rect[rectCount].Green, &rect[rectCount].Blue);

            ++rectCount;
            selectRect = -1;

            return;
        }
    }
}

void splitRect(int index) {
    if (index == -1 || rectCount >= 20) {
        return;
    }

    Rectangle oldRect = rect[index];

    GLfloat centerX = (oldRect.xPos1 + oldRect.xPos2) / 2.0f;

    deleteRect(index);

    rect[rectCount].xPos1 = oldRect.xPos1;
    rect[rectCount].yPos1 = oldRect.yPos1;
    rect[rectCount].xPos2 = centerX;
    rect[rectCount].yPos2 = oldRect.yPos2;
    setColor(&rect[rectCount].Red, &rect[rectCount].Green, &rect[rectCount].Blue);
    ++rectCount;

    rect[rectCount].xPos1 = centerX;
    rect[rectCount].yPos1 = oldRect.yPos1;
    rect[rectCount].xPos2 = oldRect.xPos2;
    rect[rectCount].yPos2 = oldRect.yPos2;
    setColor(&rect[rectCount].Red, &rect[rectCount].Green, &rect[rectCount].Blue);
    ++rectCount;
}

void framebufferSize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-3", nullptr, nullptr);

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
    glfwSetWindowAspectRatio(window, 4, 3);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            if (aFlag == false) {
                makeRect();
                aFlag = true;
            }
        }
        else {
            aFlag = false;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (leftFlag == false) {
                GLfloat mouseX;
                GLfloat mouseY;

                getMousePos(window, &mouseX, &mouseY);

                selectRect = findRect(mouseX, mouseY);

                if (selectRect != -1) {
                    GLfloat centerX = (rect[selectRect].xPos1 + rect[selectRect].xPos2) / 2.0f;
                    GLfloat centerY = (rect[selectRect].yPos1 + rect[selectRect].yPos2) / 2.0f;

                    dragX = mouseX - centerX;
                    dragY = mouseY - centerY;

                    dragging = true;
                }

                leftFlag = true;
            }

            if (dragging) {
                moveRect(window);
            }
        }
        else {
            if (dragging) {
                mergeRect();
            }

            dragging = false;
            leftFlag = false;
            selectRect = -1;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            if (rightFlag == false) {
                GLfloat mouseX;
                GLfloat mouseY;

                getMousePos(window, &mouseX, &mouseY);

                int index = findRect(mouseX, mouseY);

                if (index != -1) {
                    splitRect(index);
                }

                rightFlag = true;
            }
        }
        else {
            rightFlag = false;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < rectCount; ++i) {
            drawRect(&rect[i]);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}