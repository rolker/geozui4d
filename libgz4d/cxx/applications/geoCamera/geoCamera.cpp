#include <gz4d/glut/Window.h>
#include<gz4d/v4l/Stream.h>
#include<gz4d/v4l/Capability.h>
#include<gz4d/Image.h>
#include<gz4d/glo/Texture.h>
#include <gz4d/ptu/Ptu.h>
#include <gz4d/js/Joystick.h>
#include <gz4d/thread/ThreadedQueue.h>
#include <gz4d/ui/Joystick.h>
#include <gz4d/gPhoto/Camera.h>
#include <gz4d/time.h>
//#include <vw/Mosaic.h>

class GeoCamera: public gz4d::glut::Window
{
    gz4d::glo::Texture previewTexture,webcamTexture,canonTexture;
    gz4d::v4l::Device device;
    std::shared_ptr<gz4d::v4l::Stream> stream;

    gz4d::Ptu ptu;
    gz4d::Joystick js;
    gz4d::ThreadedQueue<gz4d::ui::Event>::Ptr jsq;

    gz4d::gphoto::Camera camera;

    float pan_max_speed;
    float tilt_max_speed;

    int width;
    int height;

    float nwidth;
    float nheight;

    gz4d::ImageRGB8 canon_image,webcam_image;

    bool snapshot;
    public:
        GeoCamera()
    : Window("GeoCamera"),previewTexture(GL_TEXTURE_2D,GL_RGB),webcamTexture(GL_TEXTURE_2D,GL_RGB),canonTexture(GL_TEXTURE_2D,GL_RGB),device("/dev/video0"),ptu("/dev/ttyUSB0"),js("/dev/input/js0"),snapshot(false)
        {
            ptu.PanSpeed(0.0);
            ptu.TiltSpeed(0.0);
            ptu.PanAcceleration(400.0);
            ptu.TiltAcceleration(400.0);

            jsq = js.NewOutQueue();

            pan_max_speed = ptu.GetPanMaxSpeed();
            tilt_max_speed = ptu.GetTiltMaxSpeed();

            ptu.VelocityMode(true);

            gz4d::v4l::Capability cap(device);
            stream = std::shared_ptr<gz4d::v4l::Stream>(new gz4d::v4l::Stream(device,cap.getFormats()[0].format,800,600));
            stream->Start();
        }

        void OnIdle()
        {
            bool need_pan = false;
            float pan = 0.0;
            bool need_tilt = false;
            float tilt = 0.0;

            while(!jsq->empty())
            {
                gz4d::ui::Event e = jsq->pop();
                if (e.IsType<gz4d::ui::JoystickPosition>())
                {
                    gz4d::ui::JoystickPosition jp(e);
                    if (jp.source == 0)
                    {
                        pan = -jp.position*pan_max_speed;
                        need_pan = true;
                    }
                    if (jp.source == 1)
                    {
                        tilt = jp.position*tilt_max_speed;
                        need_tilt = true;
                    }
                }
                if (e.IsType<gz4d::ui::JoystickButton>())
                {
                    gz4d::ui::JoystickButton jb(e);
                    if(jb.source == 2 && jb.direction == gz4d::ui::Down)
                    {
                        snapshot = true;
                        gz4d::Time now = gz4d::time::Now();

                        std::ostringstream ss_c,ss_w;

                        ss_c << now << "_canon.jpg";
                        ss_w << now << "_webcam.jpg";

                        camera.Capture(ss_c.str());
                        webcam_image = stream->Read();
                        webcam_image.Save(ss_w.str());

                        
                        canon_image.Load(ss_c.str());
                    }
                }
            }
            if(need_pan && need_tilt)
                ptu.PanTiltSpeed(pan,tilt);
            else
            {
                if(need_pan)
                    ptu.PanSpeed(pan);
                if(need_tilt)
                    ptu.TiltSpeed(tilt);
            }
            boost::thread::yield();
            postRedisplay();
        }

        void OnDisplay()
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawBuffer(GL_BACK);
            glPushMatrix();
            
            gz4d::ImageRGB8 img = stream->Read();

            if(snapshot)
            {
                if(!webcamTexture)
                {
                    webcamTexture.Bind();
                    webcamTexture.Image(webcam_image.GetRaster());
                    webcamTexture.MinFilter(GL_LINEAR);
                    webcamTexture.MagFilter(GL_NEAREST);
                }
                else
                {
                    webcamTexture.Bind();
                    webcamTexture.SubImage(img.GetRaster());
                }

                if(!canonTexture)
                {
                    canonTexture.Bind();
                    canonTexture.Image(canon_image.GetRaster());
                    canonTexture.MinFilter(GL_LINEAR);
                    canonTexture.MagFilter(GL_NEAREST);
                }
                else
                {
                    canonTexture.Bind();
                    canonTexture.SubImage(canon_image.GetRaster());
                }

                snapshot = false;
            }

            if(!previewTexture)
            {
                previewTexture.Bind();
                previewTexture.Image(img.GetRaster());
                previewTexture.MinFilter(GL_LINEAR);
                previewTexture.MagFilter(GL_NEAREST);
            }
            else
            {
                previewTexture.Bind();
                previewTexture.SubImage(img.GetRaster());
            }

            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

            int w = img.GetRaster().GetColumns();
            int h = img.GetRaster().GetRows();
            float wf = 1.0;
            float hf = 1.0;

            if(w/float(h) > width/float(height))
            {
                wf = nwidth;
                hf = wf*h/float(w);
            }
            else
            {
                hf = nheight;
                wf = hf*w/float(h);
            }

            glColor3f(1.0,1.0,1.0);
            glEnable(GL_TEXTURE_2D);

            glBegin(GL_QUADS);

            glTexCoord2f(0.0,1.0);
            glVertex2f(0.0,hf);
            glTexCoord2f(0.0,0.0);
            glVertex2f(0.0,0.0);
            glTexCoord2f(1.0,0.0);
            glVertex2f(wf,0.0);
            glTexCoord2f(1.0,1.0);
            glVertex2f(wf,hf);
            glEnd();

            if(webcamTexture)
            {
                webcamTexture.Bind();
                glBegin(GL_QUADS);
                glTexCoord2f(0.0,1.0);
                glVertex2f(0.0,0.0);
                glTexCoord2f(0.0,0.0);
                glVertex2f(0.0,-hf);
                glTexCoord2f(1.0,0.0);
                glVertex2f(wf,-hf);
                glTexCoord2f(1.0,1.0);
                glVertex2f(wf,0.0);
                glEnd();
            }

            if(canonTexture)
            {
            
                w = canon_image.GetRaster().GetColumns();
                h = canon_image.GetRaster().GetRows();
                wf = 1.0;
                hf = 1.0;
    
                if(w/float(h) > width/float(height))
                {
                    wf = nwidth;
                    hf = wf*h/float(w);
                }
                else
                {
                    hf = nheight;
                    wf = hf*w/float(h);
                }

                canonTexture.Bind();

                glBegin(GL_QUADS);
                glTexCoord2f(0.0,1.0);
                glVertex2f(-wf,0.0);
                glTexCoord2f(0.0,0.0);
                glVertex2f(-wf,-hf);
                glTexCoord2f(1.0,0.0);
                glVertex2f(0.0,-hf);
                glTexCoord2f(1.0,1.0);
                glVertex2f(0.0,0.0);
                glEnd();
            }

            glDisable(GL_TEXTURE_2D);


            glPopMatrix();
            swapBuffers();
        }

        void OnReshape(int w, int h)
        {
            height = h;
            width = w;
            nwidth = 1.0;
            nheight = 1.0;

            if (w > h)
                nwidth = w/float(h);
            else
                nheight = h/float(w);

            glViewport(0, 0, w, h);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(-nwidth,nwidth,-nheight,nheight,-1.0,1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }

        void OnMouse(int button, int state, int x, int y)
        {
        }

        void OnMotion(int x, int y)
        {
        }
};


int main(int argc, char * argv[])
{
    GeoCamera c;
    c.mainLoop();
    return 0;
}
