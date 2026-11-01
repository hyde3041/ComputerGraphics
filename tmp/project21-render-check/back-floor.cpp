#define OBJ_DIRECTORY "C:/study/2-2/ComputerGraphics/OpenGL/Project1/models"
#define main project21Main
#include "../../OpenGL/Project1/Project1-21.cpp"
#undef main
#include <cassert>
#include <array>

bool close(float a,float b) { return std::abs(a-b)<0.00001f; }
void checkInside() {
    for (const auto& s:cir) {
        assert(std::abs(s.Xpos)<=rec[0].Xsize-s.radius+0.00001f);
        assert(std::abs(s.Ypos)<=rec[0].Ysize-s.radius+0.00001f);
        assert(std::abs(s.Zpos)<=rec[0].Zsize-s.radius+0.00001f);
    }
}
int main() {
    std::srand(12345);
    setScene();
    assert(rec.size()==4 && cir.size()==1);
    for(int i=1;i<4;++i) assert(close(rec[i].Ypos-rec[i].Ysize,-rec[0].Ysize));
    keyCallback(nullptr,GLFW_KEY_Z,0,GLFW_PRESS,0);
    assert(close(rec[0].Zpos,.1f));
    keyCallback(nullptr,GLFW_KEY_Z,0,GLFW_PRESS,GLFW_MOD_SHIFT);
    assert(close(rec[0].Zpos,0));
    keyCallback(nullptr,GLFW_KEY_Y,0,GLFW_PRESS,0);
    assert(close(rec[0].Yangle,-5));
    keyCallback(nullptr,GLFW_KEY_Y,0,GLFW_PRESS,GLFW_MOD_SHIFT);
    assert(close(rec[0].Yangle,0));
    for(int i=0;i<20;++i) keyCallback(nullptr,GLFW_KEY_B,0,GLFW_PRESS,0);
    assert(cir.size()==5);
    std::array<glm::vec3,5> speeds;
    for(int i=0;i<5;++i) {
        speeds[i]=glm::abs(glm::vec3(cir[i].Xmove,cir[i].Ymove,cir[i].Zmove));
        for(int j=0;j<3;++j) assert(speeds[i][j]>=.00199f && speeds[i][j]<=.00801f);
    }
    mouseDrag=true;lastMouseX=0;
    cursorPosition(nullptr,100,0);
    assert(close(rec[0].Zangle,-30));
    mouseDrag=false;
    for(int frame=0;frame<10000;++frame) { moveScene();checkInside(); }
    for(int i=1;i<4;++i) assert(rec[i].Xpos>.1f && rec[i].Xpos+rec[i].Xsize<=1.00001f);
    for(int i=0;i<5;++i) {
        assert(close(std::abs(cir[i].Xmove),speeds[i].x));
        assert(close(std::abs(cir[i].Ymove),speeds[i].y));
        assert(close(std::abs(cir[i].Zmove),speeds[i].z));
    }
    keyCallback(nullptr,GLFW_KEY_R,0,GLFW_PRESS,0);
    keyCallback(nullptr,GLFW_KEY_F,0,GLFW_PRESS,0);
    for(int i=0;i<90;++i)moveScene();
    assert(close(floorAngle,90));
    cir[0].Zpos=-.799f;cir[0].Zmove=-.008f;
    moveScene(); assert(cir[0].Zpos<-.8f);
    for(int i=0;i<130;++i)moveScene();
    assert(cir[0].Zpos<-1.0f);
    for(int i=1;i<4;++i)assert(rec[i].Zpos<-1.0f);
    const float oldSphereZ=cir[0].Zpos;
    keyCallback(nullptr,GLFW_KEY_F,0,GLFW_PRESS,0);
    for(int i=0;i<90;++i)moveScene();
    assert(close(floorAngle,0));
    assert(cir[0].Zpos<oldSphereZ);
    for(int i=1;i<4;++i)assert(rec[i].Zpos<-1.0f);
    keyCallback(nullptr,GLFW_KEY_R,0,GLFW_PRESS,0);
    assert(rec.size()==4 && cir.size()==1 && close(rec[0].Zangle,0));
    std::cout<<"PASS: controls, random velocities, 10000-frame bounds, sliding, negative-Z fall, close after exit, reset\n";

    if(!loadOBJ("cube.obj",cubeVertices)||!loadOBJ("sphere.obj",sphereVertices))return 1;
    if(!glfwInit())return 2;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
    GLFWwindow* w=glfwCreateWindow(800,600,"render-check",nullptr,nullptr);
    if(!w)return 3;
    glfwMakeContextCurrent(w);glewExperimental=GL_TRUE;
    if(glewInit()!=GLEW_OK)return 4;
    shaderProgram=createShader();if(!shaderProgram)return 5;
    glUseProgram(shaderProgram);
    transformLocation=glGetUniformLocation(shaderProgram,"transform");
    colorLocation=glGetUniformLocation(shaderProgram,"drawColor");initBuffer();
    glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);
    glClearColor(.07f,.09f,.13f,1);glViewport(0,0,800,600);
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    while(glGetError()!=GL_NO_ERROR){}
    std::vector<unsigned char> pixels(800*600*3),initial;
    const unsigned char colors[9][3]{{255,0,0},{0,255,0},{0,0,255},{255,255,0},{255,0,255},{0,255,255},
        {255,89,38},{230,140,38},{204,64,26}};
    for(int mode=0;mode<4;++mode) {
        setScene();
        if(mode==1) { rec[0].Zangle=-30;for(int i=0;i<180;++i)moveScene(); }
        if(mode==2) { rec[0].Yangle=30; }
        if(mode==3) { floorOpen=true;for(int i=0;i<160;++i)moveScene(); }
        drawScene();glFinish();glReadBuffer(GL_BACK);
        glReadPixels(0,0,800,600,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
        assert(glGetError()==GL_NO_ERROR);
        int gray=0,changed=0;std::array<int,9> counts{};
        for(size_t i=0;i<pixels.size();i+=3) {
            if(std::abs(int(pixels[i])-77)<=1&&std::abs(int(pixels[i+1])-77)<=1&&std::abs(int(pixels[i+2])-77)<=1)++gray;
            for(int c=0;c<9;++c)if(std::abs(int(pixels[i])-colors[c][0])<=1&&std::abs(int(pixels[i+1])-colors[c][1])<=1&&std::abs(int(pixels[i+2])-colors[c][2])<=1)++counts[c];
            if(mode && (pixels[i]!=initial[i]||pixels[i+1]!=initial[i+1]||pixels[i+2]!=initial[i+2]))++changed;
        }
        if(mode==0) {
            initial=pixels;
            assert(gray>100);
            for(int c=6;c<9;++c)assert(counts[c]>20);
            assert(counts[1]==0);
            for(int c:{0,2,3,4,5})assert(counts[c]>20);
        } else assert(changed>500);
        std::cout<<"render "<<mode<<": sphere="<<gray<<" changed="<<changed<<" cube pixels="<<counts[6]<<','<<counts[7]<<','<<counts[8]<<"\n";
        std::ofstream file("tmp/project21-render-check/back-floor-"+std::to_string(mode)+".ppm",std::ios::binary);
        file<<"P6\n800 600\n255\n";
        for(int y=599;y>=0;--y)file.write(reinterpret_cast<char*>(pixels.data()+y*800*3),800*3);
    }
    glDeleteBuffers(1,&VBO);glDeleteVertexArrays(1,&VAO);glDeleteProgram(shaderProgram);
    glfwDestroyWindow(w);glfwTerminate();
    std::cout<<"PASS: GPU rendering; 5 interior faces, 3 visible cubes, sphere, rotation and floor opening\n";
}
