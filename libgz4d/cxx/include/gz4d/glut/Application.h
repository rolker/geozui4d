// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLUT_APPLICATION_H
#define GZ4D_GLUT_APPLICATION_H

//gz4d_cmake depends GLUT

#include <gz4d/gz4d.h>
#include <gz4d/glo.h>

#ifdef _WIN32
    #ifndef WINVER
        #define WINVER 0x0501
    #endif
    #ifdef NOMINMAX
        #undef NOMINMAX
    #endif
#endif

#include <GL/glut.h>

#include <iostream>
#include <gz4d/glo/Context.h>
#include <gz4d/glo/MultMatrix.h>
#include <gz4d/Rotation.h>

namespace gz4d
{
    namespace glut
    {
        class NullViewControl
        {
        protected:
            void Mouse(int button, int state, float x, float y){}
            void ApplyView(){}
            void Motion(float x, float y){}
        };
        
        class ViewControl
        {
        protected:
            float _scale;
            float _min_scale;
            float _max_scale;
          
            typedef std::shared_ptr<ViewControl> Ptr;
            
            ViewControl():_scale(1.0),_min_scale(0.0001),_max_scale(1000.0)
            {
            }
            
            void setMinScale(float s, bool redisplay=true) {_min_scale = s; updateScale(_scale,redisplay);}
            void setMaxScale(float s, bool redisplay=true) {_max_scale = s; updateScale(_scale,redisplay);}
            
            void updateScale(float s, bool redisplay=true)
            {
                _scale = std::max(std::min(s,_max_scale),_min_scale);
                if(redisplay)
                    glutPostRedisplay();
            }
            
            void Mouse(int button, int state, float x, float y)
            {
                //std::cerr << "button: " << button << "\tstate: " << state << std::endl;
                if (button == 3 && state == 1)
                {
                    updateScale(_scale*1.1);
                }
                if (button == 4 && state == 1)
                {
                    updateScale(_scale/1.1);
                }
                //std::cerr << "scale: " << _scale << std::endl;
            }
            
            void Motion(float x, float y)
            {
            }
        };


        class OrthoViewControl: public ViewControl
        {
        protected:
            bool _drag;
            float _drag_startx;
            float _drag_starty;
            float _start_offset_x;
            float _start_offset_y;
            float _offset_x;
            float _offset_y;
            
            OrthoViewControl():_drag(false),_offset_x(0.0),_offset_y(0.0)
            {
            }
            
            void Mouse(int button, int state, float x, float y)
            {
                ViewControl::Mouse(button,state,x,y);
                
                if(button == 1)
                {
                    if(state == 0)
                    {
                        _drag = true;
                        _drag_startx = x;
                        _drag_starty = y;
                        _start_offset_x = _offset_x;
                        _start_offset_y = _offset_y;
                    }
                    else
                    {
                        _drag = false;
                    }
                }
            }
            
            void Motion(float x, float y)
            {
                if(_drag)
                {
                    float dx = x-_drag_startx;
                    float dy = y-_drag_starty;
                    _offset_x = _start_offset_x + dx/_scale;
                    _offset_y = _start_offset_y + dy/_scale;
                    glutPostRedisplay();
                }
            }
            
            
            void ApplyView()
            {
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glScalef(_scale,_scale,_scale);
                glTranslatef(_offset_x,_offset_y,0.0);
            }
            
        };
        
        class PerspectiveViewControl: public ViewControl
        {
        protected:
            bool _drag;
            float _drag_startx;
            float _drag_starty;
            Rotation<float> _rotation;
            Rotation<float> _start_rotation;
            
            void Mouse(int button, int state, float x, float y)
            {
                ViewControl::Mouse(button,state,x,y);
                
                if(button == 1)
                {
                    if(state == 0)
                    {
                        _drag = true;
                        _drag_startx = x;
                        _drag_starty = y;
                        _start_rotation = _rotation;
                    }
                    else
                    {
                        _drag = false;
                    }
                }
            }
            
            void Motion(float x, float y)
            {
                if(_drag)
                {
                    float dx = x-_drag_startx;
                    float dy = y-_drag_starty;
                    Rotation<float> rx(Angle<float,Radian,ZeroCenteredPeriod>(dx),Point<float>(0.0,1.0,0.0));
                    Rotation<float> ry(Angle<float,Radian,ZeroCenteredPeriod>(dy),Point<float>(-1.0,0.0,0.0));
                    _rotation = rx*ry*_start_rotation;
                    glutPostRedisplay();
                }
            }
            
            void ApplyView()
            {
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glTranslated(0.0,0.0,-2.0);
                glScalef(_scale,_scale,_scale);
                glo::MultMatrix(_rotation.GetMatrix());
            }
            
        };
        
        template <typename VC> class Base: public VC
        {
            protected:
                int _window_width;
                int _window_height;
                float _canvas_width;
                float _canvas_height;
                float _canvas_mouse_x;
                float _canvas_mouse_y;

                Base():_window_width(0),_window_height(0),_canvas_width(1.0),_canvas_height(1.0),_canvas_mouse_x(0.0),_canvas_mouse_y(0.0){}
                void Reshape(int width, int height)
                {
                    _window_width = width;
                    _window_height = height;

                    glViewport(0,0,_window_width,_window_height);

                    if(_window_width>0 && _window_height>0)
                    {
                        _canvas_width = 1.0;
                        _canvas_height = 1.0;
                        if(_window_width>_window_height)
                            _canvas_width = _window_width/float(_window_height);
                        else
                            _canvas_height = _window_height/float(_window_width);
                    }
                }
                
                void CalculateMouseCanvasPosition(int x, int y)
                {
                    _canvas_mouse_x = -_canvas_width+(2.0*_canvas_width*x)/float(_window_width);
                    _canvas_mouse_y = _canvas_height-(2.0*_canvas_height*y)/float(_window_height);
                }
                
                void Keyboard(unsigned char key, int x, int y)
                {
                    CalculateMouseCanvasPosition(x,y);
                }
                void Special(int key, int x, int y)
                {
                    CalculateMouseCanvasPosition(x,y);
                }
                
                void Mouse(int button, int state,int x, int y)
                {
                    CalculateMouseCanvasPosition(x,y);
                    VC::Mouse(button,state,_canvas_mouse_x,_canvas_mouse_y);
                }
                
                void Motion(int x, int y)
                {
                    CalculateMouseCanvasPosition(x,y);
                    VC::Motion(_canvas_mouse_x,_canvas_mouse_y);
                }
                
                void PassiveMotion(int x, int y)
                {
                    CalculateMouseCanvasPosition(x,y);
                }
                
                void Idle(){}
                void ApplyView()
                {
                    VC::ApplyView();
                }
        };

        template<typename VC = OrthoViewControl> class Ortho: public Base<VC>
        {
            protected:
                void Reshape(int width, int height)
                {
                    Base<VC>::Reshape(width,height);
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    glOrtho(-Base<VC>::_canvas_width,Base<VC>::_canvas_width,-Base<VC>::_canvas_height,Base<VC>::_canvas_height,-1.0,1.0);
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                }
        };
        
        template<typename VC = PerspectiveViewControl> class Perspective: public Base<VC>
        {
            protected:
                void Reshape(int width, int height)
                {
                    Base<VC>::Reshape(width,height);
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    glFrustum(-Base<VC>::_canvas_width,Base<VC>::_canvas_width,-Base<VC>::_canvas_height,Base<VC>::_canvas_height,1.0,3.0);
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    //glScalef(_scale,_scale,_scale);
                    glTranslated(0.0,0.0,-2.0);
                }
        };
        
        template <typename T, typename B=Perspective<> > class Application: public B
        {
            
            static T* _self;
            static void display()
            {
                glo::Context::Active::Token c = _self->context->activate();
                _self->ApplyView();
                _self->Draw();
            }
            static void reshape(int width, int height)
            {
                _self->Reshape(width, height);
            }
            static void keyboard(unsigned char key, int x, int y)
            {
                _self->Keyboard(key, x, y);
            }
            static void special(int key, int x, int y)
            {
                _self->Special(key, x, y);
            }
            static void mouse(int button, int state,int x, int y)
            {
                _self->Mouse(button, state, x, y);
            }
            static void motion(int x, int y)
            {
                _self->Motion(x, y);
            }
            static void passiveMotion(int x, int y)
            {
                _self->PassiveMotion(x, y);
            }
            static void idle()
            {
                _self->Idle();
            }

            protected:
                glo::Context::Ptr context;
                
                void SwapBuffers()
                {
                    glutSwapBuffers();
                }

                void PostRedisplay()
                {
                    glutPostRedisplay();
                }

                void Teapot(double size, bool solid = true)
                {
                    if(solid)
                        glutSolidTeapot(size);
                    else
                        glutWireTeapot(size);
                }

                void Fullscreen()
                {
                    glutFullScreen();
                }

            public:
                Application():context(glo::Context::create())
                {
                    int argc = 0;
                    glutInit(&argc,NULL);
                    glutInitDisplayMode(GLUT_DOUBLE);
                    _self = static_cast<T*>(this);
                }
                void InitDisplayMode(unsigned int mode)
                {
                    glutInitDisplayMode(mode);
                }
                void InitWindowSize(int width, int height)
                {
                    glutInitWindowSize(width, height);
                }

                void operator()(std::string const &title)
                {
                    glutCreateWindow(title.c_str());
                    glutDisplayFunc(&display);
                    glutReshapeFunc(&reshape);
                    glutKeyboardFunc(&keyboard);
                    glutSpecialFunc(&special);
                    glutMouseFunc(&mouse);
                    glutMotionFunc(&motion);
                    glutPassiveMotionFunc(&passiveMotion);
                    glutIdleFunc(&idle);

                    glutMainLoop();
                }
        };

    }
}

template <typename T, typename B> T * gz4d::glut::Application<T,B>::_self = NULL;

#endif
