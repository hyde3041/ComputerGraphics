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

struct Shape
{
    GLfloat Xpos;
    GLfloat Ypos;
    GLfloat Zpos;

    GLfloat Xsize;
    GLfloat Ysize;
    GLfloat Zsize;

    GLfloat Yangle;
    GLfloat Xangle;
    GLfloat Zangle;

    GLfloat Rcolor;
    GLfloat Gcolor;
    GLfloat Bcolor;
};

GLuint shaderProgram{}, VAO{}, VBO{};
GLint transformLocation{ -1 }, colorLocation{ -1 };
int windowWidth{ 800 }, windowHeight{ 600 };
std::vector<GLfloat> cubeVertices;

// 기본 카메라 설정. 필요할 때 위치/목표점을 바꾸어 사용한다.
// 카메라 초기값은 여기서 정한다. r을 눌러도 이 위치로 돌아온다.
const glm::vec3 initialCameraPosition(0.0f, 20.0f, 30.0f);
glm::vec3 cameraPosition = initialCameraPosition;
glm::vec3 cameraTarget(0.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

// 직접 작성: 실습에 필요한 구조체와 변수를 여기에 추가한다.
std::vector<Shape> shape;
GLfloat setT = 0.0;
int railCount = 0;
GLfloat setB = 0.0;
int boxCount = 0;
bool beltMove{ true }; // b: 기존처럼 이동하며 시작, 다시 누를 때마다 시작/정지
bool perspectiveView{ true }; // p: 원근투영, Shift+P: 직각투영

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
    vertexColor = drawColor;
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
    // 그릴 데이터 크기에 맞춰 버퍼를 확보하고 복사한다.
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(count) * 6 * sizeof(GLfloat), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(mode, 0, count);
}

glm::mat4 getViewProjection()
{
    GLfloat aspect = static_cast<GLfloat>(windowWidth) / windowHeight;
    glm::mat4 view = glm::lookAt(cameraPosition, cameraTarget, cameraUp);
    GLfloat fov = glm::radians(30.0f);
    glm::mat4 projection;
    if (perspectiveView) {
        projection = glm::perspective(fov, aspect, 0.1f, 50.0f);
    }
    else {
        // 직각투영에서도 줌이 보이도록 카메라 거리에 맞춰 화면 범위를 정한다.
        GLfloat halfHeight = glm::length(cameraPosition - cameraTarget) * std::tan(fov / 2.0f);
        projection = glm::ortho(-halfHeight * aspect, halfHeight * aspect,
            -halfHeight, halfHeight, 0.1f, 50.0f);
    }
    return projection * view;
}

void drawCube(const glm::mat4& viewProjection, const glm::mat4& parent,
    glm::vec3 position, GLfloat yAngle, glm::vec3 halfSize, glm::vec3 color)
{
    // 행렬은 이동 * 회전 * 크기 순서로 만든다. 점에는 오른쪽부터 적용된다.
    glm::mat4 model = glm::translate(parent, position);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(yAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-2.0f, 0.0f, 0.0f));
    model = glm::scale(model, halfSize);
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3f(colorLocation, color.r, color.g, color.b);
    drawVertices(cubeVertices.data(), static_cast<int>(cubeVertices.size() / 6), GL_TRIANGLES);
}

void resetScene()
{
    cameraPosition = initialCameraPosition;
    cameraTarget = glm::vec3(0.0f);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    perspectiveView = true;
    beltMove = true;
    shape.clear();
    setT = setB = 0.0f;
    railCount = boxCount = 0;
}

void setCube()
{
    Shape defalt{0.0f, 0.0f, -5.0f, 
        6.0f, 1.0f, 1.1f, 
        0.0f, 0.0f, 0.0f,
        0.3f, 0.3f, 0.3f};
    shape.push_back(defalt);

    defalt = { -6.1f, 0.5f, 0.0f,
        0.2f, 1.0f, 6.0f,
        0.0f, 0.0f, 0.0f,
        0.8f, 0.8f, 0.0f };
    shape.push_back(defalt);

    defalt = { -3.9f, 0.5f, 1.0f,
        0.2f, 1.0f, 5.0f,
        0.0f, 0.0f, 0.0f,
        0.8f, 0.8f, 0.0f };
    shape.push_back(defalt);

    defalt = { -5.0f, 0.0f, 0.0f, 
        1.1f, 1.0f, 6.0f, 
        0.0f, 0.0f, 0.0f,
        0.3f, 0.3f, 0.3f };
    shape.push_back(defalt);

    defalt = { 1.0f, 0.5f, -3.9f,
        5.0f, 1.0f, 0.2f,
        0.0f, 0.0f, 0.0f,
        0.8f, 0.8f, 0.0f };
    shape.push_back(defalt);

    defalt = { 0.0f, 0.5f, -6.1f,
        6.0f, 1.0f, 0.1f,
        0.0f, 0.0f, 0.0f,
        0.8f, 0.8f, 0.0f };
    shape.push_back(defalt);
}

void makeRail()
{
    
    if (setT > 9.5f) {
        railCount++;
        setT = 0.0f;
        Shape defalt = { -5.0f, 1.0f, 5.0f,
        1.0f, 0.2f, 0.2f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f };
        shape.push_back(defalt);
    }
}

void makeBox() 
{
    if (setB > 30.0f) {
        boxCount++;
        setB = 0.0f;
        Shape defalt = { -5.0f, 1.0f, 5.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        0.7f, 0.2f, 0.5f };
        shape.push_back(defalt);
    }
}

void moveScene()
{
    for (int i = 6; i < shape.size(); ++i) {
        if (shape[i].Zpos >= -3.0f) {
            shape[i].Zpos -= 0.01f;
        }
        else if (shape[i].Zpos <= -3.0f && shape[i].Yangle >= -90) {
            shape[i].Yangle -= 0.5f;
        }
        else if (-5.0f <= shape[i].Xpos && shape[i].Xpos <= 4.0f) {
            shape[i].Xpos += 0.01f;
        }
        else if (shape[i].Xpos >= 4.0f) {
            shape[i].Xpos = -5.0f;
            shape[i].Zpos = 5.5f;
            shape[i].Yangle = 0.0f;
        }
    }
    
    
    // 직접 작성: 매 프레임의 이동/애니메이션 코드를 넣는다.
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    bool zoomIn = key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD;
    bool zoomOut = key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT;
    bool cameraTurn = key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT;
    // b/p/r은 한 번만 처리하고, 줌과 방향키는 길게 눌러도 움직인다.
    if (action == GLFW_REPEAT && !zoomIn && !zoomOut && !cameraTurn) return;

    if (key == GLFW_KEY_B) {
        beltMove = !beltMove;
    }
    else if (zoomIn || zoomOut) {
        glm::vec3 offset = cameraPosition - cameraTarget;
        GLfloat distance = glm::length(offset);
        if (distance > 0.0f) {
            GLfloat nextDistance = glm::clamp(distance + (zoomIn ? -0.5f : 0.5f), 5.0f, 40.0f);
            // 시선을 유지한 채 목표점 쪽으로 가까워지거나 멀어진다.
            cameraPosition = cameraTarget + offset * (nextDistance / distance);
        }
    }
    else if (key == GLFW_KEY_P) {
        perspectiveView = (mods & GLFW_MOD_SHIFT) == 0;
    }
    else if (cameraTurn) {
        GLfloat angle = key == GLFW_KEY_LEFT ? -3.0f : 3.0f;
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),
            glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 offset = cameraPosition - cameraTarget;
        cameraPosition = cameraTarget + glm::vec3(rotation * glm::vec4(offset, 0.0f));
    }
    else if (key == GLFW_KEY_R) {
        resetScene();
        setCube();
    }
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 viewProjection = getViewProjection();
    glm::mat4 model = glm::mat4(1.0f);

    for (int i = 0; i < static_cast<int>(shape.size()); ++i) {
        drawCube(
            viewProjection,
            model,
            glm::vec3(shape[i].Xpos, shape[i].Ypos, shape[i].Zpos),
            shape[i].Yangle,
            glm::vec3(shape[i].Xsize, shape[i].Ysize, shape[i].Zsize),
            glm::vec3(shape[i].Rcolor, shape[i].Gcolor, shape[i].Bcolor)
        );
    }
}

int main()
{
    if (!loadOBJ("cube.obj", cubeVertices)) return -1;
    if (glfwInit() != GLFW_TRUE) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-20", nullptr, nullptr);
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
    setCube();
    glClearColor(0.07f, 0.09f, 0.13f, 1.0f);
    glfwSwapInterval(1);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSize(window, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetKeyCallback(window, keyCallback);
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        if (beltMove) moveScene();
        drawScene();
        glfwSwapBuffers(window);
        if (beltMove) {
            if (railCount < 20) {
                makeRail();
                setT += 0.1f;
            }
            if (boxCount < 5 && railCount >= 20) {
                makeBox();
                setB += 0.1f;
            }
        }
    }
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

