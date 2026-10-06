#include <GL/glew.h>
#include <GLFW/glfw3.h>

int main()
{
    // GLFW 초기화
    if (glfwInit() != GLFW_TRUE) {
        return -1;
    }

    // 창 만들기
    GLFWwindow* window = glfwCreateWindow(
        800, 600, "OpenGLTest", nullptr, nullptr
    );

    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }

    // 이 창에 OpenGL로 그리도록 설정
    glfwMakeContextCurrent(window);

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwSwapInterval(1);

    // 배경색
    glClearColor(0.2F, 0.3F, 0.4F, 1.0F);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        // 여기에 도형을 그리는 코드를 작성한다.

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}