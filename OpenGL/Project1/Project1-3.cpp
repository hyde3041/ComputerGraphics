// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cstdlib>
#include <ctime>
#include <iostream>

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

    glm::vec3 backgroundColor(1.0F, 1.0F, 1.0F);
    bool timerRunning = false;
    double nextColorChangeTime = 0.0;

    int previousAState = GLFW_RELEASE;
    int previousTState = GLFW_RELEASE;
    int previousSState = GLFW_RELEASE;

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // 지정된 키에 따라 배경색 변경
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            backgroundColor = glm::vec3(0.0F, 1.0F, 1.0F);
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            backgroundColor = glm::vec3(1.0F, 0.0F, 1.0F);
        }
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            backgroundColor = glm::vec3(1.0F, 1.0F, 0.0F);
        }
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            backgroundColor = glm::vec3(0.5F, 0.5F, 0.5F);
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            backgroundColor = glm::vec3(0.0F, 0.0F, 0.0F);
        }

        // A: 누를 때마다 한 번 랜덤색으로 변경
        const int currentAState = glfwGetKey(window, GLFW_KEY_A);
        if (currentAState == GLFW_PRESS && previousAState == GLFW_RELEASE) {
            backgroundColor.r = static_cast<float>(std::rand()) / RAND_MAX;
            backgroundColor.g = static_cast<float>(std::rand()) / RAND_MAX;
            backgroundColor.b = static_cast<float>(std::rand()) / RAND_MAX;
        }
        previousAState = currentAState;

        // T: 타이머 시작, S: 타이머 정지
        const int currentTState = glfwGetKey(window, GLFW_KEY_T);
        const int currentSState = glfwGetKey(window, GLFW_KEY_S);

        if (currentTState == GLFW_PRESS && previousTState == GLFW_RELEASE) {
            timerRunning = true;
            nextColorChangeTime = glfwGetTime();
        }
        if (currentSState == GLFW_PRESS && previousSState == GLFW_RELEASE) {
            timerRunning = false;
        }

        previousTState = currentTState;
        previousSState = currentSState;

        // 타이머 실행 중에는 0.5초마다 랜덤색으로 변경
        const double currentTime = glfwGetTime();
        if (timerRunning && currentTime >= nextColorChangeTime) {
            backgroundColor.r = static_cast<float>(std::rand()) / RAND_MAX;
            backgroundColor.g = static_cast<float>(std::rand()) / RAND_MAX;
            backgroundColor.b = static_cast<float>(std::rand()) / RAND_MAX;
            nextColorChangeTime = currentTime + 0.5;
        }

        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
