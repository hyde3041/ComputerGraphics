#define OBJ_DIRECTORY "C:/study/2-2/ComputerGraphics/OpenGL/Project1/models"
#define main project22Main
#include "../../OpenGL/Project1/Project1-22.cpp"
#undef main
#include <cassert>
#include <array>

struct Stats { int count{}; double x{},y{}; };
std::array<Stats,3> render(const char* name) {
    const float x=tankX,z=tankZ,angle=centerRotation,height=lowerHeight;
    drawViews();glFinish();
    assert(glGetError()==GL_NO_ERROR);
    assert(glIsEnabled(GL_SCISSOR_TEST)==GL_FALSE);
    assert(tankX==x && tankZ==z && centerRotation==angle && lowerHeight==height);
    glReadBuffer(GL_BACK);
    std::vector<unsigned char> pixels(static_cast<size_t>(windowWidth)*windowHeight*3);
    glReadPixels(0,0,windowWidth,windowHeight,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
    int left=windowWidth*2/3,right=windowWidth-left-4,bottom=(windowHeight-4)/2,top=bottom+4;
    const int bounds[3][4]{{0,0,left,windowHeight},{left+4,top,right,windowHeight-top},{left+4,0,right,bottom}};
    std::array<Stats,3> stats;
    for(int v=0;v<3;++v) {
        for(int y=bounds[v][1];y<bounds[v][1]+bounds[v][3];++y)
            for(int x=bounds[v][0];x<bounds[v][0]+bounds[v][2];++x) {
                size_t k=(static_cast<size_t>(y)*windowWidth+x)*3;
                if(pixels[k]>80||pixels[k+1]>80||pixels[k+2]>80) {
                    ++stats[v].count;stats[v].x+=x;stats[v].y+=y;
                }
            }
        assert(stats[v].count>100);
        stats[v].x/=stats[v].count;stats[v].y/=stats[v].count;
        std::cout<<name<<" view="<<v+1<<" tank_pixels="<<stats[v].count<<" center="<<stats[v].x<<','<<stats[v].y<<'\n';
    }
    size_t edge=(static_cast<size_t>(windowHeight/2)*windowWidth+left+1)*3;
    assert(std::abs(int(pixels[edge])-77)<=1 && std::abs(int(pixels[edge+1])-84)<=1 && std::abs(int(pixels[edge+2])-97)<=1);
    std::ofstream image(std::string("tmp/project22-render-check/")+name+".ppm",std::ios::binary);
    image<<"P6\n"<<windowWidth<<' '<<windowHeight<<"\n255\n";
    for(int y=windowHeight-1;y>=0;--y)image.write(reinterpret_cast<char*>(pixels.data()+static_cast<size_t>(y)*windowWidth*3),windowWidth*3);
    return stats;
}

int main() {
    if(!loadOBJ("cube.obj",cubeVertices)||!glfwInit())return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
    GLFWwindow* window=glfwCreateWindow(1200,800,"render-check",nullptr,nullptr);if(!window)return 2;
    glfwMakeContextCurrent(window);glewExperimental=GL_TRUE;if(glewInit()!=GLEW_OK)return 3;
    shaderProgram=createShader();if(!shaderProgram)return 4;glUseProgram(shaderProgram);
    transformLocation=glGetUniformLocation(shaderProgram,"transform");colorLocation=glGetUniformLocation(shaderProgram,"drawColor");
    initBuffer();resetScene();glEnable(GL_DEPTH_TEST);glPixelStorei(GL_PACK_ALIGNMENT,1);
    glfwGetFramebufferSize(window,&windowWidth,&windowHeight);
    while(glGetError()!=GL_NO_ERROR){}
    for(int type=1;type<=3;++type) {
        glm::mat4 matrix=getViewProjection(type,396,398);
        for(int c=0;c<4;++c)for(int r=0;r<4;++r)assert(std::isfinite(matrix[c][r]));
    }
    auto initial=render("initial");
    for(int i=0;i<8;++i) {
        keyCallback(window,GLFW_KEY_RIGHT,0,GLFW_PRESS,0);
        keyCallback(window,GLFW_KEY_UP,0,GLFW_PRESS,0);
    }
    assert(std::abs(tankX-.8f)<.0001f && std::abs(tankZ+.8f)<.0001f);
    moveScene();auto moved=render("moved");
    assert(moved[1].x-initial[1].x>20 && moved[1].y-initial[1].y>20);
    assert(moved[2].x-initial[2].x>20 && std::abs(moved[2].y-initial[2].y)<2);
    assert(std::abs(moved[0].x-initial[0].x)>10);
    keyCallback(window,GLFW_KEY_C,0,GLFW_PRESS,0);
    assert(tankX==0 && tankZ==0);
    keyCallback(window,GLFW_KEY_D,0,GLFW_PRESS,0);
    keyCallback(window,GLFW_KEY_T,0,GLFW_PRESS,0);
    keyCallback(window,GLFW_KEY_L,0,GLFW_PRESS,0);
    keyCallback(window,GLFW_KEY_G,0,GLFW_PRESS,0);
    keyCallback(window,GLFW_KEY_P,0,GLFW_PRESS,0);
    for(int i=0;i<30;++i)moveScene();render("animated");
    resetScene();
    const int sizes[2][2]{{1500,700},{700,1000}};
    const char* names[2]{"wide","tall"};
    for(int i=0;i<2;++i) {
        glfwSetWindowSize(window,sizes[i][0],sizes[i][1]);glfwPollEvents();
        int width,height;glfwGetFramebufferSize(window,&width,&height);
        framebufferSize(window,width,height);render(names[i]);
    }
    glDeleteBuffers(1,&VBO);glDeleteVertexArrays(1,&VAO);glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);glfwTerminate();
    std::cout<<"PASS: three viewports, correct XZ/XY projections, shared movement/animation, resize and GL state\n";
}
