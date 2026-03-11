// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "Srtm.h"
#include <filesystem>
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include <gz4d/geo/Projection.h>
#include "MainWindow.h"
#include "engine/Scene3D.h"

extern MainWindow * mainWindow;


REGISTER_OBJECT(Srtm,"srtm");

Srtm::Srtm()
{
    data = NULL;
    normals = NULL;
    positions = NULL;
    binaryFile = true;
    dataCount = 0;
    hiresRow = 0;
    north = 0.0;
    west = 0.0;
    cellsize = 0.00833333;
}

Srtm::~Srtm()
{
    delete[] data;
    delete[] normals;
}

void Srtm::DrawLowRes(float degradation, Window3D *win)
{
    if(data)
    {
        unsigned short skip = (unsigned short)(1.0+degradation*25.0);
        unsigned int ps = (unsigned int)(win->getViewInfo()->getPixelSize()/1000.0);
        if (ps > skip)
                skip = ps;
        if (skip > 35)
            skip = 35;
        if (win->isRecording() && skip < 4)
            skip = 4;
        std::cerr << "skip: " << skip << " ps: " << ps << "\n";
        EnableTexture();
        glColor3f(1.0,1.0,1.0);
        for(int i=0;i+skip<6000;i+=skip)
        {
            glBegin(GL_QUAD_STRIP);
            for(int j = 0; j < 4800; j+=skip)
            {
                glNormal3sv(&normals[(i*4800+j)*3]);
                glVertex3f(positions[(i*4800+j)*2],positions[(i*4800+j)*2+1],data[i*4800+j]);
                glNormal3sv(&normals[((i+skip)*4800+j)*3]);
                glVertex3f(positions[((i+skip)*4800+j)*2],positions[((i+skip)*4800+j)*2+1],data[(i+skip)*4800+j]);
                if (j+skip >= 4800 && j < 4800-1)
                {
                    glNormal3sv(&normals[(i*4800+4800-1)*3]);
                    glVertex3f(positions[(i*4800+4800-1)*2],positions[(i*4800+4800-1)*2+1],data[i*4800+4800-1]);
                    glNormal3sv(&normals[((i+skip)*4800+4800-1)*3]);
                    glVertex3f(positions[((i+skip)*4800+4800-1)*2],positions[((i+skip)*4800+4800-1)*2+1],data[(i+skip)*4800+4800-1]);
                }
            }
            glEnd();
            if(i+skip+skip >= 6000)
            {
                glBegin(GL_QUAD_STRIP);
                for(int j = 0; j < 4800; j+=skip)
                {
                    glNormal3sv(&normals[((i+skip)*4800+j)*3]);
                    glVertex3f(positions[((i+skip)*4800+j)*2],positions[((i+skip)*4800+j)*2+1],data[(i+skip)*4800+j]);
                    glNormal3sv(&normals[((6000-1)*4800+j)*3]);
                    glVertex3f(positions[((6000-1)*4800+j)*2],positions[((6000-1)*4800+j)*2+1],data[(6000-1)*4800+j]);
                    if (j+skip >= 4800 && j < 4800-1)
                    {
                        glNormal3sv(&normals[((i+skip)*4800+4800-1)*3]);
                        glVertex3f(positions[((i+skip)*4800+4800-1)*2],positions[((i+skip)*4800+4800-1)*2+1],data[(i+skip)*4800+4800-1]);
                        glNormal3sv(&normals[((6000-1)*4800+4800-1)*3]);
                        glVertex3f(positions[((6000-1)*4800+4800-1)*2],positions[((6000-1)*4800+4800-1)*2+1],data[(6000-1)*4800+4800-1]);
                    }
                }
                glEnd();
            }
        }
        DisableTexture();
        hiresRow = 0;
    }
}

int Srtm::DrawHighRes(int eye, Window3D *win)
{
    if (!doneFlag)
    {
        EnableTexture();
        glColor3f(1.0,1.0,1.0);
        glBegin(GL_QUAD_STRIP);
        for(int j = 0; j < 4800; j++)
        {
            glNormal3sv(&normals[(hiresRow*4800+j)*3]);
            glVertex3f(positions[(hiresRow*4800+j)*2],positions[(hiresRow*4800+j)*2+1],data[hiresRow*4800+j]);
            glNormal3sv(&normals[((hiresRow+1)*4800+j)*3]);
            glVertex3f(positions[((hiresRow+1)*4800+j)*2],positions[((hiresRow+1)*4800+j)*2+1],data[(hiresRow+1)*4800+j]);
        }
        glEnd();
        DisableTexture();
        hiresRow++;
        doneFlag = (hiresRow >= 6000-1);
    }
    return doneFlag;
}

int Srtm::Load()
{
    int ret = Open();
    if (!ret)
    {
        assert(sizeof(short) == 2);

        fseek(in,0,SEEK_END);
        long filesize = ftell(in);
        fseek(in,0,SEEK_SET);
        dataCount = filesize/sizeof(short);

        data = new short[dataCount];
        if(fread(data,sizeof(short),dataCount,in)!= (unsigned int)dataCount)
        {
            Close();
            return 1;
        }

        Close();

        //endian check
        bool bigendian = true;
        int i = 1;
        if ((char*)(&i)[0])
            bigendian = false;

        if(!bigendian)
        {
            for (long i=0;i<dataCount;i++)
            {
                data[i] = ((data[i]&0xff00) >>8) | ((data[i]&0x00ff)<<8);
            }
        }

        // figure out georef
        std::string fname = GetFilenameAsPath().filename().string();
        west = atof(&fname.c_str()[1]);
        if (fname[0] == 'w')
            west = -west;
        int nindex = fname.find_first_of("ns");
        float north = atof(&fname.c_str()[nindex+1]);
        if (fname[nindex]== 's')
            north = -north;

        std::cerr << "north:" << north << "\twest: " << west << "\n";

        std::cerr << "Projecting...";

        autoProjection(west+cellsize*2400.0,north);
        gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
        gz4d::Point<double> geo = proj(north,west);
        SetPosition(geo[0],geo[1],0.0);

        positions = new float[dataCount*2];

        for(int i=0;i<6000;i++)
        {
            for(int j = 0; j < 4800; j++)
            {
                float lon = west+j*cellsize;
                float lat = north-i*cellsize;
                gz4d::Point<double> xy = proj(lat,lon);
                positions[(i*4800+j)*2]=xy[0]-geo[0];
                positions[(i*4800+j)*2+1]=xy[1]-geo[1];
            }
        }

        std::cerr << "done!" << std::endl;

        std::cerr << "Calculating normals...";
        normals = new short[dataCount*3];
        for(int i=0;i<6000-1;i++)
        {
            for(int j = 0; j < 4800-1; j++)
            {
                short v1 = data[i*4800+j];
                short v2 = data[(i+1)*4800+j];
                short v3 = data[i*4800+j+1];

                float dhy = (v1 - v2);
                float dhx = (v1 - v3);

                float l = (float)sqrt(dhx*dhx + dhy*dhy + 1000*1000);

                dhx /= l;
                dhy /= l;
                float dhz = 1000/l;

                normals[(i*4800+j)*3] = (GLshort)(32767*dhx);
                normals[(i*4800+j)*3+1] = (GLshort)(32767*dhy);
                normals[(i*4800+j)*3+2] = (GLshort)(32767*dhz);
            }
            normals[(i*4800+4800-1)*3] = normals[(i*4800+4800-2)*3];
            normals[(i*4800+4800-1)*3] = normals[(i*4800+4800-2)*3];
            normals[(i*4800+4800-1)*3] = normals[(i*4800+4800-2)*3];
        }
        for(int j = 0; j < 4800; j++)
        {
            normals[((6000-1)*4800+j)*3] = normals[((6000-2)*4800+j)*3];
            normals[((6000-1)*4800+j)*3+1] = normals[((6000-2)*4800+j)*3+1];
            normals[((6000-1)*4800+j)*3+2] = normals[((6000-2)*4800+j)*3+2];
        }
        std::cerr << "done!" << std::endl;

    }
    return ret;
}

