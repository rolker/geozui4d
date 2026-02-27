#include<gz4d/Image.h>
#include<gz4d/glo/Texture.h>
#include<gz4d/glut/Application.h>
#include<gz4d/image/io.h>

class MyScene: public gz4d::glut::Application<MyScene,gz4d::glut::Ortho<> >
{
    private:
        gz4d::ImageRGBA8 image;
        gz4d::glo::Texture texture;
        int w,h;
    public:
        MyScene(std::string const &f)
            :texture(context,GL_TEXTURE_2D,GL_RGBA)
        {
            image.Load(f);
            w = image.width();
            h = image.height();
        }

        void Draw()
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            texture.Bind();
            if(image)
            {
                texture.Image(image.GetRaster());
                image.reset();
            }

            glDisable(GL_LIGHTING);

            texture.MinFilter(GL_LINEAR);
            texture.MagFilter(GL_NEAREST);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.0);

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
    s("Image");
    return 0;
}
