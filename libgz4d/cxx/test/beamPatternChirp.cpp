// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/acoustic/BeamPattern.h>
#include <gz4d/acoustic/UniformArray.h>
#include <gz4d/acoustic/RectangularElement.h>
#include <gz4d/acoustic/CircularElement.h>
#include <gz4d/render/RasterSurface.h>
#include <gz4d/render/Grid.h>
#include <gz4d/glut/Application.h>
#include <gz4d/time.h>
#include <gz4d/av.h>
#include<gz4d/glo/FramebufferObject.h>
#include <gz4d/glo/ReadPixels.h>


class GlutRaster: public gz4d::RasterSurface<gz4d::SphericalRaster<float, float> >, public gz4d::glut::Application<GlutRaster>
{
    gz4d::acoustic::UniformArray::Ptr _array;

public:
    typedef std::map<double,gz4d::SphericalRaster<float,float> > RasterMap;
private:

    RasterMap _rasters;
    RasterMap::const_iterator _current_raster;

    bool _steering;
    float _steering_startx;
    float _steering_starty;
    float _start_steering_x;
    float _start_steering_y;

    bool _update_steering;

    gz4d::acoustic::Environment _env;

    std::shared_ptr<gz4d::AVWriter> _video_writer;
    int _frame_number;
    gz4d::glo::Framebuffer _fbo;
    gz4d::glo::Renderbuffer _rb;
    gz4d::glo::Renderbuffer _db;


    public:
        GlutRaster(gz4d::acoustic::UniformArray::Ptr a, RasterMap const & srs, gz4d::acoustic::Environment const &env, std::string const & video_out=std::string()):gz4d::RasterSurface<gz4d::SphericalRaster<float,float> >(srs.begin()->second),_array(a),_rasters(srs),_steering(false),_update_steering(false),_env(env),_frame_number(0),_fbo(context),_rb(context),_db(context,GL_DEPTH_COMPONENT)
        {
            InitDisplayMode(GLUT_DOUBLE|GLUT_DEPTH);
            _current_raster = _rasters.begin();

            if(video_out.size())
            {
                _video_writer = std::shared_ptr<gz4d::AVWriter>(new gz4d::AVWriter);
                _video_writer->SetSize(1024,1024);
                _video_writer->SetFPS(30);
                _video_writer->Open(video_out);
            }

        }

        void Draw()
        {
            if(_video_writer)
            {
                std::cout << "frame: " << _frame_number << std::endl;
                double w0 = _rasters.begin()->first;
                double w1 = _rasters.rbegin()->first;
                double t = _frame_number/30.0;
                double p = cos(t/2.0);
                double w = w0+(w1-w0)*(p+1.0)/2.0;
                RasterMap::const_iterator r = _rasters.lower_bound(w);
                if (r == _rasters.end())
                    r = _rasters.begin();
                if (r != _current_raster)
                {
                    _current_raster = r;
                    SetRaster(_current_raster->second);
                }
                
                float heading = M_PI*2.0*_frame_number/360.0;
                float pitch = -25.0*M_PI*2.0/360.0;
                
                gz4d::Rotation<float> rx(gz4d::Angle<float,gz4d::Radian,gz4d::ZeroCenteredPeriod>(heading),gz4d::Point<float>(0.0,1.0,0.0));
                gz4d::Rotation<float> ry(gz4d::Angle<float,gz4d::Radian,gz4d::ZeroCenteredPeriod>(pitch),gz4d::Point<float>(-1.0,0.0,0.0));
                _rotation = ry*rx;
                
                _fbo.Bind();
                if(_frame_number == 0)
                {
                    _rb.Bind();
                    _rb.Storage(1024,1024);
                    _fbo.Attach(_rb);

                    _db.Bind();
                    _db.Storage(1024,1024);
                    _fbo.Attach(_db);
                    
                    _fbo.CheckStatus();
                    glClearColor(0.0,0.0,0.0,1.0);
                }
                glPushAttrib(GL_VIEWPORT_BIT| GL_COLOR_BUFFER_BIT);
                glViewport(0,0,1024, 1024);
                glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                glFrustum(-1.0,1.0,-1.0,1.0,1.0,3.0);
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadIdentity();
                glTranslatef(0.0,0.0,-2.0);
                
                glScalef(_scale,_scale,_scale);
                gz4d::glo::MultMatrix(_rotation.GetMatrix());
                
                glRotated(-90.0,1.0,0.0,0.0);
                gz4d::RasterSurface<gz4d::SphericalRaster<float, float> >::Draw();
                
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
                glPopMatrix();
                
                glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
                gz4d::ImageRGBA8 frame;
                frame.Resize(1024,1024);
                gz4d::glo::ReadPixels(frame.GetRaster());
                gz4d::ImageYUV8 yuvFrame(frame.GetRaster());
                _video_writer->AddFrame(yuvFrame);

                _fbo.Unbind();
                
                glPopAttrib();
                ++_frame_number;
                if (_frame_number > 900)
                    _video_writer.reset();
            }
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
            if(_video_writer)
                return;
            gz4d::glut::Application<GlutRaster>::Mouse(button,state,x,y);
            if(button == 0)
            {
                if(state == 0)
                {
                    //_steering = true;
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
                //gz4d::acoustic::BeamPattern(*_array,_raster,_w,_env,1.0);
                //SetRaster(_raster);
                PostRedisplay();
            }
            if(_video_writer)
            {
                PostRedisplay();

            }
            else
            {
                double w0 = _rasters.begin()->first;
                double w1 = _rasters.rbegin()->first;
                double p = sin(gz4d::time::Now().secondsSinceEpoch().asSeconds()/2.0);
                double w = w0+(w1-w0)*(p+1.0)/2.0;
                RasterMap::const_iterator r = _rasters.lower_bound(w);
                if (r == _rasters.end())
                    r = _rasters.begin();
                if (r != _current_raster)
                {
                    _current_raster = r;
                    SetRaster(_current_raster->second);
                    PostRedisplay();
                }
            }
        }

};


int main(int argc, char * argv[])
{
    double f0 = 2000.0; // 2 to 9 Khz
    double f1 = 9000.0;
    double df = 100.0;


    gz4d::acoustic::Environment env(1500.0); // sound speed

    float element_size = 0.18415; // square element, in meters
    float spacing = 0.2095; // element spacing in meters

    gz4d::acoustic::Element::Ptr e(new gz4d::acoustic::RectangularElement(gz4d::acoustic::Size(element_size,element_size)));
    gz4d::acoustic::UniformArray::Ptr a(new gz4d::acoustic::UniformArray(e));
    a->SetNM(4,4);
    a->SetSpacing(gz4d::acoustic::Size(spacing,spacing));

    GlutRaster::RasterMap rasters;

    for(double f = f0; f <= f1; f += df)
    {

        double w = 2.0*M_PI*f;


        std::cout << "f: " << f << "\telement beamwidth: " <<  180.0*e->beamWidth(w,env)/M_PI;
        std::cout << " \tarray beamwidth: " <<  180.0*a->beamWidth(w,env)/M_PI << std::endl;

        gz4d::Raster<float>::Ptr r(new gz4d::Raster<float>);
        r->Resize(300,300);

        //gz4d::SphericalRaster<float,float> sr;

        rasters[w].SetPhiInterval(0.0,M_PI*2.0);
        rasters[w].SetThetaInterval(0.0,M_PI/2.0);
        rasters[w].SetValueScaler(1/50.0,1);
        rasters[w].SetRaster(r);

        gz4d::acoustic::BeamPattern(*a,rasters[w],w,env,1.0);
    }


    if(argc == 1)
    {
        GlutRaster s(a,rasters,env);
        s("Beam Pattern");
    }
    else
    {
        GlutRaster s(a,rasters,env,argv[1]);
        s("Beam Pattern");
    }
}
