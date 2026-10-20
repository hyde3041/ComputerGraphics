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

GLuint shaderProgram{}, VAO{}, VBO{};
GLint transformLocation{ -1 }, colorLocation{ -1 };
int windowWidth{ 800 }, windowHeight{ 600 };
std::vector<GLfloat> sphereVertices;

struct Planet {
    GLfloat radius; // 중심 구에서 행성까지 거리
    GLfloat size;
    GLfloat angle;
    GLfloat speed;
    GLfloat tilt; // XZ 궤도 평면을 Z축으로 기울이는 각도
    GLfloat moonRadius;
    GLfloat moonAngle;
    GLfloat moonSpeed;
};
Planet planet[3]{
    { 1.55f, 0.20f, 0.0f, 0.7f, 0.0f, 0.42f, 30.0f, 1.5f },
    { 2.30f, 0.27f, 120.0f, 0.4f, 45.0f, 0.48f, 145.0f, 1.0f },
    { 3.00f, 0.24f, 240.0f, 0.25f, -45.0f, 0.46f, 260.0f, 1.2f }
};
const glm::vec3 planetColor[3]{ { 0.95f, 0.2f, 0.2f }, { 0.2f, 0.55f, 1.0f }, { 0.25f, 0.9f, 0.35f } };
const glm::vec3 moonColor[3]{ { 1.0f, 0.95f, 0.3f }, { 0.85f, 0.35f, 1.0f }, { 0.2f, 1.0f, 0.95f } };
bool perspectiveView{ true }, wireFrame{ false };
GLfloat xPos{}, yPos{}, zPos{};
GLfloat orbitScale{ 1.0f };
int orbitDirection{}; // 1번: 모든 행성/달 궤도의 반지름 증가 또는 감소
GLfloat centerSize{ 0.55f };
int centerSizeDirection{ 1 };
bool centerMove{}; // 2번: 중심 구의 크기/색 애니메이션
GLfloat moonPlaneRotation{};
bool moonPlaneMove{}; // 3번: 달 궤도를 각 행성의 Z축으로 회전

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
    perspectiveView = true;
    wireFrame = false;
    xPos = yPos = zPos = 0.0f;
    orbitScale = 1.0f;
    orbitDirection = 0;
    centerSize = 0.55f;
    centerSizeDirection = 1;
    centerMove = false;
    moonPlaneRotation = 0.0f;
    moonPlaneMove = false;
    for (int i = 0; i < 3; ++i) {
        planet[i].angle = i * 120.0f;
        planet[i].moonAngle = 30.0f + i * 115.0f;
    }
}

glm::mat4 getView()
{
    return glm::lookAt(glm::vec3(4.5f, 3.5f, 8.0f), glm::vec3(0.0f), glm::vec3(0, 1, 0));
}

glm::mat4 getViewProjection()
{
    GLfloat aspect = static_cast<GLfloat>(windowWidth) / windowHeight;
    glm::mat4 projection;
    if (perspectiveView) projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 40.0f);
    else projection = glm::ortho(-4.7f * aspect, 4.7f * aspect, -4.7f, 4.7f, 0.1f, 40.0f);
    return projection * getView();
}

void drawSphere(const glm::mat4& viewProjection, glm::mat4 model, GLfloat size, glm::vec3 color)
{
    model = glm::scale(model, glm::vec3(size));
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3f(colorLocation, color.r, color.g, color.b);
    drawVertices(sphereVertices.data(), static_cast<int>(sphereVertices.size() / 6), GL_TRIANGLES);
}

void drawOrbit(const glm::mat4& viewProjection, const glm::mat4& model, GLfloat radius, bool xzPlane, glm::vec3 color)
{
    const int count = 120;
    GLfloat vertices[count * 6]{};
    for (int i = 0; i < count; ++i) {
        GLfloat angle = glm::radians(i * 360.0f / count);
        vertices[i * 6] = radius * std::cos(angle);
        if (xzPlane) vertices[i * 6 + 2] = radius * std::sin(angle);
        else vertices[i * 6 + 1] = radius * std::sin(angle);
        vertices[i * 6 + 3] = vertices[i * 6 + 4] = vertices[i * 6 + 5] = 1.0f;
    }
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3f(colorLocation, color.r, color.g, color.b);
    drawVertices(vertices, count, GL_LINE_LOOP);
}

void moveScene()
{
    for (int i = 0; i < 3; ++i) {
        planet[i].angle += planet[i].speed;
        planet[i].moonAngle += planet[i].moonSpeed;
        if (planet[i].angle >= 360.0f) planet[i].angle -= 360.0f;
        if (planet[i].moonAngle >= 360.0f) planet[i].moonAngle -= 360.0f;
    }
    orbitScale += orbitDirection * 0.003f;
    if (orbitScale >= 1.25f) { orbitScale = 1.25f; orbitDirection = 0; }
    if (orbitScale <= 0.8f) { orbitScale = 0.8f; orbitDirection = 0; }
    if (centerMove) {
        centerSize += centerSizeDirection * 0.002f;
        if (centerSize >= 0.75f) { centerSize = 0.75f; centerSizeDirection = -1; }
        if (centerSize <= 0.45f) { centerSize = 0.45f; centerSizeDirection = 1; }
    }
    if (moonPlaneMove) {
        moonPlaneRotation += 0.5f;
        if (moonPlaneRotation >= 360.0f) moonPlaneRotation -= 360.0f;
    }
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    bool moving = key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D ||
        key == GLFW_KEY_EQUAL || key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_ADD || key == GLFW_KEY_KP_SUBTRACT;
    if (action == GLFW_REPEAT && !moving) return;
    if (key == GLFW_KEY_P) perspectiveView = shift; // p: 직각, Shift+P: 원근
    else if (key == GLFW_KEY_M) wireFrame = shift; // m: 솔리드, Shift+M: 와이어
    else if (key == GLFW_KEY_A) xPos -= 0.1f;
    else if (key == GLFW_KEY_D) xPos += 0.1f;
    else if (key == GLFW_KEY_W) yPos += 0.1f;
    else if (key == GLFW_KEY_S) yPos -= 0.1f;
    else if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) zPos += 0.1f;
    else if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) zPos -= 0.1f;
    else if (key == GLFW_KEY_1 || key == GLFW_KEY_KP_1) {
        // 움직이는 중에는 반대로, 끝에 도착해 멈췄으면 반대 끝을 향해 움직인다.
        if (orbitDirection != 0) orbitDirection = -orbitDirection;
        else orbitDirection = orbitScale >= 1.25f ? -1 : 1;
    }
    else if (key == GLFW_KEY_2 || key == GLFW_KEY_KP_2) centerMove = !centerMove;
    else if (key == GLFW_KEY_3 || key == GLFW_KEY_KP_3) moonPlaneMove = !moonPlaneMove;
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
    glm::mat4 viewProjection = getViewProjection();
    glm::mat4 root = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, yPos, zPos));
    GLfloat colorRatio = (centerSize - 0.45f) / 0.3f;
    glm::vec3 centerColor(1.0f, 0.25f + colorRatio * 0.4f, 0.05f + colorRatio * 0.1f);
    drawSphere(viewProjection, root, centerSize, centerColor);

    // view에서 이동을 빼고 회전의 역행렬을 쓰면, 로컬 XY 평면이 화면과 평행해진다.
    glm::mat4 screenPlane = glm::mat4(glm::inverse(glm::mat3(getView())));
    for (int i = 0; i < 3; ++i) {
        glm::mat4 plane = glm::rotate(root, glm::radians(planet[i].tilt), glm::vec3(0, 0, 1));
        GLfloat radius = planet[i].radius * orbitScale;
        drawOrbit(viewProjection, plane, radius, true, planetColor[i] * 0.65f);

        glm::mat4 planetModel = glm::rotate(plane, glm::radians(planet[i].angle), glm::vec3(0, 1, 0));
        planetModel = glm::translate(planetModel, glm::vec3(radius, 0, 0));
        drawSphere(viewProjection, planetModel, planet[i].size, planetColor[i]);

        // 행성 행렬의 4번째 열([3])은 현재 행성의 최종 위치이다.
        // 행성의 공전 기울기는 빼고 위치만 물려받아 달 궤도를 화면에 평행하게 시작한다.
        glm::mat4 moonPlane = glm::translate(glm::mat4(1.0f), glm::vec3(planetModel[3]));
        moonPlane = glm::rotate(moonPlane, glm::radians(moonPlaneRotation), glm::vec3(0, 0, 1));
        moonPlane *= screenPlane;
        
        GLfloat moonRadius = planet[i].moonRadius * orbitScale;
        drawOrbit(viewProjection, moonPlane, moonRadius, false, moonColor[i] * 0.6f);
        glm::mat4 moonModel = glm::rotate(moonPlane, glm::radians(planet[i].moonAngle), glm::vec3(0, 0, 1));
        moonModel = glm::translate(moonModel, glm::vec3(moonRadius, 0, 0));
        drawSphere(viewProjection, moonModel, 0.075f, moonColor[i]);
    }
}

int main()
{
    if (!loadOBJ("sphere.obj", sphereVertices)) return -1;
    if (glfwInit() != GLFW_TRUE) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-18", nullptr, nullptr);
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
    glClearColor(0.025f, 0.025f, 0.06f, 1.0f);
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
