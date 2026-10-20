#include <GL/glew.h>
#include <GLFW/glfw3.h>
// GLM은 이동·회전·투영 계산에 쓰는 수학 라이브러리다.
// glm.hpp: vec3(숫자 3개), mat4(4x4 행렬) / matrix_transform.hpp: 변환 함수
// type_ptr.hpp: GLM 데이터를 OpenGL에 넘길 때 쓰는 value_ptr 함수
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cstdlib>
#include <ctime>

GLuint shaderProgram{};
GLuint VAO{};
GLuint VBO{};
GLint colorLocation{ -1 };
GLint transformLocation{ -1 };
int windowWidth{ 800 };
int windowHeight{ 600 };

int objectType{ 0 }; // 0: 육면체, 1: 사각뿔
bool faceVisible[6]{ true, true, true, true, true, true };

// 꼭짓점 하나는 이제 x, y, z 세 좌표를 사용한다.
// 배열의 한 행이 꼭짓점 하나다. 육면체는 서로 다른 꼭짓점 8개를 사용한다.
const GLfloat cubeVertices[8][3] = {
    {-0.35f, -0.35f,  0.35f}, { 0.35f, -0.35f,  0.35f},
    { 0.35f,  0.35f,  0.35f}, {-0.35f,  0.35f,  0.35f},
    {-0.35f, -0.35f, -0.35f}, { 0.35f, -0.35f, -0.35f},
    { 0.35f,  0.35f, -0.35f}, {-0.35f,  0.35f, -0.35f}
};

// 사각뿔은 바닥 꼭짓점 4개(0~3)와 위쪽 꼭짓점 1개(4)를 사용한다.
const GLfloat pyramidVertices[5][3] = {
    {-0.35f, -0.35f,  0.35f}, { 0.35f, -0.35f,  0.35f},
    { 0.35f, -0.35f, -0.35f}, {-0.35f, -0.35f, -0.35f},
    { 0.0f,   0.4f,   0.0f }
};

const GLclampf faceColors[6][3] = {
    {1.0f, 0.15f, 0.15f}, {0.15f, 0.4f, 1.0f},
    {0.2f, 1.0f, 0.25f}, {1.0f, 0.85f, 0.1f},
    {0.1f, 0.9f, 1.0f}, {1.0f, 0.2f, 0.85f}
};

const char* vertexSource = R"(
#version 330 core
// vec3는 좌표 3개(x, y, z)를 묶는다. 이전 2D 실습의 vec2에서 z가 추가됐다.
layout (location = 0) in vec3 position;
// mat4는 4x4 행렬이다. C++에서 전달한 변환을 모든 꼭짓점에 공통으로 적용한다.
uniform mat4 transform;
void main()
{
    // vec4(position, 1.0)은 (x, y, z, w=1)인 점이다. w=1이라 이동 변환도 적용된다.
    // 행렬 * 좌표로 변환한 뒤 gl_Position에 넣는다. 원래 좌표 배열을 수정하지 않는다.
    gl_Position = transform * vec4(position, 1.0);
}
)";

const char* fragmentSource = R"(
#version 330 core
uniform vec3 drawColor;
out vec4 fragmentColor;
void main()
{
    fragmentColor = vec4(drawColor, 1.0);
}
)";

// 육면체: 앞, 뒤, 왼쪽, 오른쪽, 위, 아래
// 한 행은 면 하나다. 안의 숫자는 위 cubeVertices의 꼭짓점 번호다.
// 예: 첫 번째 면은 0, 1, 2, 3번 꼭짓점을 테두리 순서로 연결한다.
const int cubeFaces[6][4] = {
    {0, 1, 2, 3}, {5, 4, 7, 6}, {4, 0, 3, 7},
    {1, 5, 6, 2}, {3, 2, 6, 7}, {4, 5, 1, 0}
};

// 사각뿔: 앞, 오른쪽, 뒤, 왼쪽의 삼각형 면과 사각형 바닥
// 옆면은 앞의 번호 3개만 사용한다. 마지막 0은 칸을 맞춘 값이라 그리지 않는다.
// 마지막 행인 바닥 면만 꼭짓점 번호 4개를 전부 사용한다.
const int pyramidFaces[5][4] = {
    {0, 1, 4, 0}, {1, 2, 4, 0}, {2, 3, 4, 0},
    {3, 0, 4, 0}, {3, 2, 1, 0}
};

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        char log[1024]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compilation failed:\n" << log << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createShader()
{
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertex == 0) return 0;
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragment == 0) {
        glDeleteShader(vertex);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint success{};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char log[1024]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Shader linking failed:\n" << log << '\n';
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

void framebufferSize(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
    if (width > 0 && height > 0) {
        windowWidth = width;
        windowHeight = height;
    }
}

glm::mat4 getViewProjection()
{
    glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    GLfloat aspect = static_cast<GLfloat>(windowWidth) / windowHeight;
    glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, 0.1f, 10.0f);
    return projection * view;
}

void initBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 사각형 면은 삼각형 2개, 즉 6개 꼭짓점으로 그린다.
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
    // 0번 속성은 x, y, z를 3개씩 읽는다. 다음 꼭짓점까지의 간격도 float 3개다.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
}

void drawVertices(const GLfloat* vertices, int count, GLenum mode, GLclampf Red, GLclampf Green, GLclampf Blue)
{
    glUniform3f(colorLocation, Red, Green, Blue);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * 3 * sizeof(GLfloat), vertices);
    glDrawArrays(mode, 0, count);
}

// const glm::mat4&: 호출한 쪽의 행렬을 복사하지 않고 읽어서 사용한다.
void drawAxes(const glm::mat4& viewProjection)
{
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));
    const GLfloat xAxis[]{ -0.9f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f };
    const GLfloat yAxis[]{ 0.0f, -0.9f, 0.0f, 0.0f, 0.9f, 0.0f };
    const GLfloat zAxis[]{ 0.0f, 0.0f, -0.9f, 0.0f, 0.0f, 0.9f };
    drawVertices(xAxis, 2, GL_LINES, 1.0f, 0.2f, 0.2f);
    drawVertices(yAxis, 2, GL_LINES, 0.2f, 1.0f, 0.2f);
    drawVertices(zAxis, 2, GL_LINES, 0.2f, 0.2f, 1.0f);
}

void drawShape(const glm::mat4& viewProjection)
{
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    
    int faceCount = objectType == 0 ? 6 : 5;

    const int order[6]{ 0, 1, 2, 0, 2, 3 };
    for (int i = 0; i < faceCount; ++i) {
        if (!faceVisible[i]) continue;
        GLfloat vertices[18]{};
        int count = (objectType == 1 && i < 4) ? 3 : 6;
        for (int j = 0; j < count; ++j) {
            int index = objectType == 0 ? cubeFaces[i][order[j]] : pyramidFaces[i][order[j]];
            for (int k = 0; k < 3; ++k) {
                vertices[j * 3 + k] = objectType == 0 ? cubeVertices[index][k] : pyramidVertices[index][k];
            }
        }
        drawVertices(vertices, count, GL_TRIANGLES, faceColors[i][0], faceColors[i][1], faceColors[i][2]);
    }
}

// second를 생략하면 -1이다. -1은 두 번째 면을 선택하지 않는다는 표시다.
void showFaces(int type, int first, int second = -1)
{
    objectType = type;
    for (int i = 0; i < 6; ++i) 
        faceVisible[i] = false;
    faceVisible[first] = true;
    if (second >= 0) faceVisible[second] = true;
}

void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) return;
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6) {
        showFaces(0, key - GLFW_KEY_1);
    }
    else if (key >= GLFW_KEY_7 && key <= GLFW_KEY_9) {
        showFaces(1, key - GLFW_KEY_7);
    }
    else if (key == GLFW_KEY_0) {
        showFaces(1, 3);
    }
    else if (key == GLFW_KEY_C) {
        int first = std::rand() % 6;
        int second = std::rand() % 5;
        // 0~4의 난수를 first를 뺀 5개 면 번호로 바꿔, 같은 면이 두 번 뽑히지 않게 한다.
        if (second >= first) ++second;
        showFaces(0, first, second);
    }
    else if (key == GLFW_KEY_T) {
        showFaces(1, std::rand() % 4, 4);
    }
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void drawScene()
{
    // |는 두 초기화 대상을 함께 지정한다. 화면 색과 깊이 기록을 매번 둘 다 지운다.
    // 깊이 기록도 지워야 이전 프레임의 면이 새 프레임을 가리지 않는다.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 viewProjection = getViewProjection();
    drawAxes(viewProjection);
    drawShape(viewProjection);
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
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 창을 만들 때 깊이값을 기록할 24비트 깊이 버퍼를 요청한다.
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-13", nullptr, nullptr);
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

    shaderProgram = createShader();
    if (shaderProgram == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    glUseProgram(shaderProgram);
    colorLocation = glGetUniformLocation(shaderProgram, "drawColor");
    transformLocation = glGetUniformLocation(shaderProgram, "transform");
    initBuffer();
    // 은면 제거에 사용할 깊이 검사를 켠다. 겹치는 면 중 가까운 면을 남긴다.
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glfwSwapInterval(1);
    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSize(window, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetKeyCallback(window, keyCallback);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        drawScene();
        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
