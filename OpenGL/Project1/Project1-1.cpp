// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

GLclampf Red;
GLclampf Green;
GLclampf Blue;

bool timerOn = false;
double lastTime = 0.0;

void setRandColor() {
    Red = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    Green = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    Blue = static_cast<GLclampf>(std::rand()) / RAND_MAX;
}

int main()
{
    // GLFW 초기화
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "GLFW initialization failed.\n";
        return -1;
    }

    // OpenGL 3.3 Compatibility Profile 설정
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    // 800 x 600 크기의 윈도우 생성
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-1", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Window creation failed.\n";
        glfwTerminate();
        return -1;
    }

    // 생성한 윈도우의 OpenGL 컨텍스트 활성화
    glfwMakeContextCurrent(window);

    // 최신 OpenGL 함수 사용을 위한 GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // OpenGL이 그림을 그릴 영역 설정
    glViewport(0, 0, 800, 600);

    // 윈도우가 닫힐 때까지 반복
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        // Esc 키를 누르면 프로그램 종료

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            glClearColor(0.0F, 1.0F, 1.0F, 1.0F);
        }
        else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            glClearColor(1.0F, 0.0F, 1.0F, 1.0F);
        }
        else if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            glClearColor(1.0F, 1.0F, 0.0F, 1.0F);
        }
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            setRandColor();
            glClearColor(Red, Green, Blue, 1.0F);
        }
        else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            glClearColor(0.5F, 0.5F, 0.5F, 1.0F);
        }
        else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
        }
        else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            timerOn = true;
            lastTime = glfwGetTime();
        }

        // S : 타이머 정지
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            timerOn = false;
        }
        
        if (timerOn) {

            double currentTime = glfwGetTime();

            // 1초가 지났다면
            if (currentTime - lastTime >= 1.0) {

                setRandColor();
                glClearColor(Red, Green, Blue, 1.0F);

                lastTime = currentTime;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        // 완성된 화면을 표시하고 입력 이벤트 확인
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 사용한 자원 정리
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
