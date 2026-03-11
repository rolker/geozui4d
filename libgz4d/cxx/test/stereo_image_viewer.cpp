// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/glut/Application.h>
#include <gz4d/Image.h>
#include <gz4d/image/io.h>
#include <gz4d/glo/Texture.h>
#include <vector>
#include <string>


class StereoViewer: public gz4d::glut::Application<StereoViewer>
{
    private:
        std::vector<std::shared_ptr<gz4d::glo::Texture> > textures;
	std::vector<std::shared_ptr<gz4d::ImageRGB8> > images;
	int left,right;
        int width;
        int height;
	bool mirror;

    public:
        StereoViewer(std::vector<std::string> const &fnames)
    : left(0),right(1),width(1280),height(1024),mirror(false)
        {
            InitDisplayMode(GLUT_STEREO);
            InitWindowSize(1280,1024);
	    for(std::vector<std::string>::const_iterator f = fnames.begin(); f != fnames.end(); ++f)
	    {
		std::shared_ptr<gz4d::ImageRGB8> i(new gz4d::ImageRGB8);
		std::cout << "loading: " << *f << std::endl;
		i->Load(*f);
		images.push_back(i);
		std::shared_ptr<gz4d::glo::Texture> t(new gz4d::glo::Texture(context,GL_TEXTURE_2D, GL_RGB8));
		textures.push_back(t);
	    }
        }
        
        void Reshape(int w, int h)
        {
            height = h;
            width = w;
            glViewport(0, 0, w, h);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(-1.0,1.0,-1.0,1.0,-1.0,1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        
        }
        
        void Draw()
        {
            glDrawBuffer(GL_FRONT);
            glClear(GL_COLOR_BUFFER_BIT);

            glDrawBuffer(GL_LEFT);
	    DrawEye(left);

            glDrawBuffer(GL_RIGHT);
	    DrawEye(right);
	    
	    SwapBuffers();
	}
	
	void DrawEye(int i)
	{
            if(!(*textures[i]))
            {
                textures[i]->Bind();
                textures[i]->Image(images[i]->GetRaster());
            }
            else
                textures[i]->Bind();

            textures[i]->MinFilter(GL_LINEAR);
            textures[i]->MagFilter(GL_NEAREST);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

            int w = images[i]->GetRaster().GetColumns();
            int h = images[i]->GetRaster().GetRows();

            float ir = w/float(h);
            float sr = width/float(height);

            float wf = 1.0;
            float hf = 1.0;

            if (ir > sr)
                hf = sr/ir;
            if (sr > ir)
                wf = ir/sr;

	    if(mirror)
		wf = -wf;

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

        }

        void Keyboard(unsigned char k, int x, int y)
        {
            switch(k)
            {
		case '1':
		    ++left;
		    if(left >= textures.size())
			left = textures.size()-1;
		    PostRedisplay();
		    break;
		case '2':
		    --left;
		    if(left < 0)
			left = 0;
		    PostRedisplay();
		    break;

		case '3':
		    ++right;
		    if(right >= textures.size())
			right = textures.size()-1;
		    PostRedisplay();
		    break;
		case '4':
		    --right;
		    if(right < 0)
			right = 0;
		    PostRedisplay();
		    break;

                case 'f':
		    Fullscreen();
                    break;
		case 'm':
		    mirror = !mirror;
		    PostRedisplay();
		    break;
                default:
                    break;
            }
        }


};

int main(int argc, char * argv[])
{
    if(argc < 3)
    {
        std::cerr << "usage: stereo_image_viewer image1 image2 ..." << std::endl;
        exit(1);
    }
    std::vector<std::string> images;
    for(int i = 1; i < argc; ++i)
	images.push_back(argv[i]);
	
    StereoViewer sv(images);
    sv("Stereo Viewer");
    return 0;
}
