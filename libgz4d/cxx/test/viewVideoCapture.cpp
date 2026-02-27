#include<gz4d/Image.h>
#include<gz4d/image/io.h>
#include<gz4d/glo/Texture.h>
#include<gz4d/glut/Application.h>
#include<gz4d/v4l/Stream.h>
#include<gz4d/v4l/Capability.h>

class MyScene: public gz4d::glut::Application<MyScene,gz4d::glut::Ortho<> >
{
    private:
        gz4d::glo::Texture texture;
        gz4d::v4l::Device device;
        std::shared_ptr<gz4d::v4l::Stream> stream;
        //std::shared_ptr<gz4d::Canvas> canvas;
        gz4d::v4l::Stream::output_queue_type queue;

    public:
        MyScene(std::string const &f)
            :texture(context,GL_TEXTURE_2D,GL_RGB),device(f)
        {
            gz4d::v4l::Capability cap(device);
            stream = std::shared_ptr<gz4d::v4l::Stream>(new gz4d::v4l::Stream(device,cap.getFormats()[0].format,800,600));
            stream->Start();
            queue = stream->NewOutQueue();
        }

        void Idle()
        {
            PostRedisplay();
        }
        
        void Draw()
        {
            glClear(GL_COLOR_BUFFER_BIT);
            gz4d::binary::Wrapper<gz4d::binary::Image::Frame> img = queue->pop();
            while(!queue->empty())
                img = queue->pop();

            gz4d::ImageRGB8::raster_type r(gz4d::toImage(*img).GetRaster());
            if(!texture)
            {
                texture.Bind();
                texture.Image(r);
            }
            else
            {
                texture.Bind();
                texture.SubImage(r);
            }

            texture.MinFilter(GL_LINEAR);
            texture.MagFilter(GL_NEAREST);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

            int w = img->width;
            int h = img->height;
            float wf = 1.0;
            float hf = 1.0;
            if (w<h)
                wf = w/float(h);
            if (h<w)
                hf = h/float(w);

            glColor3f(1.0,1.0,1.0);
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0,1.0);
            glVertex2f(-wf,hf);
            glTexCoord2f(0.0,0.0);
            glVertex2f(-wf,-hf);

            glTexCoord2f(1.0,0.0);
            glVertex2f(wf,-hf);

            glTexCoord2f(1.0,1.0);
            glVertex2f(wf,hf);

            glEnd();
            glDisable(GL_TEXTURE_2D);
            SwapBuffers();
        }
};

int main(int argc, char * argv[])
{
    MyScene s(argv[1]);
    s("Video");
    return 0;
}
