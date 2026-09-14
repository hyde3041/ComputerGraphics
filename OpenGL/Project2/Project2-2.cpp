// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project2-2", nullptr, nullptr);
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

        // 배경을 흰색으로 지우기
        glClearColor(1.0F, 1.0F, 1.0F, 1.0F);
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
