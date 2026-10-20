#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

GLuint shaderProgram{};
GLuint VAO{};
GLuint VBO{};
GLint transformLocation{ -1 };
int windowWidth{ 800 };
int windowHeight{ 600 };

int objectType{ 0 }; // 0: 육면체, 1: 사각뿔
bool depthTest{ true };
bool wireFrame{ false };
GLfloat xPos{};
GLfloat yPos{};
GLfloat xRotation{ 10.0f };
GLfloat yRotation{ 10.0f };
int xDirection{};
int yDirection{};

GLfloat zUpsideRotation{}; // 윗면만 따로 돌리는 각도
int zUpsideDirection{};

GLfloat xFrontRotation{}; // 앞면이 열린 각도: 0도는 닫힘, 90도는 열림
int xFrontDirection{}; // 1: 열기, -1: 닫기, 0: 멈춤

GLfloat sideRotation{};
int sideDirection{};

GLfloat backSize{ 1.0f }; // 1: 원래 크기, 0: 사라짐
int backDirection{}; // -1: 작아짐, 1: 커짐, 0: 멈춤

int randomTwoSide = 4;
GLfloat twoSideRotation{};
int twoSideDirection{};

int randomChangeSide = 4;
GLfloat twoChangeRotation{};
int twoChangeDirection{};

GLfloat yFourSideRotation{}; // 네 옆면이 중앙 Y축을 도는 각도
int yFourSideDirection{};

GLfloat xFourFaceRotation{}; // 위, 아래, 앞, 뒤 면이 중앙 X축을 도는 각도
int xFourFaceDirection{};




int pyramidMove{}; // 0: 정지, 6~9: 사각뿔 애니메이션 종류

GLfloat pyramidAllRotation{}; // 6번: 네 옆면을 함께 여는 실제 각도
int pyramidAllDirection{};

GLfloat pyramidOneRotation[4]{}; // 7번: 앞, 오른쪽, 뒤, 왼쪽 면의 실제 각도
int pyramidOneDirection{};
int pyramidFace{}; // 현재 움직이고 있는 면 번호

GLfloat pyramidSwapRotation{}; // 8번: 앞, 뒤 면을 교환하는 각도
int pyramidSwapDirection{};

GLfloat pyramidTopRotation{}; // 9번: 꼭짓점을 고정하고 벌리는 각도
int pyramidTopDirection{};

const GLfloat cubeVertices[8][6] = {
    {-0.35f, -0.35f,  0.35f, 1.0f, 0.0f, 0.0f},
    { 0.35f, -0.35f,  0.35f, 0.0f, 1.0f, 0.0f},
    { 0.35f,  0.35f,  0.35f, 0.0f, 0.0f, 1.0f},
    {-0.35f,  0.35f,  0.35f, 1.0f, 1.0f, 0.0f},
    {-0.35f, -0.35f, -0.35f, 1.0f, 0.0f, 1.0f},
    { 0.35f, -0.35f, -0.35f, 0.0f, 1.0f, 1.0f},
    { 0.35f,  0.35f, -0.35f, 1.0f, 0.5f, 0.0f},
    {-0.35f,  0.35f, -0.35f, 1.0f, 1.0f, 1.0f}
};

const GLfloat pyramidVertices[5][6] = {
    {-0.35f, -0.35f,  0.35f, 1.0f, 0.0f, 0.0f},
    { 0.35f, -0.35f,  0.35f, 0.0f, 1.0f, 0.0f},
    { 0.35f, -0.35f, -0.35f, 0.0f, 0.0f, 1.0f},
    {-0.35f, -0.35f, -0.35f, 1.0f, 1.0f, 0.0f},
    { 0.0f,   0.4f,   0.0f, 1.0f, 0.0f, 1.0f}
};

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

// 육면체: Y축이 높이이고, 앞면은 +Z쪽이다.
const int cubeFaces[6][4] = {
    {3, 2, 6, 7}, // 0: 윗면   (Y = +0.35)
    {4, 5, 1, 0}, // 1: 아랫면 (Y = -0.35)
    {4, 0, 3, 7}, // 2: 왼쪽   (X = -0.35)
    {1, 5, 6, 2}, // 3: 오른쪽 (X = +0.35)
    {0, 1, 2, 3}, // 4: 앞면   (Z = +0.35)
    {5, 4, 7, 6}  // 5: 뒷면   (Z = -0.35)
};

// 사각뿔도 Y축이 높이인 기준
const int pyramidFaces[5][4] = {
    {0, 1, 4, 0}, // 0: +Z쪽 옆면
    {1, 2, 4, 0}, // 1: +X쪽 옆면
    {2, 3, 4, 0}, // 2: -Z쪽 옆면
    {3, 0, 4, 0}, // 3: -X쪽 옆면
    {3, 2, 1, 0}  // 4: 바닥 (Y = -0.35)
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
    // 앞면이 열리면서 카메라 쪽으로 나와도 잘리지 않도록 near를 0.1로 둔다.
    glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, 0.1f, 10.0f);
    return projection * view;
}

void initBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 36 * 6 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
}

void drawVertices(const GLfloat* vertices, int count, GLenum mode)
{
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * 6 * sizeof(GLfloat), vertices);
    glDrawArrays(mode, 0, count);
}

void drawAxes(const glm::mat4& viewProjection)
{
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));
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

void upFaceRotateZ(GLfloat* vertices)
{
    // 윗면 중심 (0, 0.35, 0)을 지나는 Z축을 기준으로 회전한다.
    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::translate(faceModel, glm::vec3(0.0f, 0.35f, 0.0f));
    faceModel = glm::rotate(faceModel, glm::radians(zUpsideRotation), glm::vec3(0.0f, 0.0f, 1.0f));
    faceModel = glm::translate(faceModel, glm::vec3(0.0f, -0.35f, 0.0f));

    for (int i = 0; i < 6; ++i) {
        // 현재 면의 XYZ를 점 하나로 묶는다. W=1이라 이동도 적용된다.
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        // 변환 결과는 현재 면의 좌표 사본에만 넣는다. RGB는 그대로 둔다.
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void frontFaceRotateX(GLfloat* vertices)
{
    // 앞면 위쪽 모서리의 가운데는 (0, 0.35, 0.35)이고, 모서리는 X축 방향이다.
    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::translate(faceModel, glm::vec3(0.0f, 0.35f, 0.35f));
    // 앞면은 +Z쪽이므로 음의 X축 회전으로 바깥쪽으로 연다.
    faceModel = glm::rotate(faceModel, glm::radians(-xFrontRotation), glm::vec3(1.0f, 0.0f, 0.0f));
    faceModel = glm::translate(faceModel, glm::vec3(0.0f, -0.35f, -0.35f));
    // 점에는 오른쪽 행렬부터 적용된다: 위쪽 모서리를 원점으로 이동 -> 회전 -> 복귀.
    for (int i = 0; i < 6; ++i) {
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void sideRotate(GLfloat* vertices)
{
    // 양쪽 옆면의 중심은 X축 위에 있으므로 X축 회전만으로 제자리에서 돈다.
    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::rotate(faceModel, glm::radians(sideRotation), glm::vec3(1.0f, 0.0f, 0.0f));

    for (int i = 0; i < 6; ++i) {
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void backScale(GLfloat* vertices)
{
    glm::mat4 faceModel = glm::mat4(1.0f);
    // 뒷면의 Z 위치는 유지하고, 가로 X와 세로 Y만 줄인다.
    faceModel = glm::scale(faceModel, glm::vec3(backSize, backSize, 1.0f));

    for (int i = 0; i < 6; ++i) {
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void yTwoRotate(GLfloat* vertices)
{
    // 선택한 두 면이 공유하는 세로 모서리의 X, Z 위치이다.
    GLfloat pivotX = (randomTwoSide == 0 || randomTwoSide == 3) ? -0.35f : 0.35f;
    GLfloat pivotZ = (randomTwoSide == 0 || randomTwoSide == 1) ? 0.35f : -0.35f;
    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::translate(faceModel, glm::vec3(pivotX, 0.0f, pivotZ));
    faceModel = glm::rotate(faceModel, glm::radians(twoSideRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    faceModel = glm::translate(faceModel, glm::vec3(-pivotX, 0.0f, -pivotZ));

    for (int i = 0; i < 6; ++i) {
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void twoChageRotate(GLfloat* vertices, int direction)
{
    glm::mat4 faceModel = glm::mat4(1.0f);
    // 두 옆면을 육면체 중앙의 Y축 주위로 반대 방향으로 돌린다.
    faceModel = glm::rotate(faceModel, glm::radians(twoChangeRotation * direction), glm::vec3(0.0f, 1.0f, 0.0f));

    for (int i = 0; i < 6; ++i) {
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void fourSideRotateY(GLfloat* vertices)
{
    // 육면체 중심이 원점이므로 따로 이동하지 않고 Y축 회전만 적용한다.
    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::rotate(faceModel, glm::radians(yFourSideRotation), glm::vec3(0.0f, 1.0f, 0.0f));

    for (int i = 0; i < 6; ++i) {
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void fourFaceRotateX(GLfloat* vertices)
{
    // 중앙 X축 주위로 돌리면 윗면, 앞면, 아랫면, 뒷면의 위치가 차례로 바뀐다.
    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::rotate(faceModel, glm::radians(xFourFaceRotation), glm::vec3(1.0f, 0.0f, 0.0f));

    for (int i = 0; i < 6; ++i) {
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void pyramidOpenAll(GLfloat* vertices, int index)
{
    // 6번: 네 옆면을 아래 모서리 기준으로 열어 바닥 아래에서 만나게 한다.
    glm::vec3 pivot;
    glm::vec3 axis;
    // 각 옆면의 아래 모서리 가운데와 그 모서리 방향이다.
    if (index == 0) {
        pivot = glm::vec3(0.0f, -0.35f, 0.35f);
        axis = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    else if (index == 1) {
        pivot = glm::vec3(0.35f, -0.35f, 0.0f);
        axis = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    else if (index == 2) {
        pivot = glm::vec3(0.0f, -0.35f, -0.35f);
        axis = glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    else {
        pivot = glm::vec3(-0.35f, -0.35f, 0.0f);
        axis = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::translate(faceModel, pivot);
    faceModel = glm::rotate(faceModel, glm::radians(pyramidAllRotation), axis);
    faceModel = glm::translate(faceModel, -pivot);

    for (int i = 0; i < 3; ++i) { // 옆면은 삼각형이므로 꼭짓점 3개만 변환한다.
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void pyramidOpenOne(GLfloat* vertices, int index)
{
    // 7번: 현재 면을 아래 모서리 기준으로 바닥 평면까지 연다.
    // 한 면씩 각도를 늘리는 순서는 moveShape()에서 정한다.
    glm::vec3 pivot;
    glm::vec3 axis;
    // 각 옆면의 아래 모서리 가운데와 그 모서리 방향이다.
    if (index == 0) {
        pivot = glm::vec3(0.0f, -0.35f, 0.35f);
        axis = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    else if (index == 1) {
        pivot = glm::vec3(0.35f, -0.35f, 0.0f);
        axis = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    else if (index == 2) {
        pivot = glm::vec3(0.0f, -0.35f, -0.35f);
        axis = glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    else {
        pivot = glm::vec3(-0.35f, -0.35f, 0.0f);
        axis = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::translate(faceModel, pivot);
    faceModel = glm::rotate(faceModel, glm::radians(pyramidOneRotation[index]), axis);
    faceModel = glm::translate(faceModel, -pivot);

    for (int i = 0; i < 3; ++i) { // 옆면은 삼각형이므로 꼭짓점 3개만 변환한다.
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void pyramidSwapFaces(GLfloat* vertices)
{
    // 8번: 꼭짓점을 지나는 Y축을 기준으로 앞, 뒤 면의 자리를 교환한다.
    glm::vec3 pivot(0.0f, 0.4f, 0.0f);
    glm::vec3 axis(0.0f, 1.0f, 0.0f);
    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::translate(faceModel, pivot);
    faceModel = glm::rotate(faceModel, glm::radians(pyramidSwapRotation), axis);
    faceModel = glm::translate(faceModel, -pivot);

    for (int i = 0; i < 3; ++i) { // 옆면은 삼각형이므로 꼭짓점 3개만 변환한다.
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void pyramidOpenFromTop(GLfloat* vertices, int index)
{
    // 9번: 꼭짓점은 고정하고 아래 모서리가 바깥으로 벌어지게 한다.
    glm::vec3 pivot(0.0f, 0.4f, 0.0f);
    glm::vec3 axis;
    if (index == 0) axis = glm::vec3(1.0f, 0.0f, 0.0f);
    else if (index == 1) axis = glm::vec3(0.0f, 0.0f, -1.0f);
    else if (index == 2) axis = glm::vec3(-1.0f, 0.0f, 0.0f);
    else axis = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::mat4 faceModel = glm::mat4(1.0f);
    faceModel = glm::translate(faceModel, pivot);
    // 음의 각도를 주면 아래 모서리가 바깥쪽으로 벌어진다.
    faceModel = glm::rotate(faceModel, glm::radians(-pyramidTopRotation), axis);
    faceModel = glm::translate(faceModel, -pivot);
    for (int i = 0; i < 3; ++i) { // 옆면은 삼각형이므로 꼭짓점 3개만 변환한다.
        glm::vec4 point(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
        point = faceModel * point;
        vertices[i * 6] = point.x;
        vertices[i * 6 + 1] = point.y;
        vertices[i * 6 + 2] = point.z;
    }
}

void drawShape(const glm::mat4& viewProjection)
{
    const int order[6]{ 0, 1, 2, 0, 2, 3 };
    int faceCount = objectType == 0 ? 6 : 5;
    glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);

    for (int i = 0; i < faceCount; ++i) {
        // 면마다 기본 설정부터 다시 만든다.
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xPos, yPos, 0.0f));
        model = glm::rotate(model, glm::radians(yRotation), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(xRotation), glm::vec3(1, 0, 0));

        // 면의 회전은 좌표에 직접 적용하므로 여기에는 전체 도형 변환만 넣는다.
        glm::mat4 transform = viewProjection * model;
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));

        GLfloat vertices[6 * 6]{};
        int count = (objectType == 1 && i < 4) ? 3 : 6;
        for (int j = 0; j < count; ++j) {
            int index = objectType == 0 ? cubeFaces[i][order[j]] : pyramidFaces[i][order[j]];
            for (int k = 0; k < 6; ++k) {
                vertices[j * 6 + k] = objectType == 0 ? cubeVertices[index][k] : pyramidVertices[index][k];
            }
        }

        // 육면체 윗면에만 적용한다. 정지 중에도 멈춘 각도를 유지해서 그린다.
        if (objectType == 0 && i == 0) {
            upFaceRotateZ(vertices);
        }
        // 앞면만 위쪽 모서리를 중심으로 연다. 다른 면의 좌표는 바꾸지 않는다.
        if (objectType == 0 && i == 4) {
            frontFaceRotateX(vertices);
        }

        if (objectType == 0 && (i == 2 || i == 3)) {
            sideRotate(vertices);
        }

        if (objectType == 0 && i == 5) {
            backScale(vertices);
        }

        if (randomTwoSide == 0) {
            if (objectType == 0 && (i == 2 || i == 4)) {
                yTwoRotate(vertices);
            }
        }
        else if (randomTwoSide == 1) {
            if (objectType == 0 && (i == 4 || i == 3)) {
                yTwoRotate(vertices);
            }
        }
        else if (randomTwoSide == 2) {
            if (objectType == 0 && (i == 3 || i == 5)) {
                yTwoRotate(vertices);
            }
        }
        else if (randomTwoSide == 3) {
            if (objectType == 0 && (i == 5 || i == 2)) {
                yTwoRotate(vertices);
            }
        }

        // 2번: 선택한 두 면을 서로 반대 방향으로 90도 돌려 자리를 교환한다.
        if (randomChangeSide == 0) {
            if (objectType == 0 && (i == 2 || i == 4)) {
                twoChageRotate(vertices, i == 2 ? 1 : -1);
            }
        }
        else if (randomChangeSide == 1) {
            if (objectType == 0 && (i == 4 || i == 3)) {
                twoChageRotate(vertices, i == 4 ? 1 : -1);
            }
        }
        else if (randomChangeSide == 2) {
            if (objectType == 0 && (i == 3 || i == 5)) {
                twoChageRotate(vertices, i == 3 ? 1 : -1);
            }
        }
        else if (randomChangeSide == 3) {
            if (objectType == 0 && (i == 5 || i == 2)) {
                twoChageRotate(vertices, i == 5 ? 1 : -1);
            }
        }

        // 옆면 네 개는 중앙 Y축을 기준으로 공전한다.
        if (objectType == 0 && (i == 2 || i == 3 || i == 4 || i == 5)) {
            fourSideRotateY(vertices);
        }
        // 앞, 뒤 면도 이 조건에 들어가야 하므로 별도의 if로 검사한다.
        if (objectType == 0 && (i == 0 || i == 1 || i == 4 || i == 5)) {
            fourFaceRotateX(vertices);
        }

        // 사각뿔 바닥(i == 4)은 고정하고 번호에 맞는 옆면 회전 함수를 호출한다.
        if (objectType == 1 && i < 4) {
            if (pyramidMove == 6) {
                pyramidOpenAll(vertices, i);
            }
            else if (pyramidMove == 7) {
                pyramidOpenOne(vertices, i);
            }
            else if (pyramidMove == 8 && (i == 0 || i == 2)) {
                pyramidSwapFaces(vertices);
            }
            else if (pyramidMove == 9) {
                pyramidOpenFromTop(vertices, i);
            }
        }

        // 8번은 중간에 다른 옆면과 겹친다. 깊이값을 조금 당겨 색이 깜빡이지 않게 한다.
        if (objectType == 1 && pyramidMove == 8 && (i == 0 || i == 2)) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -1.0f);
        }
        drawVertices(vertices, count, GL_TRIANGLES);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void moveShape()
{
    xRotation += xDirection * 0.5f;
    yRotation += yDirection * 0.5f;
    if (xRotation >= 360.0f) xRotation -= 360.0f;
    if (xRotation <= -360.0f) xRotation += 360.0f;
    if (yRotation >= 360.0f) yRotation -= 360.0f;
    if (yRotation <= -360.0f) yRotation += 360.0f;

    // 각도 증가는 면을 그리는 반복문이 아니라 매 프레임 한 번만 한다.
    if (objectType == 0) {
        zUpsideRotation += zUpsideDirection * 0.5f;
        if (zUpsideRotation >= 360.0f) {
            zUpsideRotation -= 360.0f;
        }

        xFrontRotation += xFrontDirection * 0.5f;
        if (xFrontRotation >= 90.0f) {
            xFrontRotation = 90.0f;
            xFrontDirection = 0;
        }
        else if (xFrontRotation <= 0.0f) {
            xFrontRotation = 0.0f;
            xFrontDirection = 0;
        }

        sideRotation += sideDirection * 0.5f;
        if (sideRotation >= 360.0f) {
            sideRotation -= 360.0f;
        }

        backSize += backDirection * 0.01f;
        if (backSize >= 1.0f) {
            backSize = 1.0f;
            backDirection = 0;
        }
        else if (backSize <= 0.0f) {
            backSize = 0.0f;
            backDirection = 0;
        }

        twoChangeRotation += twoChangeDirection * 0.5f;
        if (twoChangeRotation >= 90.0f) {
            twoChangeRotation = 90.0f; // 0도로 되돌리지 않고 교환된 위치를 유지한다.
            twoChangeDirection = 0;
        }

        yFourSideRotation += yFourSideDirection * 0.5f;
        if (yFourSideRotation >= 360.0f) {
            yFourSideRotation -= 360.0f;
        }

        xFourFaceRotation += xFourFaceDirection * 0.5f;
        if (xFourFaceRotation >= 360.0f) {
            xFourFaceRotation -= 360.0f;
        }

        twoSideRotation += twoSideDirection * 0.5f;
        if (twoSideRotation >= 360.0f) {
            twoSideRotation = 0.0f;
            twoSideDirection = 0;
        }
    }
    else {
        if (pyramidMove == 6) {
            pyramidAllRotation += pyramidAllDirection * 0.5f;
            if (pyramidAllRotation >= 230.0338f) {
                pyramidAllRotation = 230.0338f;
                pyramidAllDirection = 0;
            }
            else if (pyramidAllRotation <= 0.0f) {
                pyramidAllRotation = 0.0f;
                pyramidAllDirection = 0;
            }
        }
        else if (pyramidMove == 7 && pyramidOneDirection != 0) {
            // 현재 면의 각도만 바꾸고 끝에 도착하면 다음 면으로 넘어간다.
            pyramidOneRotation[pyramidFace] += pyramidOneDirection * 0.5f;
            if (pyramidOneDirection == 1 && pyramidOneRotation[pyramidFace] >= 115.0169f) {
                pyramidOneRotation[pyramidFace] = 115.0169f;
                if (pyramidFace < 3) ++pyramidFace;
                else pyramidOneDirection = 0;
            }
            else if (pyramidOneDirection == -1 && pyramidOneRotation[pyramidFace] <= 0.0f) {
                pyramidOneRotation[pyramidFace] = 0.0f;
                if (pyramidFace > 0) --pyramidFace;
                else pyramidOneDirection = 0;
            }
        }
        else if (pyramidMove == 8) {
            pyramidSwapRotation += pyramidSwapDirection * 0.5f;
            if (pyramidSwapRotation >= 180.0f) {
                pyramidSwapRotation = 180.0f;
                pyramidSwapDirection = 0;
            }
            else if (pyramidSwapRotation <= 0.0f) {
                pyramidSwapRotation = 0.0f;
                pyramidSwapDirection = 0;
            }
        }
        else if (pyramidMove == 9) {
            pyramidTopRotation += pyramidTopDirection * 0.5f;
            if (pyramidTopRotation >= 45.0f) {
                pyramidTopRotation = 45.0f;
                pyramidTopDirection = 0;
            }
            else if (pyramidTopRotation <= 0.0f) {
                pyramidTopRotation = 0.0f;
                pyramidTopDirection = 0;
            }
        }
    }
}

void changePyramidMove(int mode)
{
    if (pyramidMove != mode) {
        // 다른 종류를 누르면 닫힌 사각뿔에서 새 애니메이션을 시작한다.
        pyramidMove = mode;
        pyramidAllRotation = 0.0f;
        pyramidAllDirection = 0;
        for (int i = 0; i < 4; ++i) pyramidOneRotation[i] = 0.0f;
        pyramidOneDirection = 0;
        pyramidFace = 0;
        pyramidSwapRotation = 0.0f;
        pyramidSwapDirection = 0;
        pyramidTopRotation = 0.0f;
        pyramidTopDirection = 0;
    }

    if (mode == 6) {
        if (pyramidAllDirection != 0) pyramidAllDirection = -pyramidAllDirection;
        else pyramidAllDirection = pyramidAllRotation == 0.0f ? 1 : -1;
    }
    else if (mode == 7) {
        if (pyramidOneDirection != 0) pyramidOneDirection = -pyramidOneDirection;
        else pyramidOneDirection = pyramidOneRotation[0] == 0.0f ? 1 : -1;
    }
    else if (mode == 8) {
        if (pyramidSwapDirection != 0) pyramidSwapDirection = -pyramidSwapDirection;
        else pyramidSwapDirection = pyramidSwapRotation == 0.0f ? 1 : -1;
    }
    else if (mode == 9) {
        if (pyramidTopDirection != 0) pyramidTopDirection = -pyramidTopDirection;
        else pyramidTopDirection = pyramidTopRotation == 0.0f ? 1 : -1;
    }
}

void resetMove()
{
    xPos = 0.0f;
    yPos = 0.0f;
    xRotation = 10.0f;
    yRotation = 10.0f;
    xDirection = 0;
    yDirection = 0;
    zUpsideRotation = 0.0f;
    zUpsideDirection = 0;
    xFrontRotation = 0.0f;
    xFrontDirection = 0;
    sideRotation = 0.0f;
    sideDirection = 0;
    backSize = 1.0f;
    backDirection = 0;
    randomTwoSide = 4; // 0~3은 면 조합, 4는 아직 선택하지 않은 상태
    twoSideRotation = 0.0f;
    twoSideDirection = 0;
    randomChangeSide = 4;
    twoChangeRotation = 0.0f;
    twoChangeDirection = 0;
    yFourSideRotation = 0.0f;
    yFourSideDirection = 0;
    xFourFaceRotation = 0.0f;
    xFourFaceDirection = 0;
    pyramidMove = 0;
    pyramidAllRotation = 0.0f;
    pyramidAllDirection = 0;
    for (int i = 0; i < 4; ++i) pyramidOneRotation[i] = 0.0f;
    pyramidOneDirection = 0;
    pyramidFace = 0;
    pyramidSwapRotation = 0.0f;
    pyramidSwapDirection = 0;
    pyramidTopRotation = 0.0f;
    pyramidTopDirection = 0;
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_P) {
        objectType = objectType == 0 ? 1 : 0;
    }
    else if (key == GLFW_KEY_H) {
        depthTest = !depthTest;
    }
    else if (key == GLFW_KEY_X) {
        xDirection = xDirection == 0 ? 1 : 0;
    }
    else if (key == GLFW_KEY_Y) {
        yDirection = yDirection == 0 ? 1 : 0;
    }
    else if (key == GLFW_KEY_R) {
        resetMove();
    }
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_T && objectType == 0) {
        zUpsideDirection = zUpsideDirection == 0 ? 1 : 0;
    }
    else if (key == GLFW_KEY_F && objectType == 0) {
        // 움직이는 중에 다시 누르면 반대로 움직인다.
        if (xFrontDirection != 0) xFrontDirection = -xFrontDirection;
        else xFrontDirection = xFrontRotation == 0.0f ? 1 : -1;
    }
    else if (key == GLFW_KEY_S && objectType == 0) {
        sideDirection = sideDirection == 0 ? 1 : 0;
    }
    else if (key == GLFW_KEY_B && objectType == 0) {
        if (backDirection != 0) backDirection = -backDirection;
        // 멈췄을 때는 크기를 확인한다. 원래 크기면 줄이고, 사라졌으면 다시 키운다.
        else backDirection = backSize >= 1.0f ? -1 : 1;
    }
    else if (key == GLFW_KEY_1 && objectType == 0 && twoSideDirection == 0) {
        randomTwoSide = rand() % 4;//0:(2, 4), 1:(4, 3), 2:(3, 5), 3: (5, 2)    
        twoSideRotation = 0.0f;
        twoSideDirection = 1;
    }
    else if (key == GLFW_KEY_2 && objectType == 0 && twoChangeDirection == 0) {
        randomChangeSide = rand() % 4;//0:(2, 4), 1:(4, 3), 2:(3, 5), 3: (5, 2)    
        twoChangeRotation = 0.0f;
        twoChangeDirection = 1;
    }
    else if (key == GLFW_KEY_3 && objectType == 0) {
        yFourSideDirection = yFourSideDirection == 0 ? 1 : 0;
    }
    else if ((key == GLFW_KEY_4 || key == GLFW_KEY_KP_4) && objectType == 0) {
        yFourSideDirection = yFourSideDirection == 0 ? 1 : 0;
    }
    else if ((key == GLFW_KEY_5 || key == GLFW_KEY_KP_5) && objectType == 0) {
        xFourFaceDirection = xFourFaceDirection == 0 ? 1 : 0;
    }
    else if (key == GLFW_KEY_6 && objectType == 1) {
        changePyramidMove(6);
    }
    else if (key == GLFW_KEY_7 && objectType == 1) {
        changePyramidMove(7);
    }
    else if (key == GLFW_KEY_8 && objectType == 1) {
        changePyramidMove(8);
    }
    else if (key == GLFW_KEY_9 && objectType == 1) {
        changePyramidMove(9);
    }
}

void drawScene()
{
    if (depthTest) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 viewProjection = getViewProjection();
    drawAxes(viewProjection);
    drawShape(viewProjection);
}

int main()
{
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "GLFW initialization failed.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 창을 만들 때 깊이값을 기록할 24비트 깊이 버퍼를 요청한다.
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-15", nullptr, nullptr);
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
        moveShape();
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
