#include "GZincludes.h"
#include "Chart.h"
#include <filesystem>
#include <gz4d/Image.h>
#include<string.h>
#include"objects/gutm.h"
#include<math.h>
#include"engine/PosableStateBase.h"


using namespace std;

REGISTER_OBJECT(Chart,"");

extern int dominantEye;

extern MainWindow * mainWindow;

Chart::Chart()
{
	rows = 0;
	columns = 0;
}

Chart::~Chart()
{
}

Object * Chart::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
	Point3d gr;
	
	Object::LoadXML(xml,path, ncfile);
	
	fs::path ifname;
	fs::path wfname;
	
	FILE * wfile;
	float eastscale, northscale, rot1, rot2;
	double eastref, northref;
	
	//TIFF * tif;
	
	// get filenames from gzx file
        ifname = xml.get("<xmlattr>.imagefile",ifname);
        wfname = xml.get("<xmlattr>.worldfile",wfname);
	
	// get world file info
	if(!wfname.empty())
	{
		wfile = fopen((fs::path(path)/wfname).string().c_str(),"r");
		if(wfile)
		{
			int i = fscanf(wfile, "%f %f %f %f %lf %lf", &eastscale, &rot1, &rot2, &northscale, &eastref, &northref);
			if(i == 6 || rot1 == 0 || rot2 == 0)	// must have all 6 entries, and 0 rots
			{
				gr[0] = eastref;
				gr[1] = northref;
				gr[2] = 0.0;
				SetPosition(gr);
			}
			else
			{
				cerr << "Chart::LoadXML - world file value error\n";
			}
			fclose(wfile);
		}
		else
		{
			cerr << "Chart::LoadXML - world file load error\n";
			return NULL;
		}
	}
	else
	{
		cerr << "Chart::LoadXML - world file filename error\n";
		return NULL;
	}

    image.Load((path/ifname).string());


    columns = image.GetRaster().GetColumns();
    rows = image.GetRaster().GetRows();

    if(eastscale < 0)
    {
        bbox.e=eastref;
        bbox.w= bbox.e + columns*eastscale;
        xstep = eastscale;
    }
    else
    {
        bbox.w=eastref;
        bbox.e= bbox.w + columns*eastscale;
        xstep = eastscale;
    }

    if(northscale < 0)
    {
        bbox.n = northref;
        bbox.s = bbox.n + rows*northscale;
        ystep = northscale;
    }
    else
    {
        bbox.s = northref;
        bbox.n = bbox.s + rows*northscale;
        ystep = northscale;
    }

    bbox.t = bbox.b = 0.0;

	return this;
}

void Chart::DrawLowRes(float degradation, Window3D * win)
{
    glPushAttrib(GL_COLOR_BUFFER_BIT);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    
    if(!texture)
    {
        texture = std::shared_ptr<gz4d::glo::Texture>(new gz4d::glo::Texture(win->getGloContext()));
        texture->Bind();
        texture->Image(image.GetRaster());
    }
    else
        texture->Bind();
    
    texture->MinFilter(GL_NEAREST);
    texture->MagFilter(GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
    
    glColor3f(1.0,1.0,1.0);

    glBegin(GL_QUADS);
    glNormal3d(0.0,0.0,1.0);
    
    glTexCoord2d(0.0,0.0);
    glVertex3f(0.0, ystep*rows, 0.0);

    glTexCoord2d(0.0,1.0);
    glVertex3f(0.0,0.0,0.0);

    glTexCoord2d(1.0,1.0);
    glVertex3f(xstep*columns, 0.0, 0.0);

    glTexCoord2d(1.0,0.0);
    glVertex3f(xstep*columns,ystep*rows,0.0);
    glEnd();

    texture->Unbind();
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();

}


void Chart::Fuse(Object * other)
{
	Gutm * Other;
	if(other == this)
		return;

	if(!other || other->getRegistrationEntry()->className != "Gutm")
		return;

	Other = ((Gutm *)other);
	if(GetBoundingBox().overlapps(Other->GetBoundingBox()))
	{
            gz4d::Pixel<gz4d::RGBA8> blank;
            blank[0] = 0;
            blank[1] = 0;
            blank[2] = 0;
            blank[3] = 0;
            for(unsigned int i = 0; i < columns; ++i)
                for(unsigned int j = 0; j < rows; ++j)
                {
                    if(Other->HasValueInRange(bbox.w + i*xstep, bbox.s + j*ystep, bbox.w + (i+1)*xstep, bbox.s + (j+1)*ystep))
                        image.GetRaster().GetRow(j)[i] = blank;
                }
        texture->SubImage(image);
    }
}

double Chart::getResolution()
{
	if(xstep>ystep)
		return xstep;
	else
		return ystep;
}
