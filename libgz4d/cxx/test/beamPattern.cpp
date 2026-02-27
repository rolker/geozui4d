#include <gz4d/acoustic/BeamPattern.h>
#include <gz4d/acoustic/UniformArray.h>
#include <gz4d/acoustic/RectangularElement.h>
#include <gz4d/acoustic/CircularElement.h>
#include <gz4d/render/RasterSurface.h>
#include <gz4d/render/Grid.h>
#include <gz4d/glut/Application.h>
#include <gz4d/time.h>

class GlutRaster: public gz4d::RasterSurface<gz4d::SphericalRaster<float, float> >, public gz4d::glut::Application<GlutRaster>
{
    gz4d::acoustic::UniformArray::Ptr _array;
    gz4d::SphericalRaster<float,float> _raster;
    
    bool _steering;
    float _steering_startx;
    float _steering_starty;
    float _start_steering_x;
    float _start_steering_y;
    
    bool _update_steering;
    
    double _w;
    gz4d::acoustic::Environment _env;
    public:
        GlutRaster(gz4d::acoustic::UniformArray::Ptr a, gz4d::SphericalRaster<float,float> sr, double w, gz4d::acoustic::Environment const &env):gz4d::RasterSurface<gz4d::SphericalRaster<float, float> >(sr),_array(a),_raster(sr),_steering(false),_update_steering(false),_w(w),_env(env)
        {
            InitDisplayMode(GLUT_DOUBLE|GLUT_DEPTH);
        }
        
        void Draw()
        {
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
            glEnable(GL_DEPTH_TEST);
            //const GLfloat light_dir[] = {0.0f,-1.0f,0.0f,0.0f};
            //glLightfv(GL_LIGHT0,GL_POSITION,light_dir);
            //const GLfloat light_amb[] = {0.2f, 0.2f, 0.2f, 1.0f};
            //glLightfv(GL_LIGHT0,GL_AMBIENT,light_amb);
            glColor3f(1.0,1.0,1.0);
            glPushMatrix();
            glRotated(-90.0,1.0,0.0,0.0);
            //double rot = fmod(gz4d::time::Now().secondsSinceEpoch().asSeconds()*30.0,360.0);
            //glRotated(rot,0.0,0.0,1.0);
            gz4d::RasterSurface<gz4d::SphericalRaster<float, float> >::Draw();
            glPopMatrix();
            SwapBuffers();
        }
        
        void Mouse(int button, int state,int x, int y)
        {
            gz4d::glut::Application<GlutRaster>::Mouse(button,state,x,y);
            
            if(button == 0)
            {
                if(state == 0)
                {
                    _steering = true;
                    _steering_startx = _canvas_mouse_x;
                    _steering_starty = _canvas_mouse_y;
                    _start_steering_x = _array->getSteeringX().value();
                    _start_steering_y = _array->getSteeringY().value();
                }
                else
                {
                    _steering = false;
                }
            }
            
        }
        
        void Motion(int x, int y)
        {
            gz4d::glut::Application<GlutRaster>::Motion(x,y);
            if(_steering)
            {
                float dx = _canvas_mouse_x-_steering_startx;
                float dy = _canvas_mouse_y-_steering_starty;
                _array->SetSteering(_start_steering_x+10.0*dx,_start_steering_y+10.0*dy);
                std::cerr << _start_steering_x+dx << ", " << _start_steering_y+dy << std::endl;
                _update_steering = true;
            }
        }
        
        void Idle()
        {
            if(_update_steering)
            {
                _update_steering = false;
                gz4d::acoustic::BeamPattern(*_array,_raster,_w,_env,1.0);
                SetRaster(_raster);
                PostRedisplay();
            }
        }
        
};


int main()
{
    //gz4d::glut::Application g;
    //g.NewCanvas("Beam Pattern Test");
    gz4d::acoustic::Environment env;
    double w = 1500*2.0*M_PI;
    
    gz4d::acoustic::Element::Ptr e(new gz4d::acoustic::RectangularElement(gz4d::acoustic::Size(0.5,0.5)));
    //gz4d::acoustic::Element::Ptr e(new gz4d::acoustic::CircularElement(1.59));
    //gz4d::acoustic::Element::Ptr e(new gz4d::acoustic::Element());
    gz4d::acoustic::UniformArray::Ptr a(new gz4d::acoustic::UniformArray(e));
    a->SetNM(8,4);
    a->SetSpacing(gz4d::acoustic::Size(0.5,0.5));
    a->SetSteering(M_PI/6.0,0.0);

    gz4d::Raster<float>::Ptr r(new gz4d::Raster<float>);
    r->Resize(150,150);

    gz4d::SphericalRaster<float,float> sr;

    sr.SetPhiInterval(0.0,M_PI*2.0);
    sr.SetThetaInterval(0.0,M_PI/2.0);
    sr.SetValueScaler(1/50.0,1);
    sr.SetRaster(r);

    gz4d::acoustic::BeamPattern(*a,sr,w,env,1.0);

    GlutRaster s(a,sr,w,env);
    //gz4d::RasterSurface<gz4d::SphericalRaster<float,float> >::Ptr s(new gz4d::RasterSurface<gz4d::SphericalRaster<float,float> >(sr));
    //g.Create3DViewer(s);

/*    gz4d::Grid<gz4d::SphericalPoint<float> > *grid = new gz4d::Grid<gz4d::SphericalPoint<float> >;
    grid->SetStartPoint(gz4d::SphericalPoint<float>(0.0,gz4d::Radians(-90.0),0.0));
    grid->AddDimension(gz4d::SphericalPoint<float>(0.0,gz4d::Radians(5.0),0.0),36);
    grid->AddDimension(gz4d::SphericalPoint<float>(0.2,0.0,0.0),5);*/
//     gz4d::Drawable::Ptr gd(grid);
//     g.SetScene(gd);

    //g();
    s("Beam Pattern");
}
