#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream> // ifstream: 파일을 읽는다.
#include <sstream> // istringstream: 한 줄의 내용을 나누어 읽는다.
#include <string>
#include <vector> // vector: 읽은 점 개수에 맞게 저장 공간이 늘어난다.

#ifndef OBJ_DIRECTORY
#define OBJ_DIRECTORY "models"
#endif

struct Rectangle
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

    GLfloat Xmove{};
    GLfloat Ymove{};
    GLfloat Zmove{};
};

std::vector<Rectangle> rec;

struct Sphere
{
    GLfloat Xpos;
    GLfloat Ypos;
    GLfloat Zpos;
    GLfloat radius;

    GLfloat Rcolor;
    GLfloat Gcolor;
    GLfloat Bcolor;

    // 매 프레임 이동량이다. 양수/음수로 이동 방향을 정한다.
    GLfloat Xmove{ 0.003f };
    GLfloat Ymove{ 0.004f };
    GLfloat Zmove{ 0.008f };
};

std::vector<Sphere> cir;

GLuint shaderProgram{}, VAO{}, VBO{};
GLint transformLocation{-1}, colorLocation{-1};
int windowWidth{800}, windowHeight{600};
std::vector<GLfloat> cubeVertices;
std::vector<GLfloat> sphereVertices;
glm::vec3 cameraPosition(0.0f, 0.0f, 6.0f);
glm::vec3 cameraTarget(0.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

bool mouseDrag{ false };
double lastMouseX{};
bool floorOpen{ false };
GLfloat floorAngle{}; // F: 정면에 보이는 큰 면(뒤쪽 Z=-1)을 0~90도로 여닫는다.


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
                    // XYZRGB 형식이다. 최종 도형 색은 drawColor로 지정한다.
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
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    return projection * view;
}

// 받은 색 하나로 육면체 전체를 그린다.
void drawCube(const glm::mat4& viewProjection, glm::vec3 position, glm::vec3 halfSize,
    glm::vec3 color, glm::vec3 rotation = glm::vec3(0.0f))
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, halfSize);
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3f(colorLocation, color.r, color.g, color.b);
    drawVertices(cubeVertices.data(), static_cast<int>(cubeVertices.size() / 6), GL_TRIANGLES);
}

// 면마다 다른 색으로 나누어 그린다.
void drawRec(const glm::mat4& viewProjection, glm::vec3 position, glm::vec3 halfSize)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, halfSize);
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    const GLfloat faceColor[6][3]{
    {1.0f, 0.0f, 0.0f}, // 위: 빨강
    {0.0f, 1.0f, 0.0f}, // 앞: 초록
    {0.0f, 0.0f, 1.0f}, // 왼쪽: 파랑
    {1.0f, 1.0f, 0.0f}, // 아래: 노랑
    {1.0f, 0.0f, 1.0f}, // 오른쪽: 보라
    {0.0f, 1.0f, 1.0f}  // 뒤: 청록
    };

    for (int i = 0; i < 6; ++i) {
        if (i == 1) continue; // 앞면은 그리지 않아 방 안이 보이게 한다.

        glm::mat4 faceModel = model;
        if (i == 5) { // 정면에 보이는 청록색 면: 아래 모서리의 X축을 경첩으로 사용한다.
            faceModel = glm::translate(faceModel, glm::vec3(0.0f, -1.0f, -1.0f));
            faceModel = glm::rotate(faceModel, glm::radians(-floorAngle), glm::vec3(1.0f, 0.0f, 0.0f));
            faceModel = glm::translate(faceModel, glm::vec3(0.0f, 1.0f, 1.0f));
        }
        transform = viewProjection * faceModel;
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform3f(colorLocation,
            faceColor[i][0], faceColor[i][1], faceColor[i][2]);

        drawVertices(cubeVertices.data() + i * 36,
            6, GL_TRIANGLES);
    }
}

// sphere.obj로 구를 그린다. 원본 구의 반지름은 1이다.
void drawSphere(const glm::mat4& viewProjection, glm::vec3 position, GLfloat radius, glm::vec3 color)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(radius));
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3f(colorLocation, color.r, color.g, color.b);
    drawVertices(sphereVertices.data(), static_cast<int>(sphereVertices.size() / 6), GL_TRIANGLES);
}

// rec[0]은 방이다. 방과 그 안의 물체에 같은 행렬을 곱한다.
glm::mat4 getRoomModel()
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(rec[0].Xpos, rec[0].Ypos, rec[0].Zpos));
    model = glm::rotate(model, glm::radians(rec[0].Yangle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rec[0].Xangle), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rec[0].Zangle), glm::vec3(0.0f, 0.0f, 1.0f));
    return model;
}

void addSphere()
{
    if (cir.size() >= 5) return;

    Sphere sphere{ 0.0f, 0.0f, 0.0f, 0.2f, 0.3f, 0.3f, 0.3f };
    // 공을 추가할 때만 속도와 방향을 정한다.
    sphere.Xmove = (std::rand() % 7 + 2) * 0.001f;
    sphere.Ymove = (std::rand() % 7 + 2) * 0.001f;
    sphere.Zmove = (std::rand() % 7 + 2) * 0.001f;
    if (std::rand() % 2 == 0) sphere.Xmove = -sphere.Xmove;
    if (std::rand() % 2 == 0) sphere.Ymove = -sphere.Ymove;
    if (std::rand() % 2 == 0) sphere.Zmove = -sphere.Zmove;
    cir.push_back(sphere);
}

void setScene()
{
    rec.clear();
    cir.clear();
    floorOpen = false;
    floorAngle = 0.0f;
    mouseDrag = false;

    Rectangle rectangle{
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        0.3f, 0.3f, 0.3f };
    rec.push_back(rectangle); // rec[0]: 방

    // rec[1]~rec[3]: 바닥 위에 Z축을 따라 놓인 서로 다른 크기의 큐브
    rectangle = {
        0.0f, -0.74f, -0.6f,
        0.22f, 0.26f, 0.18f,
        0.0f, 0.0f, 0.0f,
        1.0f, 0.35f, 0.15f };
    rec.push_back(rectangle);
    rectangle = {
        0.0f, -0.80f, 0.0f,
        0.18f, 0.20f, 0.16f,
        0.0f, 0.0f, 0.0f,
        0.9f, 0.55f, 0.15f };
    rec.push_back(rectangle);
    rectangle = {
        0.0f, -0.86f, 0.6f,
        0.12f, 0.14f, 0.12f,
        0.0f, 0.0f, 0.0f,
        0.8f, 0.25f, 0.1f };
    rec.push_back(rectangle);

    addSphere(); // 처음에는 1개, B를 누를 때마다 최대 5개까지 추가
}

void moveScene()
{
    if (rec.empty()) return;

    if (floorOpen && floorAngle < 90.0f) floorAngle += 1.0f;
    else if (!floorOpen && floorAngle > 0.0f) floorAngle -= 1.0f;

    // 화면 아래쪽 중력을 방 내부의 좌표로 바꾼다.
    // inverse는 행렬의 변환을 거꾸로 적용한다. w=0이면 이동은 무시하고 방향만 변환한다.
    glm::vec3 gravity = glm::vec3(glm::inverse(getRoomModel()) * glm::vec4(0.0f, -0.0003f, 0.0f, 0.0f));

    for (int i = 1; i < static_cast<int>(rec.size()); ++i) {
        GLfloat minZ = -rec[0].Zsize + rec[i].Zsize;
        // 이미 열린 면을 넘어갔다면, 다시 닫아도 안쪽으로 계속 떨어진다.
        bool outside = rec[i].Zpos < minZ;
        if (floorAngle >= 90.0f || outside) {
            rec[i].Zmove -= 0.0003f; // 큰 면 너머인 -Z 방향으로 점점 빨라진다.
        }
        else {
            rec[i].Xmove += gravity.x;
            rec[i].Ymove += gravity.y;
            rec[i].Zmove += gravity.z;
        }
        rec[i].Xpos += rec[i].Xmove;
        rec[i].Ypos += rec[i].Ymove;
        rec[i].Zpos += rec[i].Zmove;
        if (outside) continue;

        // 방 안의 좌표이므로 방 중심은 항상 (0, 0, 0)이다.
        GLfloat minX = -rec[0].Xsize + rec[i].Xsize;
        GLfloat maxX = rec[0].Xsize - rec[i].Xsize;
        GLfloat minY = -rec[0].Ysize + rec[i].Ysize;
        GLfloat maxY = rec[0].Ysize - rec[i].Ysize;
        GLfloat maxZ = rec[0].Zsize - rec[i].Zsize;

        // 큐브는 벽에 튕기지 않고 멈춘 뒤, 기울어진 바닥을 따라 미끄러진다.
        if (rec[i].Xpos < minX) { rec[i].Xpos = minX; rec[i].Xmove = 0.0f; }
        else if (rec[i].Xpos > maxX) { rec[i].Xpos = maxX; rec[i].Xmove = 0.0f; }
        if (rec[i].Ypos <= minY) {
            rec[i].Ypos = minY;
            rec[i].Ymove = 0.0f;
        }
        else if (rec[i].Ypos > maxY) { rec[i].Ypos = maxY; rec[i].Ymove = 0.0f; }
        if (rec[i].Zpos < minZ && floorAngle < 90.0f) { rec[i].Zpos = minZ; rec[i].Zmove = 0.0f; }
        else if (rec[i].Zpos > maxZ) { rec[i].Zpos = maxZ; rec[i].Zmove = 0.0f; }
    }

    for (int i = 0; i < static_cast<int>(cir.size()); ++i) {
        GLfloat minZ = -rec[0].Zsize + cir[i].radius;
        bool outside = cir[i].Zpos < minZ;
        // 별도의 falling 변수 없이, 면이 열렸거나 이미 넘어갔으면 -Z로 가속한다.
        if (floorAngle >= 90.0f || outside) cir[i].Zmove -= 0.0003f;
        cir[i].Xpos += cir[i].Xmove;
        cir[i].Ypos += cir[i].Ymove;
        cir[i].Zpos += cir[i].Zmove;
        if (outside) continue;

        GLfloat minX = -rec[0].Xsize + cir[i].radius;
        GLfloat maxX = rec[0].Xsize - cir[i].radius;
        GLfloat minY = -rec[0].Ysize + cir[i].radius;
        GLfloat maxY = rec[0].Ysize - cir[i].radius;
        GLfloat maxZ = rec[0].Zsize - cir[i].radius;

        if (cir[i].Xpos <= minX) {
            cir[i].Xpos = minX;
            cir[i].Xmove = -cir[i].Xmove;
        }
        else if (cir[i].Xpos >= maxX) {
            cir[i].Xpos = maxX;
            cir[i].Xmove = -cir[i].Xmove;
        }
        if (cir[i].Ypos <= minY) {
            cir[i].Ypos = minY;
            cir[i].Ymove = -cir[i].Ymove;
        }
        else if (cir[i].Ypos >= maxY) {
            cir[i].Ypos = maxY;
            cir[i].Ymove = -cir[i].Ymove;
        }
        if (cir[i].Zpos <= minZ && floorAngle < 90.0f) {
            cir[i].Zpos = minZ;
            cir[i].Zmove = -cir[i].Zmove;
        }
        else if (cir[i].Zpos >= maxZ) {
            cir[i].Zpos = maxZ;
            cir[i].Zmove = -cir[i].Zmove;
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    if (key == GLFW_KEY_Z) {
        rec[0].Zpos += shift ? -0.1f : 0.1f;
    }
    else if (key == GLFW_KEY_Y) {
        rec[0].Yangle += shift ? 5.0f : -5.0f;
    }
    else if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_B) addSphere();
        else if (key == GLFW_KEY_F) floorOpen = !floorOpen;
        else if (key == GLFW_KEY_R) setScene();
        else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void mouseButton(GLFWwindow* window, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    mouseDrag = (action == GLFW_PRESS);
    double y;
    glfwGetCursorPos(window, &lastMouseX, &y);
}

void cursorPosition(GLFWwindow*, double x, double)
{
    if (mouseDrag) {
        // 왼쪽으로 드래그하면 +Z 회전, 오른쪽으로 드래그하면 -Z 회전
        rec[0].Zangle += static_cast<GLfloat>(x - lastMouseX) * 0.3f;
    }
    lastMouseX = x;
}

void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (rec.empty()) return;

    // 내부 좌표를 먼저 방의 위치/각도로 변환하고, 그 결과를 카메라로 본다.
    glm::mat4 viewProjection = getViewProjection() * getRoomModel();
    glCullFace(GL_FRONT); // 방은 안쪽 면을 그린다.
    drawRec(viewProjection, glm::vec3(0.0f),
        glm::vec3(rec[0].Xsize, rec[0].Ysize, rec[0].Zsize));

    glCullFace(GL_BACK); // 내부 큐브와 공은 바깥쪽 면을 그린다.
    for (int i = 1; i < static_cast<int>(rec.size()); ++i) {
        drawCube(viewProjection,
            glm::vec3(rec[i].Xpos, rec[i].Ypos, rec[i].Zpos),
            glm::vec3(rec[i].Xsize, rec[i].Ysize, rec[i].Zsize),
            glm::vec3(rec[i].Rcolor, rec[i].Gcolor, rec[i].Bcolor),
            glm::vec3(rec[i].Xangle, rec[i].Yangle, rec[i].Zangle));
    }
    for (int i = 0; i < static_cast<int>(cir.size()); ++i) {
        drawSphere(viewProjection,
            glm::vec3(cir[i].Xpos, cir[i].Ypos, cir[i].Zpos),
            cir[i].radius,
            glm::vec3(cir[i].Rcolor, cir[i].Gcolor, cir[i].Bcolor));
    }
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    // 프로젝트 설정의 OBJ_DIRECTORY에서 cube.obj를 읽는다.
    if (!loadOBJ("cube.obj", cubeVertices)) return -1;
    if (!loadOBJ("sphere.obj", sphereVertices)) return -1;
    if (glfwInit() != GLFW_TRUE) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-21", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window); glfwTerminate(); return -1;
    }
    shaderProgram = createShader();
    if (!shaderProgram) {
        glfwDestroyWindow(window); glfwTerminate(); return -1;
    }
    glUseProgram(shaderProgram);
    transformLocation = glGetUniformLocation(shaderProgram, "transform");
    colorLocation = glGetUniformLocation(shaderProgram, "drawColor");
    initBuffer();
    setScene();
    glEnable(GL_DEPTH_TEST); // 가까운 면이 먼 면을 가리도록
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);   // 앞을 향한 면 제거
    glClearColor(0.07f, 0.09f, 0.13f, 1.0f);
    glfwSwapInterval(1);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSize(window, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButton);
    glfwSetCursorPosCallback(window, cursorPosition);
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
