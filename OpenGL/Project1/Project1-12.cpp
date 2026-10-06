#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

using namespace std;


struct Shape
{
    GLclampf Red{};
    GLclampf Green{};
    GLclampf Blue{};

    int vertexCount{};
    GLfloat xPos[4]{};
    GLfloat yPos[4]{};
    int type{};

    int move{};
    GLfloat speed{};
    bool enter{ false };
};


vector<Shape> shapes;
int shapesCount = -1;
Shape rec[3];


GLuint shaderProgram{};
GLuint VAO{};
GLuint VBO{};
GLint colorLocation{};


// Vertex Shader
const char* vertexSource = R"(
#version 330 core

layout(location = 0) in vec2 position;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
}
)";


// Fragment Shader
const char* fragmentSource = R"(
#version 330 core

uniform vec3 drawColor;

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(drawColor, 1.0);
}
)";


GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        char log[512]{};

        glGetShaderInfoLog(
            shader,
            sizeof(log),
            nullptr,
            log
        );

        cout << "Shader Compile Error\n";
        cout << log << '\n';

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}


GLuint createShader()
{
    GLuint vertexShader =
        compileShader(GL_VERTEX_SHADER, vertexSource);

    GLuint fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentSource);


    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);


    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    return program;
}


void initBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);


    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);


    // 사각형이면 삼각형 2개로 만들기 때문에
    // 최대 정점 6개
    // 정점 하나당 x,y → float 2개
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(GLfloat) * 12,
        nullptr,
        GL_DYNAMIC_DRAW
    );


    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GLfloat) * 2,
        nullptr
    );

    glEnableVertexAttribArray(0);
}

void setColor(GLclampf* Red, GLclampf* Green, GLclampf* Blue)
{
    *Red = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Green = static_cast<GLclampf>(std::rand()) / RAND_MAX;
    *Blue = static_cast<GLclampf>(std::rand()) / RAND_MAX;
}

bool overlapRectangle(const Shape& a, const Shape& b)
{
    return a.xPos[0] <= b.xPos[2] &&
        a.xPos[2] >= b.xPos[0] &&
        a.yPos[1] <= b.yPos[0] &&
        a.yPos[0] >= b.yPos[1];
}

void drawShape(Shape& shape)
{
    GLfloat vertices[12]{};

    int count = shape.vertexCount;

    if (shape.vertexCount == 3)
    {
        vertices[0] = shape.xPos[0];
        vertices[1] = shape.yPos[0];

        vertices[2] = shape.xPos[1];
        vertices[3] = shape.yPos[1];

        vertices[4] = shape.xPos[2];
        vertices[5] = shape.yPos[2];
    }
    else if (shape.type == 0 && shape.vertexCount == 4)
    {
        int order[6] ={0, 1, 2, 1, 3, 2};

        count = 6;

        for (int i = 0; i < 6; ++i) {
            int index = order[i];

            vertices[i * 2] =  shape.xPos[index];

            vertices[i * 2 + 1] = shape.yPos[index];
        }
    }
    else if (shape.type == 1 && shape.vertexCount == 4) {
        int order[4] = { 0, 1, 3, 2 };
        count = 4;

        for (int i = 0; i < count; ++i) {
            int index = order[i];

            vertices[i * 2] = shape.xPos[index];
            vertices[i * 2 + 1] = shape.yPos[index];
        }
    }


    glUniform3f(colorLocation, shape.Red, shape.Green, shape.Blue);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * count * 2, vertices);
    if (shape.type == 0) {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }
    else {
        glDrawArrays(GL_LINE_LOOP, 0, count);
    }


}



void drawAllShape()
{
    for (int i = 0; i < shapes.size(); ++i) {
        drawShape(shapes[i]);
    }

    for (int i = 0; i < 3; ++i) {
        drawShape(rec[i]);
    }
}


void addRectangle(int type)
{
    Shape shape{};

    setColor(&shape.Red, &shape.Green, &shape.Blue);

    shape.vertexCount = 4;

    if (type == 0) {
        shape.xPos[0] = -0.3f;
        shape.yPos[0] = 0.0f;

        shape.xPos[1] = -0.3f;
        shape.yPos[1] = -0.4f;

        shape.xPos[2] = 0.1f;
        shape.yPos[2] = 0.0f;

        shape.xPos[3] = 0.1f;
        shape.yPos[3] = -0.4f;
    }
    else {
        shape.xPos[0] = -0.8f;
        shape.yPos[0] = -0.4f;

        shape.xPos[1] = -0.8f;
        shape.yPos[1] = -0.8f;

        shape.xPos[2] = -0.4f;
        shape.yPos[2] = -0.4f;

        shape.xPos[3] = -0.4f;
        shape.yPos[3] = -0.8f;
    }

    shape.move = rand() % 2;
    shape.speed = (rand() % 9 + 1) * 0.001f;
    shape.enter = false;
    shape.type = 0;

    shapes.push_back(shape);
    shapesCount++;
}

void setStart()
{
    rec[0].xPos[0] = -0.3f;
    rec[0].yPos[0] = 1.0f;

    rec[0].xPos[1] = -0.3f;
    rec[0].yPos[1] = -1.0f;

    rec[0].xPos[2] = 0.1f;
    rec[0].yPos[2] = 1.0f;

    rec[0].xPos[3] = 0.1f;
    rec[0].yPos[3] = -1.0f;

    rec[1].xPos[0] = -0.8f;
    rec[1].yPos[0] = 1.0f;

    rec[1].xPos[1] = -0.8f;
    rec[1].yPos[1] = -1.0f;

    rec[1].xPos[2] = -0.4f;
    rec[1].yPos[2] = 1.0f;

    rec[1].xPos[3] = -0.4f;
    rec[1].yPos[3] = -1.0f;

    rec[2].xPos[0] = -0.8f;
    rec[2].yPos[0] = 0.05f;

    rec[2].xPos[1] = -0.8f;
    rec[2].yPos[1] = -0.05f;

    rec[2].xPos[2] = 0.1f;
    rec[2].yPos[2] = 0.05f;

    rec[2].xPos[3] = 0.1f;
    rec[2].yPos[3] = -0.05f;

    for (int i = 0; i < 3; ++i) {
        rec[i].move = 0;
        rec[i].speed = 0;
        rec[i].enter = false;
        setColor(&rec[i].Red, &rec[i].Green, &rec[i].Blue);
        rec[i].vertexCount = 4;
        rec[i].type = 1;
    }
    

    addRectangle(0);
    addRectangle(1);
}

void moveRectangle() 
{
    for (int i = 0; i < shapes.size(); ++i) {
        if (!shapes[i].enter) {
            if (shapes[i].move == 0) {
                for (int j = 0; j < 4; ++j) {
                    shapes[i].yPos[j] += shapes[i].speed;
                }
                if (shapes[i].yPos[0] > 1.0f) {
                    shapes[i].move = 1;
                }
            }
            else if(shapes[i].move == 1){
                for (int j = 0; j < 4; ++j) {
                    shapes[i].yPos[j] -= shapes[i].speed;
                }
                if (shapes[i].yPos[1] < -1.0f) {
                    shapes[i].move = 0;
                }
            }
            else if (shapes[i].move == 2) {
                for (int j = 0; j < 4; ++j) {
                    shapes[i].xPos[j] += shapes[i].speed;
                }

                if (shapes[i].xPos[2] >= 1.0f) {
                    // 오른쪽 벽에 정확히 맞춘다.
                    GLfloat dx = 1.0f - shapes[i].xPos[2];
                    for (int j = 0; j < 4; ++j) {
                        shapes[i].xPos[j] += dx;
                    }

                    if (i == 0) {
                        shapes[i].move = 3;
                    }
                    else if (shapes[i - 1].enter) {
                        // 앞 네모가 멈춘 다음, 그 윗면을 기준으로 방향을 정한다.
                        if (shapes[i].yPos[1] >= shapes[i - 1].yPos[0]) {
                            shapes[i].move = 3;  // 위에 있으니 내려간다.
                        }
                        else {
                            shapes[i].move = 4;  // 아래에 있으니 올라간다.
                        }
                    }
                }
            }
            else if (shapes[i].move == 3) {
                for (int j = 0; j < 4; ++j) {
                    shapes[i].yPos[j] -= shapes[i].speed;
                }
                if (i == 0) {
                    if (shapes[i].yPos[1] < -1.0f) {
                        shapes[i].enter = true;
                    }
                }
                else {
                    if (overlapRectangle(shapes[i - 1], shapes[i])) {
                        shapes[i].enter = true;
                    }
                }
            }
            else if (shapes[i].move == 4) {
                for (int j = 0; j < 4; ++j) {
                    shapes[i].yPos[j] += shapes[i].speed;
                }

                if (shapes[i].yPos[1] >= shapes[i - 1].yPos[0]) {
                    GLfloat dy = shapes[i - 1].yPos[0] - shapes[i].yPos[1];

                    for (int j = 0; j < 4; ++j) {
                        shapes[i].yPos[j] += dy;
                    }

                    shapes[i].enter = true;
                }
            }
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) {
        return;
    } 

    if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_ENTER) {
        if (overlapRectangle(shapes[shapesCount], rec[2]) && overlapRectangle(shapes[shapesCount-1], rec[2])) {
            shapes[shapesCount].move = 2;
            shapes[shapesCount].speed = 0.01f;
            shapes[shapesCount-1].move = 2;
            shapes[shapesCount-1].speed = 0.01f;
            addRectangle(0);
            addRectangle(1);
        }
    }
    else if (key == GLFW_KEY_R) {
        shapes.clear();
        shapesCount = -1;
        setStart();
    }
}

int main()
{
    if (glfwInit() == GLFW_FALSE) {
        cout << "GLFW Init Failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Shape", nullptr, nullptr);

    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        cout << "GLEW Init Failed\n";

        glfwDestroyWindow(window);
        glfwTerminate();

        return -1;
    }

    shaderProgram = createShader();

    colorLocation = glGetUniformLocation(shaderProgram, "drawColor");

    initBuffer();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);


    setStart();
    glfwSetKeyCallback(window, keyCallback);
    


    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        moveRectangle();
        drawAllShape();

        glfwSwapBuffers(window);
    }


    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);


    glfwDestroyWindow(window);
    glfwTerminate();


    return 0;
}