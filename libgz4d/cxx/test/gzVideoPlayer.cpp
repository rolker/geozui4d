#include<gz4d/Image.h>
#include<gz4d/glo/Texture.h>
#include<gz4d/glut/Application.h>
#include <gz4d/av.h>
#include <iostream>
#include <gz4d/time/ios.h>

class MyScene: public gz4d::glut::Application<MyScene,gz4d::glut::Ortho<> >
{
    private:
        gz4d::ImageRGBA8 image;
        gz4d::glo::Texture texture;
        gz4d::AVReader avr;
        gz4d::AVReader::VideoStream::output_queue_type vq;
        gz4d::Time startTime;

        gz4d::AVReader::VideoFrame currentFrame;

        gz4d::time::Duration videoLength;
        gz4d::AVReader::VideoStream::Ptr vs;

        int mouse_start;
        int mouse_last;
        bool seeking;
        gz4d::time::Duration seek_start;
        gz4d::time::Duration seek_to;
        gz4d::time::Duration last_seek_to;

    public:
        MyScene(std::string const &f)
        :texture(context,GL_TEXTURE_2D,GL_RGBA),seeking(false)
        {
            avr.Open(f);
            avr.DumpFormat();
            if (avr.GetVideoStreams().size())
            {
                vs = avr.GetVideoStreams()[0];
                vq = vs->NewOutQueue();
                image.Resize(vs->GetWidth(),vs->GetHeight());
                InitWindowSize(vs->GetWidth(),vs->GetHeight()*1.1);
                videoLength = vs->GetDuration();
            }
            else
                exit(1);
        }

        void Draw()
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            if(startTime.valid())
            {
                if(!texture)
                {
                    texture.Bind();
                    texture.Image(image.GetRaster());
                }
                else
                {
                    texture.Bind();
                    image = gz4d::ImageRGBA8(currentFrame.frame.GetRaster());
                    texture.SubImage(image.GetRaster());
                }

                glDisable(GL_LIGHTING);

                texture.MinFilter(GL_LINEAR);
                texture.MagFilter(GL_NEAREST);
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_ALPHA_TEST);
                glAlphaFunc(GL_GREATER, 0.0);
                
                int w = image.GetRaster().GetColumns();
                int h = image.GetRaster().GetRows();
                float wf = 1.0;
                float hf = 1.0;
                if (w<h)
                    wf = w/float(h);
                if (h<w)
                    hf = h/float(w);
                float scale = _canvas_width/wf;
                if(_canvas_height/(hf*1.1) < scale)
                    scale = _canvas_height/(hf*1.1);

                wf *= scale;
                hf *= scale;

                glColor3f(1.0,1.0,1.0);
                glEnable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0,0.0);
                glVertex2f(-wf,hf);
                glTexCoord2f(0.0,1.0);
                glVertex2f(-wf,-hf);
                
                glTexCoord2f(1.0,1.0);
                glVertex2f(wf,-hf);
                
                glTexCoord2f(1.0,0.0);
                glVertex2f(wf,hf);
                
                glEnd();
                glDisable(GL_TEXTURE_2D);

                float p = 2.0*(1.0-((videoLength- currentFrame.presentationTime).asSeconds()/videoLength.asSeconds()));

                glColor3f(0.0,1.0,0.0);
                glBegin(GL_QUADS);
                glVertex2f(-wf,-hf);
                glVertex2f(-wf,-hf*1.1);
                glVertex2f(-wf+p*wf,-hf*1.1);
                glVertex2f(-wf+p*wf,-hf);
                glEnd();
            }
            SwapBuffers();
        }

                


        void Idle()
        {
            if(!startTime.valid())
            {
                if(vq->empty())
                    avr.ReadFrame();
                if(!vq->empty())
                {
                    currentFrame = vq->pop();
                    image = gz4d::ImageRGBA8(currentFrame.frame.GetRaster());
                    startTime = gz4d::time::Now();
                    PostRedisplay();
                }
            }
            else
            {
                gz4d::Time now = gz4d::time::Now();

                gz4d::time::Duration lastCurrentTime = currentFrame.presentationTime;

                if(seeking)
                {
                    startTime = now-seek_to;
                    if(seek_to != last_seek_to && (seek_to < currentFrame.presentationTime || seek_to > currentFrame.presentationTime+currentFrame.duration))
                    {
/*                        std::cerr << "before seek: " << currentFrame.presentationTime << std::endl;
                        std::cerr << "doing seek" << seek_to << std::endl;*/
                        //vs->Seek(seek_to);
                        avr.Seek(seek_to);
                        last_seek_to = seek_to;
/*                        while(!vq->empty())
                            currentFrame = vq->pop();
                        std::cerr << "after seek: " << currentFrame.presentationTime << " key?: " << currentFrame.isKeyframe << " convergence: " << currentFrame.convergenceDuration << std::endl;*/
                    }
                }

                gz4d::time::Duration elapsed = now - startTime;
                if(elapsed > videoLength)
                {
                    elapsed = videoLength;
                    startTime = now-videoLength;
                }

                //std::cerr << "elapsed: " << elapsed << " currentFrame: " << currentFrame.presentationTime << std::endl;


                while(!vq->empty())
                    currentFrame = vq->pop();

                if (elapsed > currentFrame.presentationTime+currentFrame.duration)
                {
                        avr.ReadFrame();
                }

                if(lastCurrentTime != currentFrame.presentationTime)
                {
                        PostRedisplay();
                }
            }
        }



        void Mouse(int button, int state,int x, int y)
        {
            if(button == 0)
            {
                if(state == 0)
                {
                    std::cerr << "left down" << std::endl;
                    mouse_start = x;
                    mouse_last = x;
                    seek_start = currentFrame.presentationTime;
                    seeking = true;
                    seek_to = currentFrame.presentationTime;
                    last_seek_to = seek_to;
                }
                else
                {
                    std::cerr << "left up" << std::endl;
                    seeking = false;
                }
            }
        }

        void Motion(int x, int y)
        {
            if(seeking)
            {
                if (x != mouse_last)
                {
                    seek_to = seek_start + videoLength*((x-mouse_start)/float(_window_width));
                    if(seek_to.asSeconds() < 0.0)
                        seek_to = gz4d::seconds(0.0);
                    if(seek_to > videoLength)
                        seek_to = videoLength;
                    std::cerr << "seek to: " << seek_to << std::endl;
//                     vs->Seek(seek_to);
//                     avr.ReadFrame();
//                     avr.ReadFrame();

                    mouse_last = x;
                    PostRedisplay();
                }
            }
        }
};

int main(int argc, char * argv[])
{
    if(argc != 2)
    {
        std::cerr << "usage: gzVideoPlayer infile" << std::endl;
        return 1;
    }
    MyScene s(argv[1]);
    s("Video Player");
    return 0;
}
