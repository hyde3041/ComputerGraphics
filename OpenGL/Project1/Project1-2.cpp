// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cstdlib>
#include <ctime>
#include <iostream>

int main()
{
    // 난수 생성기가 실행할 때마다 다른 값을 만들도록 초기화
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

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

    // 최초 배경색은 흰색
    glm::vec3 backgroundColor(1.0F, 1.0F, 1.0F);
    int previousAState = GLFW_RELEASE;

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // C: 청록색, M: 자홍색, Y: 노란색
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            backgroundColor = glm::vec3(0.0F, 1.0F, 1.0F);
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            backgroundColor = glm::vec3(1.0F, 0.0F, 1.0F);
        }
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            backgroundColor = glm::vec3(1.0F, 1.0F, 0.0F);
        }

        // G: 회색, K: 검정색
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            backgroundColor = glm::vec3(0.5F, 0.5F, 0.5F);
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            backgroundColor = glm::vec3(0.0F, 0.0F, 0.0F);
        }

        // A 키를 새로 눌렀을 때만 한 번 랜덤색으로 변경
        const int currentAState = glfwGetKey(window, GLFW_KEY_A);
        if (currentAState == GLFW_PRESS && previousAState == GLFW_RELEASE) {
            backgroundColor.r = static_cast<float>(std::rand()) / RAND_MAX;
            backgroundColor.g = static_cast<float>(std::rand()) / RAND_MAX;
            backgroundColor.b = static_cast<float>(std::rand()) / RAND_MAX;
        }
        previousAState = currentAState;

        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
