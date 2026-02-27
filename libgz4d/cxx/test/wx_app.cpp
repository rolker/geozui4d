
#ifdef GZ4D_WX

#include<gz4d/wxGLApplication.h>
#include <GL/glut.h>

class MyScene: public gz4d::Drawable
{
    void Draw(std::shared_ptr<gz4d::glo::Context> context)
    {
        glColor3f(.3,.4,1.0);
        glutSolidTeapot(0.2);
    }
};

class MyApp: public gz4d::wxGLApplication
{
    public:
        bool OnInit()
        {
            glutInit(&argc, (char **)argv);

            MyScene *s = new MyScene;
            gz4d::Drawable::Ptr d(s);
            sm.AddDrawable(d);
            return wxGLApplication::OnInit();
        }
};

IMPLEMENT_APP(MyApp)

#else
int main(int argc, char*argv[]){return 1;}
#endif
