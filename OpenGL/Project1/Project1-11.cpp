// 읽는 순서: main -> resetBoard -> updatePlayer -> movePlayer -> drawScene
// 1. main에서 행/열을 입력받고, resetBoard에서 보드와 주인공을 만든다.
// 2. Space를 누르면 moveOn이 true가 되어 이동을 시작한다.
// 3. updatePlayer: 0.2초가 지났는지 확인한다.
// 4. movePlayer: 한 칸 이동하고, 장애물이 있으면 서로 모양을 교환한다.
// 5. drawScene: 현재 배열과 주인공 위치를 화면에 그린다.

// GLEW 헤더는 GLFW보다 먼저 포함한다.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cstdio>

// obstacle[row][col] 하나가 보드의 한 칸을 담당한다.
struct Obstacle {
    // -1: 빈 칸, 0: 사각형, 1: 위쪽 삼각형, 2: 아래쪽 삼각형
    int type{ -1 };
    GLclampf Red{}, Green{}, Blue{};
    // 칸 크기에 대한 반너비/반높이. 0.4이면 도형 한 변이 칸의 80%이다.
    GLfloat size{};
    // 마지막으로 충돌한 시각. -10으로 시작하면 처음에는 충돌 표시가 안 나온다.
    double hitTime{ -10.0 };
};

int boardRow{};      // 입력받을 행 개수(세로).
int boardCol{};      // 입력받을 열 개수(가로).
GLfloat cellSize{};  // 입력한 보드가 화면에 들어가도록 계산할 한 칸의 세로 길이.
GLfloat boardLeft{}; // 보드 왼쪽 끝의 x좌표. 열 개수로 계산한다.
GLfloat boardTop{};  // 보드 위쪽 끝의 y좌표. 행 개수로 계산한다.
double moveInterval = 0.2;    // 한 칸 이동한 뒤 다음 이동까지 기다릴 시간(초).
const double effectDuration = 0.45; // 충돌한 칸을 주황색으로 보여 줄 시간(초).

// 바깥 vector는 행, 안쪽 vector는 각 행의 칸들을 저장한다.
std::vector<std::vector<Obstacle>> obstacle;
int obstacleCount{};    // 실제로 만들어진 장애물 개수.
int playerRow{};        // 주인공의 행 번호. 0은 맨 위, boardRow - 1은 맨 아래.
int playerCol{};        // 주인공의 열 번호. 0은 맨 왼쪽, boardCol - 1은 맨 오른쪽.
int playerType{};       // 주인공의 현재 모양. 장애물과 같은 0, 1, 2를 사용한다.
bool moveOn{ false };   // true일 때만 이동한다. Space로 켜고 끈다.
bool finished{ false }; // 마지막 칸에 도착했는지 기록한다.
double lastMoveTime{};  // 마지막으로 한 칸 이동을 처리한 시각.

// OpenGL에서 만든 셰이더, 색상 변수, 꼭짓점 설정, 좌표 저장 공간의 번호.
GLuint shaderProgram{};
GLint colorLocation{ -1 };
GLuint vao{};
GLuint vbo{};

// GLSL 소스 문자열을 컴파일하고 셰이더 번호를 돌려준다. 실패하면 0을 반환한다.
GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        char message[1024]{};
        glGetShaderInfoLog(shader, 1024, nullptr, message);
        std::cerr << "Shader compilation failed:\n" << message << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// 꼭짓점 위치를 정하는 셰이더와 색상을 정하는 셰이더를 한 프로그램으로 연결한다.
GLuint createShader()
{
    const char* vertexSource = R"(
#version 330 core
layout (location = 0) in vec2 position;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
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

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) {
            glDeleteShader(vertexShader);
        }
        if (fragmentShader != 0) {
            glDeleteShader(fragmentShader);
        }
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char message[1024]{};
        glGetProgramInfoLog(program, 1024, nullptr, message);
        std::cerr << "Shader linking failed:\n" << message << '\n';
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

// RGB 각각에 0~1 사이의 무작위 값을 넣는다.
void setColor(GLclampf* Red, GLclampf* Green, GLclampf* Blue)
{
    *Red = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Green = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Blue = static_cast<GLclampf>(std::rand()) / RAND_MAX;
}

// min~max 사이의 실수를 무작위로 하나 뽑는다.
GLfloat randomFloat(GLfloat min, GLfloat max)
{
    return min + static_cast<GLfloat>(std::rand()) / RAND_MAX * (max - min);
}

// 처음 실행할 때와 R을 누를 때 보드 전체를 새로 만든다.
void resetBoard()
{
    // 1. 입력한 행 개수만큼 공간을 만들고, 각 행에 열 개수만큼 칸을 만든다.
    obstacle.resize(boardRow);
    for (int row = 0; row < boardRow; ++row) {
        obstacle[row].resize(boardCol);
        for (int col = 0; col < boardCol; ++col) {
            // R로 리셋할 때도 빈 칸(type=-1, hitTime=-10)으로 돌아간다.
            obstacle[row][col] = Obstacle{};
        }
    }

    // 보드 전체가 가로/세로 각각 -0.9~0.9 안에 들어가도록 칸 크기를 정한다.
    // 800x600 창에서 칸이 정사각형으로 보이도록 x 길이에 0.75를 곱한다.
    cellSize = 1.8F / boardRow;
    GLfloat sizeByColumns = 1.8F / (boardCol * 0.75F);
    if (sizeByColumns < cellSize) {
        cellSize = sizeByColumns;
    }
    boardLeft = -boardCol * cellSize * 0.75F / 2.0F;
    boardTop = boardRow * cellSize / 2.0F;

    // 2. 주인공을 왼쪽 위에 놓고, 사각형 모양과 정지 상태로 시작한다.
    playerRow = 0;
    playerCol = 0;
    playerType = 0;
    moveOn = false;
    finished = false;
    lastMoveTime = 0.0;
    // 1행 1열이면 시작 칸이 곧 마지막 칸이다.
    if (boardRow == 1 && boardCol == 1) {
        finished = true;
    }

    // 3. 전체 칸의 약 35~50%에 장애물을 만든다. 20x20이면 기존처럼 140~200개다.
    // 최대 절반만 채우므로 작은 보드에서도 시작 칸을 비워 둘 수 있다.
    int totalCells = boardRow * boardCol;
    int minObstacles = static_cast<int>(totalCells * 0.35);
    int maxObstacles = totalCells / 2;
    obstacleCount = minObstacles + std::rand() % (maxObstacles - minObstacles + 1);
    int createdCount = 0;
    while (createdCount < obstacleCount) {
        int row = std::rand() % boardRow;
        int col = std::rand() % boardCol;

        // 시작 칸에는 주인공이 있으므로 장애물을 만들지 않는다.
        if (row == 0 && col == 0) {
            continue;
        }
        // 이미 장애물이 있는 칸이면 다른 칸을 다시 뽑는다.
        if (obstacle[row][col].type != -1) {
            continue;
        }

        obstacle[row][col].type = std::rand() % 3;
        obstacle[row][col].size = randomFloat(0.20F, 0.40F);
        setColor(&obstacle[row][col].Red,
            &obstacle[row][col].Green, &obstacle[row][col].Blue);
        // 너무 어두운 색을 피하도록 RGB 범위를 0.25~0.90으로 바꾼다.
        obstacle[row][col].Red = 0.25F + obstacle[row][col].Red * 0.65F;
        obstacle[row][col].Green = 0.25F + obstacle[row][col].Green * 0.65F;
        obstacle[row][col].Blue = 0.25F + obstacle[row][col].Blue * 0.65F;
        ++createdCount;
    }
}

// 배열의 행/열 번호를 그 칸 중심의 OpenGL 좌표로 바꾼다.
void getCellPos(int row, int col, GLfloat* x, GLfloat* y)
{
    // 0.5를 더하는 이유: 칸의 왼쪽/위쪽 테두리가 아니라 중심을 구하기 위해서다.
    *x = boardLeft + (col + 0.5F) * cellSize * 0.75F;
    // 행 번호가 커질수록 아래로 간다. OpenGL의 y는 위쪽이 +이므로 빼 준다.
    *y = boardTop - (row + 0.5F) * cellSize;
}

// 이 함수를 한 번 부르면 정확히 한 칸 이동한다. 이동 간격은 updatePlayer가 관리한다.
void movePlayer(double currentTime)
{
    if (finished) {
        return;
    }

    // 1. 다음 칸으로 이동한다.
    // 0, 2, 4...행: 오른쪽으로 가다가 오른쪽 끝에서 한 칸 내려간다.
    if (playerRow % 2 == 0) {
        if (playerCol < boardCol - 1) {
            ++playerCol;
        }
        else {
            ++playerRow;
        }
    }
    // 1, 3, 5...행: 왼쪽으로 가다가 왼쪽 끝에서 한 칸 내려간다.
    else {
        if (playerCol > 0) {
            --playerCol;
        }
        else {
            ++playerRow;
        }
    }
    // 행이 바뀌면 다음 호출에서는 반대쪽 조건에 들어가므로 지그재그가 된다.

    // 2. 도착한 칸에 장애물이 있는지 확인한다. -1이면 빈 칸이다.
    if (obstacle[playerRow][playerCol].type != -1) {
        // 주인공 모양을 따로 저장한 뒤 서로 바꾼다. 색상과 크기는 바꾸지 않는다.
        int oldPlayerType = playerType;
        playerType = obstacle[playerRow][playerCol].type;
        obstacle[playerRow][playerCol].type = oldPlayerType;
        // 충돌한 시각을 저장한다. 이후 0.45초 동안 이 칸을 주황색으로 그린다.
        obstacle[playerRow][playerCol].hitTime = currentTime;
    }

    // 3. 행이 짝수 개면 왼쪽 아래, 홀수 개면 오른쪽 아래가 마지막 칸이다.
    int lastCol = 0;
    if (boardRow % 2 == 1) {
        lastCol = boardCol - 1;
    }
    if (playerRow == boardRow - 1 && playerCol == lastCol) {
        finished = true;
        moveOn = false;
    }
}

// 매 프레임 호출되지만, 0.2초가 지났을 때만 movePlayer를 호출한다.
// 실제로 이동했으면 true를 반환해서 창 제목의 행/열 표시도 갱신하게 한다.
bool updatePlayer(double currentTime)
{
    if (!moveOn || finished) {
        return false;
    }

    bool moved = false;
    // 현재 시각 - 마지막 이동 시각 = 다음 이동을 기다린 시간.
    // while인 이유: 처리가 늦어져 0.65초가 지났다면 한 칸씩 3번 처리하기 위해서다.
    // 한 번에 여러 칸을 건너뛰지 않으므로 중간 장애물과의 충돌도 모두 처리된다.
    while (currentTime - lastMoveTime >= moveInterval && !finished) {
        lastMoveTime += moveInterval;
        movePlayer(lastMoveTime);
        moved = true;
    }
    return moved;
}

// 이 칸의 충돌 표시를 아직 보여 줘야 하는지 판단한다.
bool collisionVisible(int row, int col, double currentTime)
{
    double timeAfterHit = currentTime - obstacle[row][col].hitTime;
    if (timeAfterHit >= 0.0 && timeAfterHit < effectDuration) {
        return true;
    }
    return false;
}

// 창 제목에 이동 상태와 현재 행/열을 보여 준다.
void updateTitle(GLFWwindow* window)
{
    const char* state;
    if (finished) {
        state = "Finished";
    }
    else if (moveOn) {
        state = "Moving";
    }
    else {
        state = "Paused";
    }

    // 배열 번호는 0부터지만, 화면에서는 알아보기 쉽게 1부터 표시한다.
    char title[160]{};
    std::snprintf(title, sizeof(title),
        "Project1-11 | %s | Row %d Col %d | Space: Start/Pause | R: Reset | Q: Quit",
        state, playerRow + 1, playerCol + 1);
    glfwSetWindowTitle(window, title);
}

// 키를 새로 눌렀을 때만 처리한다. 누르고 있는 동안 반복되는 입력은 무시한다.
void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) {
        return;
    }
    if (key == GLFW_KEY_SPACE && !finished) {
        if (moveOn) {
            moveOn = false;
        }
        else {
            moveOn = true;
        }
        // 정지해 있던 시간을 이동 시간으로 세지 않도록 기준 시각을 다시 잡는다.
        lastMoveTime = glfwGetTime();
        updateTitle(window);
    }
    else if (key == GLFW_KEY_R) {
        resetBoard();
        updateTitle(window);
    }
    else if (key == GLFW_KEY_EQUAL) {
        moveInterval /= 5;
        lastMoveTime = glfwGetTime();
    }
    else if (key == GLFW_KEY_MINUS) {
        moveInterval *= 5;
        lastMoveTime = glfwGetTime();
    }
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// 좌표 배열을 GPU에 보내고, primitive에 지정한 방식(삼각형/선)으로 그린다.
void drawVertices(const GLfloat* vertices, int count, GLenum primitive, GLclampf Red, GLclampf Green, GLclampf Blue)
{
    // 사용할 셰이더를 고르고 이번 도형의 색을 전달한다.
    glUseProgram(shaderProgram);
    glUniform3f(colorLocation, Red, Green, Blue);
    // VAO는 좌표 읽는 방법, VBO는 실제 좌표를 담는 공간이다.
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // 꼭짓점 하나에 x, y 두 값이 있으므로 count * 2개를 전달한다.
    glBufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(GLfloat), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(primitive, 0, count);
}

// (x, y)를 중심으로 도형을 그린다. filled가 true면 면, false면 테두리만 그린다.
void drawShape(int type, GLfloat x, GLfloat y, GLfloat size, GLclampf Red, GLclampf Green, GLclampf Blue, bool filled = true)
{
    GLfloat halfWidth = cellSize * size * 0.75F;
    GLfloat halfHeight = cellSize * size;
    GLfloat left = x - halfWidth;
    GLfloat right = x + halfWidth;
    GLfloat bottom = y - halfHeight;
    GLfloat top = y + halfHeight;

    if (type == 1 || type == 2) {
        // 위쪽 삼각형: 왼쪽 아래 -> 오른쪽 아래 -> 위쪽 꼭짓점.
        GLfloat vertices[] = { left, bottom, right, bottom, x, top };
        if (type == 2) {
            // 아래쪽 삼각형은 세 꼭짓점의 y만 반대로 놓는다.
            vertices[1] = top;
            vertices[3] = top;
            vertices[5] = bottom;
        }
        if (filled) {
            drawVertices(vertices, 3, GL_TRIANGLES, Red, Green, Blue);
        }
        else {
            drawVertices(vertices, 3, GL_LINE_LOOP, Red, Green, Blue);
        }
    }
    else {
        if (filled) {
            // 사각형 면은 삼각형 두 개로 나눠 그린다. 따라서 꼭짓점은 총 6개다.
            const GLfloat vertices[] = {
                left, bottom, right, bottom, right, top,
                left, bottom, right, top, left, top
            };
            drawVertices(vertices, 6, GL_TRIANGLES, Red, Green, Blue);
        }
        else {
            // 테두리는 네 꼭짓점을 차례로 연결하면 된다.
            const GLfloat vertices[] = {
                left, bottom, right, bottom, right, top, left, top
            };
            drawVertices(vertices, 4, GL_LINE_LOOP, Red, Green, Blue);
        }
    }
}

// 한 프레임의 화면을 그린다. 나중에 그린 것이 앞에 보이므로 주인공을 마지막에 그린다.
void drawScene(double currentTime)
{
    // 1. 이전 화면을 지우고 보드의 어두운 배경을 그린다.
    glClear(GL_COLOR_BUFFER_BIT);
    // 행과 열이 다르면 보드가 직사각형이므로 실제 가로/세로 끝점으로 그린다.
    const GLfloat background[] = {
        boardLeft, -boardTop, -boardLeft, -boardTop, -boardLeft, boardTop,
        boardLeft, -boardTop, -boardLeft, boardTop, boardLeft, boardTop
    };
    drawVertices(background, 6, GL_TRIANGLES, 0.08F, 0.10F, 0.13F);

    // 2. 충돌한 칸은 주황색, 현재 주인공이 있는 칸은 파란색으로 표시한다.
    for (int row = 0; row < boardRow; ++row) {
        for (int col = 0; col < boardCol; ++col) {
            GLfloat x, y;
            getCellPos(row, col, &x, &y);
            if (collisionVisible(row, col, currentTime)) {
                double timeAfterHit = currentTime - obstacle[row][col].hitTime;
                // 충돌 직후에는 1, 0.45초가 지나면 0에 가까워져 점점 어두워진다.
                GLfloat strength = 1.0F - static_cast<GLfloat>(timeAfterHit / effectDuration);
                drawShape(0, x, y, 0.48F, 0.15F + strength * 0.7F, 0.1F + strength * 0.35F, 0.08F);
            }
            else if (row == playerRow && col == playerCol) {
                drawShape(0, x, y, 0.48F, 0.10F, 0.30F, 0.45F);
            }
        }
    }

    // 3. 세로선은 열 개수+1개, 가로선은 행 개수+1개를 그린다.
    for (int col = 0; col <= boardCol; ++col) {
        GLfloat x = boardLeft + col * cellSize * 0.75F;
        const GLfloat verticalLine[] = { x, -boardTop, x, boardTop };
        drawVertices(verticalLine, 2, GL_LINES, 0.30F, 0.34F, 0.40F);
    }
    for (int row = 0; row <= boardRow; ++row) {
        GLfloat y = boardTop - row * cellSize;
        const GLfloat horizontalLine[] = { boardLeft, y, -boardLeft, y };
        drawVertices(horizontalLine, 2, GL_LINES, 0.30F, 0.34F, 0.40F);
    }

    // 4. 빈 칸(type=-1)은 건너뛰고 장애물만 그린다.
    for (int row = 0; row < boardRow; ++row) {
        for (int col = 0; col < boardCol; ++col) {
            if (obstacle[row][col].type == -1) {
                continue;
            }
            GLfloat x, y;
            getCellPos(row, col, &x, &y);
            drawShape(obstacle[row][col].type, x, y, obstacle[row][col].size,
                obstacle[row][col].Red, obstacle[row][col].Green, obstacle[row][col].Blue);
        }
    }

    // 5. 주인공은 흰색 면을 먼저 그리고, 같은 위치에 하늘색 테두리를 덧그린다.
    GLfloat playerX, playerY;
    getCellPos(playerRow, playerCol, &playerX, &playerY);
    drawShape(playerType, playerX, playerY, 0.36F, 1.0F, 1.0F, 1.0F);
    drawShape(playerType, playerX, playerY, 0.36F, 0.2F, 0.9F, 1.0F, false);
}

// 창 크기가 달라지면 OpenGL이 그림을 표시할 영역도 바꾼다.
void framebufferSize(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    // 창을 만들기 전에 콘솔에서 행(세로), 열(가로)을 차례로 입력받는다.
    // 예: 5 8을 입력하면 세로 5줄, 가로 8칸짜리 보드가 된다.
    std::cout << "Rows Columns (e.g. 5 8): ";
    if (!(std::cin >> boardRow >> boardCol) || boardRow <= 0 || boardCol <= 0) {
        std::cerr << "Rows and columns must be positive integers.\n";
        return 1;
    }
    // 전체 칸 수를 int로 계산할 수 있는 범위인지 확인한다.
    if (boardRow > std::numeric_limits<int>::max() / boardCol) {
        std::cerr << "Too many board cells.\n";
        return 1;
    }

    // 실행할 때마다 장애물 배치가 달라지도록 난수의 시작값을 정한다.
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "GLFW initialization failed.\n";
        return -1;
    }

    // 1. OpenGL 3.3을 사용할 800x600 창을 만든다.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Project1-11", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Window creation failed.\n";
        glfwTerminate();
        return -1;
    }

    // 이 창에서 OpenGL을 사용하도록 연결하고 함수들을 준비한다.
    glfwMakeContextCurrent(window);
    glfwSetWindowAspectRatio(window, 4, 3);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 2. 셰이더와 좌표를 전달할 VAO/VBO를 준비한다.
    shaderProgram = createShader();
    if (shaderProgram == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    colorLocation = glGetUniformLocation(shaderProgram, "drawColor");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // position 입력에는 GLfloat 두 개(x, y)를 한 꼭짓점으로 묶어 전달한다.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    // 3. 화면 영역, 창 크기 변경 함수, 키 입력 함수를 설정한다.
    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSize);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);
    glClearColor(0.025F, 0.03F, 0.04F, 1.0F);
    // 4. 장애물과 주인공을 초기화한 뒤 반복을 시작한다.
    resetBoard();
    updateTitle(window);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        // 키 입력이 있으면 위에서 등록한 keyCallback이 실행된다.
        glfwPollEvents();

        // glfwGetTime은 경과 시간을 초 단위로 준다. 현재 시각으로 이동 간격을 검사한다.
        double currentTime = glfwGetTime();
        bool moved = updatePlayer(currentTime);
        if (moved) {
            updateTitle(window);
        }

        // 지금 상태를 새로 그린 뒤, 완성된 화면을 창에 보여 준다.
        drawScene(currentTime);
        glfwSwapBuffers(window);
    }

    // 창이 닫히면 사용한 OpenGL 자원과 창을 정리한다.
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
