#define OBJ_DIRECTORY "C:/study/2-2/ComputerGraphics/OpenGL/Project1/models"
#define main project21Main
#include "../../OpenGL/Project1/Project1-21.cpp"
#undef main
#include <cassert>

static bool near(float a,float b) { return std::fabs(a-b)<0.00001f; }
int main()
{
    setScene();
    assert(rec.size()==1 && cir.size()==1);
    const Sphere start=cir[0];
    moveScene();
    assert(near(cir[0].Xpos,start.Xpos+start.Xmove));
    assert(near(cir[0].Ypos,start.Ypos+start.Ymove));
    assert(near(cir[0].Zpos,start.Zpos+start.Zmove));
    for(int axis=0;axis<3;++axis)for(int side=-1;side<=1;side+=2){
        Sphere s=start;s.Xmove=s.Ymove=s.Zmove=0;
        if(axis==0){s.Xpos=side*.499f;s.Xmove=side*.01f;}
        if(axis==1){s.Ypos=side*.499f;s.Ymove=side*.01f;}
        if(axis==2){s.Zpos=side*.499f;s.Zmove=side*.01f;}
        cir.assign(1,s);moveScene();
        const Sphere& out=cir[0];
        assert(near(axis==0?out.Xpos:axis==1?out.Ypos:out.Zpos,side*.5f));
        assert(near(axis==0?out.Xmove:axis==1?out.Ymove:out.Zmove,-side*.01f));
    }
    cir.assign(1,start);cir[0].Xpos=cir[0].Ypos=cir[0].Zpos=.499f;
    moveScene();
    assert(near(cir[0].Xpos,.5f)&&near(cir[0].Ypos,.5f)&&near(cir[0].Zpos,.5f));
    assert(cir[0].Xmove<0&&cir[0].Ymove<0&&cir[0].Zmove<0);
    rec[0].Xpos=3;rec[0].Ypos=-2;rec[0].Zpos=4;
    rec[0].Xsize=2;rec[0].Ysize=3;rec[0].Zsize=4;
    cir.assign(1,start);
    cir[0].Xpos=4.499f;cir[0].Ypos=.499f;cir[0].Zpos=7.499f;
    moveScene();
    assert(near(cir[0].Xpos,4.5f)&&near(cir[0].Ypos,.5f)&&near(cir[0].Zpos,7.5f));
    assert(cir[0].Xmove<0&&cir[0].Ymove<0&&cir[0].Zmove<0);
    rec.clear();cir.clear();setScene();
    Sphere second=cir[0];second.Xmove=-second.Xmove;second.Ymove=-second.Ymove;second.Zmove=-second.Zmove;cir.push_back(second);
    for(int frame=0;frame<20000;++frame){
        moveScene();
        for(const Sphere& s:cir){
            assert(s.Xpos>=-.5f&&s.Xpos<=.5f);
            assert(s.Ypos>=-.5f&&s.Ypos<=.5f);
            assert(s.Zpos>=-.5f&&s.Zpos<=.5f);
            assert(near(std::fabs(s.Xmove),std::fabs(start.Xmove)));
            assert(near(std::fabs(s.Ymove),std::fabs(start.Ymove)));
            assert(near(std::fabs(s.Zmove),std::fabs(start.Zmove)));
        }
    }
    assert(near(cir[0].Xpos,-cir[1].Xpos)&&near(cir[0].Ypos,-cir[1].Ypos)&&near(cir[0].Zpos,-cir[1].Zpos));
    rec.clear();cir.assign(1,start);moveScene();assert(near(cir[0].Xpos,start.Xmove));
    std::cout<<"PASS: movement, six walls, corner reflection, shifted/resized box, two spheres, 20000 frames, empty box.\n";
}

