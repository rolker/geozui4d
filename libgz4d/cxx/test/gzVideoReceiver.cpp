// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gz4d/Image.h>
#include<gz4d/glo/Texture.h>
#include<gz4d/glut/Application.h>
#include <gz4d/socket/TCPSocket.h>

class MyScene: public gz4d::glut::Application<MyScene,gz4d::glut::Ortho<> >
{
    private:
        gz4d::ImageRGBA8 image;
        gz4d::ImageL8 bw_image;
        gz4d::glo::Texture texture;
        gz4d::TCPSocket server;
        gz4d::Socket client;
        gz4d::IPSocket::Address clientAddress;
    public:
        MyScene(int w, int h, int port)
        :texture(context,GL_TEXTURE_2D,GL_RGBA)
        {
            image.Resize(w,h);
            bw_image.Resize(w,h);
            //image.Load(f);
            server.Bind(gz4d::IPSocket::Address("ANY",port));
            server.Listen(1);
            client = server.Accept(clientAddress);
            InitWindowSize(w,h);
        }

        void Draw()
        {
            gz4d::glo::Context::Active::Token c = context->activate();
            
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            if(!texture)
            {
                texture.Bind();
                texture.Image(image.GetRaster());
            }
            else
            {
                texture.Bind();
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
            SwapBuffers();
        }
        
        void Idle()
        {
            int image_size = image.GetRaster().GetColumns()*image.GetRaster().GetRows();
            int index = 0;
            std::vector<uint8_t> data(image_size);
            while(index < image_size)
            {
                client.Recv(data);
                for (int i = 0; i < data.size(); ++i)
                    reinterpret_cast<uint8_t*>(bw_image.GetRaster().raw())[index+i]=data[i];
                index += data.size();
            }
            image = gz4d::ImageRGBA8(bw_image.GetRaster());
            PostRedisplay();
        }
};

int main(int argc, char * argv[])
{
    if(argc != 4)
    {
        std::cerr << "usage: gzVideoReceiver width height tcp_port" << std::endl;
        return 1;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int port = atoi(argv[3]);
    MyScene s(width,height,port);
    s("Video Receiver");
    return 0;
}
