#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <fstream> // ifstream: 파일을 읽는다.
#include <sstream> // istringstream: 한 줄의 내용을 나누어 읽는다.
#include <string>
#include <vector> // vector: 읽은 점 개수에 맞게 저장 공간이 늘어난다.

#ifndef OBJ_DIRECTORY
#define OBJ_DIRECTORY "models"
#endif

GLuint shaderProgram{}, VAO{}, VBO{};
GLint transformLocation{ -1 }, colorLocation{ -1 };
int windowWidth{ 800 }, windowHeight{ 600 };
std::vector<GLfloat> cubeVertices;

const GLfloat floorHalfSize{ 5.0f };
const GLfloat tankRadius{ 2.7f }; // 모든 부품이 회전할 때의 바닥 위 최대 범위
GLfloat tankX{}, tankZ{};
GLfloat lowerHeight{ 0.24f }; // 아래 몸체의 반높이: 밑면은 항상 바닥 Y=0에 붙는다.
int lowerDirection{ 1 };
bool lowerMove{};
GLfloat centerRotation{};
bool centerMove{};
GLfloat upperSwap{}; // 0: 원래 위치, 1: 두 상부 몸체의 위치가 바뀜
GLfloat upperTarget{};
GLfloat gunRotation{};
bool gunMove{};
GLfloat flagRotation{};
bool flagMove{};

glm::vec3 cameraPosition;
glm::vec3 cameraFront; // 카메라가 보는 방향
// lookAt의 화면 위쪽 방향이다. 카메라가 자전/공전할 때 함께 사용한다.
glm::vec3 cameraUp;
int cameraSelfDirection{};
int cameraOrbitDirection{};

const glm::vec3 initialColor[12]{
    { 0.18f, 0.30f, 0.20f }, // 바닥
    { 0.20f, 0.23f, 0.28f }, { 0.30f, 0.34f, 0.40f }, // 아래 몸체 둘
    { 0.25f, 0.65f, 0.35f }, // 중앙 몸체
    { 0.65f, 0.85f, 0.30f }, { 0.20f, 0.70f, 0.70f }, // 상부 몸체 둘
    { 0.85f, 0.40f, 0.15f }, { 0.30f, 0.45f, 0.95f }, // 포신 둘
    { 0.95f, 0.85f, 0.20f }, { 0.90f, 0.65f, 0.20f }, // 깃대 둘
    { 0.95f, 0.20f, 0.20f }, { 0.80f, 0.25f, 0.95f }  // 깃발 둘
};
glm::vec3 bodyColor[12]{};

const char* vertexSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
uniform mat4 transform;
uniform vec3 drawColor;
out vec3 vertexColor;
void main()
{
    gl_Position = transform * vec4(position, 1.0);
    vertexColor = drawColor * (vec3(0.65) + 0.35 * color);
}
)";

const char* fragmentSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 fragmentColor;
void main()
{
    fragmentColor = vec4(vertexColor, 1.0);
}
)";

bool loadOBJ(const char* fileName, std::vector<GLfloat>& vertices)
{
    // OBJ_DIRECTORY는 프로젝트 설정에서 지정한 models 폴더이다.
    std::string path = std::string(OBJ_DIRECTORY) + "/" + fileName;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "OBJ file open failed: " << path << '\n';
        return false;
    }

    std::vector<glm::vec3> positions; // v 줄에서 읽은 원래 점 좌표
    std::string line;
    vertices.clear();
    while (std::getline(file, line)) { // 파일 끝까지 한 줄씩 읽는다.
        std::istringstream data(line);
        std::string type;
        data >> type; // 줄의 첫 글자: v, f, vt, vn 등

        if (type == "v") {
            GLfloat x, y, z;
            if (!(data >> x >> y >> z)) {
                std::cerr << "OBJ vertex error: " << path << '\n';
                return false;
            }
            positions.push_back(glm::vec3(x, y, z)); // 점을 맨 뒤에 추가한다.
        }
        else if (type == "f") {
            std::vector<int> face; // 현재 면을 구성하는 점 번호
            std::string word;
            while (data >> word) {
                if (word[0] == '#') break;
                // 5/2/1에서 앞의 정수 5만 읽는다. vt, vn 번호는 사용하지 않는다.
                std::istringstream indexData(word);
                int index;
                if (!(indexData >> index) || index == 0) {
                    std::cerr << "OBJ face error: " << path << '\n';
                    return false;
                }
                if (index > 0) --index; // OBJ의 1번 점은 C++ 배열의 0번이다.
                else index = static_cast<int>(positions.size()) + index;
                if (index < 0 || index >= static_cast<int>(positions.size())) {
                    std::cerr << "OBJ index out of range: " << path << '\n';
                    return false;
                }
                face.push_back(index);
            }
            if (face.size() < 3) {
                std::cerr << "OBJ face needs at least 3 vertices: " << path << '\n';
                return false;
            }

            // 이 파일들의 볼록한 면을 A-B-C, A-C-D, A-D-E...로 나눈다.
            // 사각형뿐 아니라 원뿔의 32개 점짜리 바닥도 처리한다.
            for (int i = 1; i + 1 < static_cast<int>(face.size()); ++i) {
                const int triangle[3]{ face[0], face[i], face[i + 1] };
                for (int j = 0; j < 3; ++j) {
                    glm::vec3 point = positions[triangle[j]];
                    vertices.push_back(point.x);
                    vertices.push_back(point.y);
                    vertices.push_back(point.z);
                    // 지금 셰이더는 XYZRGB를 받는다. 색은 좌표 -1~1을 0~1로 바꿔 지정한다.
                    vertices.push_back((point.x + 1.0f) / 2.0f);
                    vertices.push_back((point.y + 1.0f) / 2.0f);
                    vertices.push_back((point.z + 1.0f) / 2.0f);
                }
            }
        }
        // vt, vn, mtllib 등은 현재 좌표와 색으로 그리는 단계에서는 사용하지 않는다.
    }
    if (vertices.empty()) {
        std::cerr << "OBJ contains no faces: " << path << '\n';
        return false;
    }
    return true;
}

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

void initBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
}

void drawVertices(const GLfloat* vertices, int count, GLenum mode)
{
    // 구는 점이 더 많으므로, 그릴 데이터 크기에 맞춰 버퍼를 확보하고 복사한다.
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(count) * 6 * sizeof(GLfloat), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(mode, 0, count);
}

void resetScene()
{
    tankX = tankZ = 0.0f;
    lowerHeight = 0.24f;
    lowerDirection = 1;
    lowerMove = false;
    centerRotation = 0.0f;
    centerMove = false;
    upperSwap = upperTarget = 0.0f;
    gunRotation = flagRotation = 0.0f;
    gunMove = flagMove = false;
    cameraPosition = glm::vec3(7.0f, 5.0f, 9.0f);
    cameraFront = glm::normalize(-cameraPosition);
    // 두 방향의 외적(cross)은 두 방향에 수직인 방향이다.
    glm::vec3 right = glm::normalize(glm::cross(cameraFront, glm::vec3(0, 1, 0)));
    cameraUp = glm::normalize(glm::cross(right, cameraFront));
    cameraSelfDirection = cameraOrbitDirection = 0;
    for (int i = 0; i < 12; ++i) bodyColor[i] = initialColor[i];
}

glm::mat4 getViewProjection()
{
    GLfloat aspect = static_cast<GLfloat>(windowWidth) / windowHeight;
    // 목표점을 위치+방향으로 정하면, 제자리에서 보는 방향만 돌릴 수 있다.
    glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    return glm::perspective(glm::radians(50.0f), aspect, 0.1f, 50.0f) * view;
}

void drawCube(const glm::mat4& viewProjection, const glm::mat4& parent,
    glm::vec3 position, glm::vec3 halfSize, int color)
{
    // cube.obj는 -1~1이다. halfSize가 각 방향의 반길이가 된다.
    glm::mat4 model = glm::translate(parent, position);
    model = glm::scale(model, halfSize);
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3f(colorLocation, bodyColor[color].r, bodyColor[color].g, bodyColor[color].b);
    drawVertices(cubeVertices.data(), static_cast<int>(cubeVertices.size() / 6), GL_TRIANGLES);
}

void moveScene()
{
    if (lowerMove) {
        lowerHeight += lowerDirection * 0.002f;
        if (lowerHeight >= 0.4f) { lowerHeight = 0.4f; lowerDirection = -1; }
        if (lowerHeight <= 0.18f) { lowerHeight = 0.18f; lowerDirection = 1; }
    }
    if (centerMove) {
        centerRotation += 0.5f;
        if (centerRotation >= 360.0f) centerRotation -= 360.0f;
    }
    if (upperSwap < upperTarget) upperSwap = glm::min(upperSwap + 0.01f, upperTarget);
    if (upperSwap > upperTarget) upperSwap = glm::max(upperSwap - 0.01f, upperTarget);
    if (gunMove) {
        gunRotation += 0.6f;
        if (gunRotation >= 360.0f) gunRotation -= 360.0f;
    }
    if (flagMove) {
        flagRotation += 0.7f;
        if (flagRotation >= 360.0f) flagRotation -= 360.0f;
    }

    if (cameraSelfDirection != 0) {
        // W=0인 방향 벡터에는 이동이 적용되지 않는다. 카메라 위치는 유지한다.
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),
            glm::radians(cameraSelfDirection * 0.5f), cameraUp);
        cameraFront = glm::normalize(glm::vec3(rotation * glm::vec4(cameraFront, 0.0f)));
    }
    if (cameraOrbitDirection != 0) {
        // 카메라 위치와 보는 방향/위 방향을 함께 돌리면 중심 주위를 공전한다.
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),
            glm::radians(cameraOrbitDirection * 0.4f), glm::vec3(0, 1, 0));
        cameraPosition = glm::vec3(rotation * glm::vec4(cameraPosition, 1.0f));
        cameraFront = glm::normalize(glm::vec3(rotation * glm::vec4(cameraFront, 0.0f)));
        cameraUp = glm::normalize(glm::vec3(rotation * glm::vec4(cameraUp, 0.0f)));
    }
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    bool moving = key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT || key == GLFW_KEY_UP ||
        key == GLFW_KEY_DOWN || key == GLFW_KEY_X || key == GLFW_KEY_Z;
    if (action == GLFW_REPEAT && !moving) return;
    if (key == GLFW_KEY_LEFT) tankX -= 0.1f;
    else if (key == GLFW_KEY_RIGHT) tankX += 0.1f;
    else if (key == GLFW_KEY_UP) tankZ -= 0.1f;
    else if (key == GLFW_KEY_DOWN) tankZ += 0.1f;
    else if (key == GLFW_KEY_D) lowerMove = !lowerMove;
    else if (key == GLFW_KEY_T) centerMove = !centerMove;
    else if (key == GLFW_KEY_L) upperTarget = 1.0f - upperTarget;
    else if (key == GLFW_KEY_G) gunMove = !gunMove;
    else if (key == GLFW_KEY_P) flagMove = !flagMove;
    else if (key == GLFW_KEY_X) cameraPosition.x += shift ? -0.25f : 0.25f;
    else if (key == GLFW_KEY_Z) cameraPosition.z += shift ? -0.25f : 0.25f;
    else if (key == GLFW_KEY_Y) {
        int direction = shift ? 1 : -1;
        cameraSelfDirection = cameraSelfDirection == direction ? 0 : direction;
    }
    else if (key == GLFW_KEY_R) {
        int direction = shift ? 1 : -1;
        cameraOrbitDirection = cameraOrbitDirection == direction ? 0 : direction;
    }
    else if (key == GLFW_KEY_O) {
        for (int i = 0; i < 12; ++i) {
            bodyColor[i] = glm::vec3(0.25f + (std::rand() % 76) / 100.0f,
                0.25f + (std::rand() % 76) / 100.0f, 0.25f + (std::rand() % 76) / 100.0f);
        }
    }
    else if (key == GLFW_KEY_C) resetScene();
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    // 가장 바깥으로 나온 포신까지 바닥 안에 머물도록 중심 이동 범위를 줄인다.
    tankX = glm::clamp(tankX, -floorHalfSize + tankRadius, floorHalfSize - tankRadius);
    tankZ = glm::clamp(tankZ, -floorHalfSize + tankRadius, floorHalfSize - tankRadius);
}

void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 viewProjection = getViewProjection();
    glm::mat4 identity(1.0f);
    // 바닥의 윗면이 Y=0이다.
    drawCube(viewProjection, identity, glm::vec3(0, -0.08f, 0),
        glm::vec3(floorHalfSize, 0.08f, floorHalfSize), 0);
    glm::mat4 root = glm::translate(identity, glm::vec3(tankX, 0, tankZ));
    drawCube(viewProjection, root, glm::vec3(-0.72f, lowerHeight, 0),
        glm::vec3(0.23f, lowerHeight, 1.35f), 1);
    drawCube(viewProjection, root, glm::vec3(0.72f, lowerHeight, 0),
        glm::vec3(0.25f, lowerHeight, 1.25f), 2);

    // 아래 몸체 높이가 변하면 그 위 부품도 같은 높이만큼 올라간다.
    glm::mat4 center = glm::translate(root, glm::vec3(0, lowerHeight * 2.0f + 0.2f, 0));
    center = glm::rotate(center, glm::radians(centerRotation), glm::vec3(0, 1, 0));
    drawCube(viewProjection, center, glm::vec3(0), glm::vec3(0.96f, 0.2f, 1.05f), 3);
    for (int i = 0; i < 2; ++i) {
        GLfloat side = i == 0 ? -1.0f : 1.0f;
        GLfloat upperHeight = i == 0 ? 0.24f : 0.20f;
        GLfloat upperDepth = i == 0 ? 0.56f : 0.64f;
        GLfloat x, z;
        // 먼저 앞/뒤로 벌어지고 -> 서로 지나가고 -> 원래 Z 위치로 돌아온다.
        if (upperSwap < 0.25f) {
            x = side * 0.48f;
            z = side * 0.8f * upperSwap / 0.25f;
        }
        else if (upperSwap < 0.75f) {
            x = side * 0.48f * (1.0f - 2.0f * (upperSwap - 0.25f) / 0.5f);
            z = side * 0.8f;
        }
        else {
            x = -side * 0.48f;
            z = side * 0.8f * (1.0f - upperSwap) / 0.25f;
        }
        glm::mat4 upper = glm::translate(center, glm::vec3(x, 0.2f + upperHeight, z));
        drawCube(viewProjection, upper, glm::vec3(0),
            glm::vec3(i == 0 ? 0.34f : 0.30f, upperHeight, upperDepth), 4 + i);

        // 포신과 깃대는 상부 몸체 행렬을 물려받아 이동/중앙 회전을 따라간다.
        glm::mat4 gun = glm::translate(upper, glm::vec3(0, 0, upperDepth));
        gun = glm::rotate(gun, glm::radians(side * gunRotation), glm::vec3(0, 1, 0));
        drawCube(viewProjection, gun, glm::vec3(0, 0, 0.5f), glm::vec3(0.075f, 0.075f, 0.5f), 6 + i);
        glm::mat4 flag = glm::translate(upper, glm::vec3(0, upperHeight, 0));
        flag = glm::rotate(flag, glm::radians(side * flagRotation), glm::vec3(1, 0, 0));
        drawCube(viewProjection, flag, glm::vec3(0, 0.4f, 0), glm::vec3(0.035f, 0.4f, 0.035f), 8 + i);
        drawCube(viewProjection, flag, glm::vec3(0.18f, 0.68f, 0), glm::vec3(0.16f, 0.1f, 0.02f), 10 + i);
    }
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    if (!loadOBJ("cube.obj", cubeVertices)) return -1;
    if (glfwInit() != GLFW_TRUE) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-19", nullptr, nullptr);
    if (window == nullptr) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window); glfwTerminate(); return -1;
    }
    shaderProgram = createShader();
    if (shaderProgram == 0) {
        glfwDestroyWindow(window); glfwTerminate(); return -1;
    }
    glUseProgram(shaderProgram);
    transformLocation = glGetUniformLocation(shaderProgram, "transform");
    colorLocation = glGetUniformLocation(shaderProgram, "drawColor");
    initBuffer();
    resetScene();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.07f, 0.09f, 0.13f, 1.0f);
    glfwSwapInterval(1);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSize(window, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetKeyCallback(window, keyCallback);
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        moveScene();
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
