#include<gz4d/glut/Application.h>
#include <gz4d/Kinect.h>

class MyScene: public gz4d::glut::Application<MyScene>
{
    private:
        gz4d::Kinect k;
    public:
        MyScene()
        {
        }
        
        void Idle()
        {
            gz4d::Kinect::PointCloud pc = k.getPointCloud();
            PostRedisplay();
        }
        
        void Draw()
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            //gz4d::Kinect::Frame f = k.getFrame();
            gz4d::Kinect::PointCloud pc = k.getPointCloud(5.0);

            glPushMatrix();
            
//            glEnable(GL_LIGHTING);
/*            glColor3f(1.0,0.0,0.0);
            Teapot(2.0);*/
            //glScalef(0.1,0.1,0.1);
            glDisable(GL_LIGHTING);
            //gz4d::glo::Buffer abo(GL_ARRAY_BUFFER);
            if(!pc.empty())
            {
                glInterleavedArrays(GL_C3F_V3F,0,&pc.front());
                glDrawArrays(GL_POINTS,0,pc.size());
/*                glBegin(GL_POINTS);
                for(gz4d::Kinect::PointCloud::const_iterator i = pc.begin(); i != pc.end(); ++i)
                {
                    glColor3fv(&i->r);
                    glVertex3fv(&i->x);
                }
                glEnd();*/
            }
            glPopMatrix();

            SwapBuffers();
        }
};

int main(int argc, char * argv[])
{
    MyScene s;
    s("Kinect");
    return 0;
}
