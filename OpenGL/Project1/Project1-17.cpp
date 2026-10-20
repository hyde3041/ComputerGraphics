#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>
#include <fstream> // ifstream: 파일을 읽는다.
#include <sstream> // istringstream: 한 줄의 내용을 나누어 읽는다.
#include <string>
#include <vector> // vector: 읽은 점 개수에 맞게 저장 공간이 늘어난다.

#ifndef OBJ_DIRECTORY
#define OBJ_DIRECTORY "models"
#endif

struct object {
    int objectType{}; // 0: 육면체, 1: 원뿔, 2: 구
    GLfloat xPos{}, yPos{}, zPos{};
    int xDirection{}, yDirection{};
    GLfloat xRotation{}, yRotation{}, zRotation{};
    GLfloat orbitRotation{}; // 중앙 Y축을 기준으로 도는 공전 각도
    int orbitDirection{};
    GLfloat size{ 0.35f }; // 제자리 크기
    GLfloat originSize{ 1.0f }; // 원점 기준 크기: 위치와 도형 크기가 함께 바뀐다.
};

GLuint shaderProgram{};
GLuint VAO{};
GLuint VBO{};
GLint transformLocation{ -1 };
int windowWidth{ 800 };
int windowHeight{ 600 };

int selectedObject{ 3 }; // 기존 방식: 1=first, 2=secound, 3=둘 다
object first{};
object secound{};
bool depthTest{ true };
bool wireFrame{ false };

int animationMode{}; // 0: 일반 조작, 1: 직선 교환, 2: 위/아래 교환, 3: 크기+자전+공전
GLfloat animationProgress{}; // 0에서 시작해 1이 되면 자리 교환 완료
int sizeDirection{ 1 };
glm::vec3 startPosition[2]{}; // t/u를 누른 순간의 위치

// 각 OBJ를 삼각형으로 나눈 XYZRGB 데이터이다. 시작할 때 한 번만 읽는다.
std::vector<GLfloat> modelVertices[3];

const char* vertexSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
uniform mat4 transform;
out vec3 vertexColor;
void main()
{
    gl_Position = transform * vec4(position, 1.0);
    vertexColor = color;
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

glm::mat4 getViewProjection()
{
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.5f),
        glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    GLfloat aspect = static_cast<GLfloat>(windowWidth) / windowHeight;
    return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 30.0f) * view;
}

glm::mat4 getSceneModel()
{
    // 좌표축과 두 도형을 같은 좌표계에서 X/Y축으로 30도 기울인다.
    // 개별 xRotation/yRotation은 이 상태를 기준으로 하는 추가 자전 각도이다.
    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0, 1, 0));
    return glm::rotate(model, glm::radians(30.0f), glm::vec3(1, 0, 0));
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

void drawAxes(const glm::mat4& viewProjection)
{
    glm::mat4 transform = viewProjection * getSceneModel();
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    const GLfloat axes[]{
        -0.9f, 0.0f, 0.0f, 1.0f, 0.2f, 0.2f,
         0.9f, 0.0f, 0.0f, 1.0f, 0.2f, 0.2f,
         0.0f,-0.9f, 0.0f, 0.2f, 1.0f, 0.2f,
         0.0f, 0.9f, 0.0f, 0.2f, 1.0f, 0.2f,
         0.0f, 0.0f,-0.9f, 0.3f, 0.5f, 1.0f,
         0.0f, 0.0f, 0.9f, 0.3f, 0.5f, 1.0f
    };
    drawVertices(axes, 6, GL_LINES);
}



void drawShape(const glm::mat4& viewProjection, object* object)
{
    glm::mat4 model = getSceneModel();
    // 원점 기준 확대 -> 중앙 Y축 공전 -> 위치 이동 -> 제자리 자전 -> 제자리 크기.
    // 실제 점에는 오른쪽의 변환부터 적용된다.
    model = glm::scale(model, glm::vec3(object->originSize));
    model = glm::rotate(model, glm::radians(object->orbitRotation), glm::vec3(0, 1, 0));
    model = glm::translate(model, glm::vec3(object->xPos, object->yPos, object->zPos));
    model = glm::rotate(model, glm::radians(object->yRotation), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(object->xRotation), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(object->size));
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
    const std::vector<GLfloat>& vertices = modelVertices[object->objectType];
    drawVertices(vertices.data(), static_cast<int>(vertices.size() / 6), GL_TRIANGLES);
}

void moveShape(object* object)
{
    // 매 프레임 고정 각도만큼 움직인다. 방향값 0이면 멈춘다.
    object->xRotation += object->xDirection * 0.5f;
    object->yRotation += object->yDirection * 0.5f;
    object->orbitRotation += object->orbitDirection * 0.5f;
    if (object->xRotation >= 360.0f) object->xRotation -= 360.0f;
    if (object->xRotation <= -360.0f) object->xRotation += 360.0f;
    if (object->yRotation >= 360.0f) object->yRotation -= 360.0f;
    if (object->yRotation <= -360.0f) object->yRotation += 360.0f;
    if (object->orbitRotation >= 360.0f) object->orbitRotation -= 360.0f;
    if (object->orbitRotation <= -360.0f) object->orbitRotation += 360.0f;
}

void resetMove()
{
    first = object{};
    first.objectType = 0;
    first.xPos = 0.8f;
    secound = object{};
    secound.objectType = 1;
    secound.xPos = -0.8f;
    selectedObject = 3;
    animationMode = 0;
    animationProgress = 0.0f;
    sizeDirection = 1;
    depthTest = true;
    wireFrame = false;
}

void keepCurrentPosition(object* object)
{
    // 공전/원점 확대 상태를 현재 좌표에 합쳐 놓는다.
    // t/u를 눌렀을 때 원래 자리로 튀지 않고 지금 보이는 자리에서 출발한다.
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),
        glm::radians(object->orbitRotation), glm::vec3(0, 1, 0));
    glm::vec4 point(object->xPos, object->yPos, object->zPos, 1.0f);
    point = rotation * point;
    object->xPos = point.x * object->originSize;
    object->yPos = point.y * object->originSize;
    object->zPos = point.z * object->originSize;
    object->size *= object->originSize;
    object->yRotation += object->orbitRotation;
    object->originSize = 1.0f;
    object->orbitRotation = 0.0f;
    object->xDirection = object->yDirection = object->orbitDirection = 0;
}

void startAnimation(int mode)
{
    if (mode == 3 && animationMode == 3) {
        animationMode = 0;
        first.xDirection = first.yDirection = first.orbitDirection = 0;
        secound.xDirection = secound.yDirection = secound.orbitDirection = 0;
        return;
    }
    keepCurrentPosition(&first);
    keepCurrentPosition(&secound);
    animationMode = mode;
    animationProgress = 0.0f;
    startPosition[0] = glm::vec3(first.xPos, first.yPos, first.zPos);
    startPosition[1] = glm::vec3(secound.xPos, secound.yPos, secound.zPos);
    if (mode == 3) {
        first.size = secound.size = 0.35f;
        sizeDirection = 1;
        first.yDirection = 1;
        secound.yDirection = -1;
        first.orbitDirection = secound.orbitDirection = 1;
    }
}

void moveAnimation()
{
    if (animationMode == 1 || animationMode == 2) {
        animationProgress += 0.008f;
        if (animationProgress > 1.0f) animationProgress = 1.0f;
        // 시작 위치와 상대 위치 사이를 조금씩 이동한다.
        GLfloat t = animationProgress;
        glm::vec3 a = startPosition[0] * (1.0f - t) + startPosition[1] * t;
        glm::vec3 b = startPosition[1] * (1.0f - t) + startPosition[0] * t;
        if (animationMode == 1) {
            // 수동 이동한 뒤에도 반드시 중앙 원점을 통과하고 상대 위치로 간다.
            if (t <= 0.5f) {
                a = startPosition[0] * (1.0f - t * 2.0f);
                b = startPosition[1] * (1.0f - t * 2.0f);
            }
            else {
                a = startPosition[1] * (t * 2.0f - 1.0f);
                b = startPosition[0] * (t * 2.0f - 1.0f);
            }
        }
        else if (animationMode == 2) {
            // sin(0)=sin(180도)=0: 출발/도착 높이는 같고 중간에서 가장 높다.
            GLfloat height = 0.7f * std::sin(glm::radians(180.0f) * t);
            a.y += height;
            b.y -= height;
        }
        first.xPos = a.x; first.yPos = a.y; first.zPos = a.z;
        secound.xPos = b.x; secound.yPos = b.y; secound.zPos = b.z;
        if (animationProgress >= 1.0f) animationMode = 0;
        return;
    }
    if (selectedObject == 1 || selectedObject == 3 || animationMode == 3) moveShape(&first);
    if (selectedObject == 2 || selectedObject == 3 || animationMode == 3) moveShape(&secound);
    if (animationMode == 3) {
        first.size += sizeDirection * 0.002f;
        if (first.size >= 0.55f) { first.size = 0.55f; sizeDirection = -1; }
        if (first.size <= 0.15f) { first.size = 0.15f; sizeDirection = 1; }
        secound.size = 0.7f - first.size;
    }
}

void changeObject(object* object, int key, bool shift)
{
    int direction = shift ? -1 : 1;
    if (key == GLFW_KEY_X) {
        object->xDirection = object->xDirection == direction ? 0 : direction;
    }
    else if (key == GLFW_KEY_Y) {
        object->yDirection = object->yDirection == direction ? 0 : direction;
    }
    else if (key == GLFW_KEY_R) {
        object->orbitDirection = object->orbitDirection == direction ? 0 : direction;
    }
    else if (key == GLFW_KEY_A) {
        object->size += direction * 0.02f;
        object->size = glm::clamp(object->size, 0.08f, 0.7f);
    }
    else if (key == GLFW_KEY_B) {
        object->originSize += direction * 0.05f;
        object->originSize = glm::clamp(object->originSize, 0.25f, 1.8f);
    }
    else if (key == GLFW_KEY_D) object->xPos -= direction * 0.05f;
    else if (key == GLFW_KEY_E) object->yPos += direction * 0.05f;
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    bool continuous = key == GLFW_KEY_A || key == GLFW_KEY_B || key == GLFW_KEY_D || key == GLFW_KEY_E;
    if (action == GLFW_REPEAT && !continuous) return;
    if (key == GLFW_KEY_1 || key == GLFW_KEY_KP_1) selectedObject = 1;
    else if (key == GLFW_KEY_2 || key == GLFW_KEY_KP_2) selectedObject = 2;
    else if (key == GLFW_KEY_3 || key == GLFW_KEY_KP_3) selectedObject = 3;
    else if (key == GLFW_KEY_H) depthTest = !depthTest;
    else if (key == GLFW_KEY_C) {
        first.objectType = (first.objectType + 1) % 3;
        secound.objectType = (secound.objectType + 1) % 3;
    }
    else if (key == GLFW_KEY_S) resetMove();
    else if (key == GLFW_KEY_T) startAnimation(1);
    else if (key == GLFW_KEY_U) startAnimation(2);
    else if (key == GLFW_KEY_V) startAnimation(3);
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    else if (key == GLFW_KEY_X || key == GLFW_KEY_Y || key == GLFW_KEY_R || continuous) {
        if (animationMode != 0) {
            keepCurrentPosition(&first);
            keepCurrentPosition(&secound);
            animationMode = 0;
        }
        // 같은 조작 함수를 선택한 도형에만 적용한다.
        if (selectedObject == 1 || selectedObject == 3) changeObject(&first, key, shift);
        if (selectedObject == 2 || selectedObject == 3) changeObject(&secound, key, shift);
    }
}

void drawScene()
{
    if (depthTest) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 viewProjection = getViewProjection();
    drawAxes(viewProjection);
    drawShape(viewProjection, &first);
    drawShape(viewProjection, &secound);
}

int main()
{
    if (!loadOBJ("cube.obj", modelVertices[0]) ||
        !loadOBJ("cone.obj", modelVertices[1]) ||
        !loadOBJ("sphere.obj", modelVertices[2])) {
        return -1;
    }

    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "GLFW initialization failed.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 창을 만들 때 깊이값을 기록할 24비트 깊이 버퍼를 요청한다.
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-17", nullptr, nullptr);
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
    transformLocation = glGetUniformLocation(shaderProgram, "transform");
    initBuffer();
    resetMove();
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
        moveAnimation();
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
