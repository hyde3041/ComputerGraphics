#define OBJ_DIRECTORY "C:/study/2-2/ComputerGraphics/OpenGL/Project1/models"
#define main project21Main
#include "../../OpenGL/Project1/Project1-21.cpp"
#undef main
#include <array>

int main()
{
    if (!loadOBJ("cube.obj", cubeVertices)) return 1;
    if (!glfwInit()) return 2;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "render-check", nullptr, nullptr);
    if (!window) return 3;
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return 4;
    shaderProgram = createShader();
    if (!shaderProgram) return 5;
    glUseProgram(shaderProgram);
    transformLocation = glGetUniformLocation(shaderProgram, "transform");
    colorLocation = glGetUniformLocation(shaderProgram, "drawColor");
    initBuffer();
    setScene();
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(.07f, .09f, .13f, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    while (glGetError() != GL_NO_ERROR) {}
    std::vector<unsigned char> pixels(windowWidth * windowHeight * 3);
    bool passed = true;
    const unsigned char colors[6][3]{
        {255,0,0},{0,255,0},{0,0,255},{255,255,0},{255,0,255},{0,255,255}
    };
    for (int mode = 0; mode < 4; ++mode) {
        cameraPosition = mode == 0 ? glm::vec3(0,0,6) : glm::vec3(4,3,6);
        glCullFace(mode == 2 ? GL_BACK : GL_FRONT);
        if (mode == 3) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawCube(getViewProjection(), glm::vec3(0), glm::vec3(1), glm::vec3(.25f,.5f,.75f));
        } else drawScene();
        glFinish();
        glReadBuffer(GL_BACK);
        glReadPixels(0,0,windowWidth,windowHeight,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
        std::array<int,6> counts{};
        int solidPixels=0;
        for (size_t i=0;i<pixels.size();i+=3)
            if (std::abs(int(pixels[i])-64)<=1 && std::abs(int(pixels[i+1])-128)<=1 && std::abs(int(pixels[i+2])-191)<=1) ++solidPixels;
        for (size_t i=0; i<pixels.size(); i+=3)
            for (int c=0;c<6;++c)
                if (pixels[i]==colors[c][0]&&pixels[i+1]==colors[c][1]&&pixels[i+2]==colors[c][2])
                    ++counts[c];
        int total=0,visible=0;
        for (int c:counts) { total+=c; visible+=c>100; }
        const GLenum error=glGetError();
        std::cout << "case " << mode << ": pixels=" << total << " distinct_faces=" << visible
                  << " gl_error=" << error << " solid_pixels=" << solidPixels << " colors=";
        for (int c:counts) std::cout << c << ',';
        std::cout << '\n';
        passed &= error==GL_NO_ERROR && (mode==3 ? solidPixels>1000 && total==0 : total>1000 && (mode==0||visible>=3));
        const char* names[]{"current","angled-inside","angled-outside","solid-color"};
        std::ofstream image(std::string("tmp/project21-render-check/")+names[mode]+".ppm",std::ios::binary);
        image << "P6\n" << windowWidth << ' ' << windowHeight << "\n255\n";
        for (int y=windowHeight-1;y>=0;--y)
            image.write(reinterpret_cast<char*>(pixels.data()+y*windowWidth*3),windowWidth*3);
    }
    glDeleteBuffers(1,&VBO);glDeleteVertexArrays(1,&VAO);glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);glfwTerminate();
    return passed?0:6;
}

