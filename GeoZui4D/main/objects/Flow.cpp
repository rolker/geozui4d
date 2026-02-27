/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include"Flow.h"
#include "engine/Scene3D.h"
#include "engine/InputMediator.h"
#include <gz4d/glo/Vertex.h>
#include <gz4d/glo/Color.h>
#include <gz4d/glo/Error.h>
#include <gz4d/vector/ios.h>
#include "MainWindow.h"

extern MainWindow *mainWindow;

REGISTER_OBJECT(Flow, "flow");
extern int dominantEye;

Flow::Flow()
    :loadedTimeSpan(0.0),linewidth(1.0),aa(false),displayTime(0.0),firstDraw(true),haveMultiDraw(false)
{
    loadedParticles.first = particles.end();
    loadedParticles.second = particles.end();
    lastLoadedParticles = loadedParticles;
}

Flow::~Flow()
{
}

void Flow::DrawLowRes(float degradation, Window3D *win)
{
    if(firstDraw)
    {
        GLenum err = glewInit();
        if(err == GLEW_OK)
            if (glewIsSupported("GL_EXT_multi_draw_arrays"))
                haveMultiDraw = true;
    }
    std::vector<float> dataArray;
    if(loadedParticles != lastLoadedParticles)
    {
        dataArray.resize(4*particleCount*(loadedParticles.second-loadedParticles.first));
        int j = 0;
        for (std::vector<TimeStep>::iterator ji = loadedParticles.first; ji != loadedParticles.second; ++ji, ++j)
        {
            for(int i=0; i<particleCount; ++i)
            {
                int ai = i*4*(loadedParticles.second-loadedParticles.first)+j*4;
                dataArray[ai]=((ji->time - loadedParticles.first->time)/loadedTimeSpan).asSeconds();
                dataArray[ai+1] = (*ji->data)[i][0];
                dataArray[ai+2] = (*ji->data)[i][1];
                dataArray[ai+3] = (*ji->data)[i][2];
            }
        }
        glTexCoordPointer(1,GL_FLOAT,4*4,&dataArray[0]);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(3,GL_FLOAT,4*4,&dataArray[1]);
        glEnableClientState(GL_VERTEX_ARRAY);
    }

    glDepthMask(GL_FALSE);
    for(unsigned int c = 0; c < colorGroups.size(); ++c)
    {
        if(colorGroups[c].displayList == 0)
        {
            colorGroups[c].displayList = glGenLists(1);
            colorGroups[c].texture=std::shared_ptr<gz4d::glo::Texture>(new gz4d::glo::Texture(win->getGloContext(),GL_TEXTURE_1D, GL_RGBA8));
            colorGroups[c].texture->Bind();
            colorGroups[c].texture->Image(colorGroups[c].raster);
            colorGroups[c].texture->MinFilter(GL_LINEAR);
            colorGroups[c].texture->MagFilter(GL_LINEAR);
            colorGroups[c].texture->WrapS(GL_CLAMP);
        }
    
        for(int i = 0; i < 2048; ++i)
        {
            double t = loadedParticles.first->time.secondsSinceEpoch().asSeconds() + i*loadedTimeSpan/2048.0;
            if( t < lastIdleTime - displayTime || t > lastIdleTime)
                colorGroups[c].raster.GetRow(0)[i] = gz4d::Color(0.0,0.0,0.0,0.0);
            else
                colorGroups[c].raster.GetRow(0)[i] = colorGroups[c].head.Blend(colorGroups[c].tail,(lastIdleTime-t)/displayTime);
        }
    
        colorGroups[c].texture->Bind();
        colorGroups[c].texture->SubImage(colorGroups[c].raster);

        if(loadedParticles != lastLoadedParticles)
        {
            glNewList(colorGroups[c].displayList,GL_COMPILE);

            glPushAttrib(GL_LINE_BIT|GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.0);

            glLineWidth(linewidth);
            if(aa)
                glEnable(GL_LINE_SMOOTH);
        
            glColor3f(1.0,1.0,1.0);

            if(haveMultiDraw)
            {
                std::vector<GLint> first;
                std::vector<GLsizei> count;
                for(int i=0; i<particleCount; ++i)
                {
                    if (colors[i] == c)
                    {
                        first.push_back(i*(loadedParticles.second-loadedParticles.first));
                        count.push_back(loadedParticles.second-loadedParticles.first);
                    }
                }

                glMultiDrawArraysEXT(GL_LINE_STRIP,&first[0],&count[0],first.size());
            }
            else
            {
                for(int i=0; i<particleCount; ++i)
                {
                    if (colors[i] == c)
                    {
                        glDrawArrays(GL_LINE_STRIP,i*(loadedParticles.second-loadedParticles.first),loadedParticles.second-loadedParticles.first);
                    }
                }
            }
            glPopAttrib();

            glEndList();
        }
    
        glEnable(GL_TEXTURE_1D);
        glCallList(colorGroups[c].displayList);
        glDisable(GL_TEXTURE_1D);
    }
    glDepthMask(GL_TRUE);
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
    for(unsigned int c = 0; c < colorGroups.size(); ++c)
    {
        colorGroups[c].texture->Bind();
        glEnable(GL_TEXTURE_1D);
        glCallList(colorGroups[c].displayList);
        glDisable(GL_TEXTURE_1D);
    }
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    lastLoadedParticles = loadedParticles;
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void Flow::Idle(double time)
{
    load(gz4d::Time(time));
    if (loadedParticles.first != particles.end() && lastIdleTime != time)
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    Object::Idle(time);
}

Object * Flow::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object::LoadXML(xml,path,ncfile);
	
    linewidth = 1.0;

    std::string varName = xml.get<std::string>("<xmlattr>.variable","Drogues");

    variable = ncfile.GetVar(varName);
    if(!variable)
    {
        std::cerr << "Flow: problem with variable\n";
        return 0;
    }

    gz4d::netcdf::Dim coordDim = ncfile.GetDim("Coordinate");
    dimensions = coordDim.Size();

    gz4d::netcdf::Dim timedim = ncfile.GetDim("Time");

    gz4d::netcdf::Var timesvar = ncfile.GetVar("Time");
    if(timesvar)
    {
        std::vector<double> t(timedim.Size());
        timesvar.Get(t);
        for(std::vector<double>::iterator i = t.begin(); i != t.end(); ++i)
            particles.push_back(TimeStep(gz4d::Time(*i)));
    }
    else
    {
        double qlength = variable.GetAtt("Timestep").As<double>(0);
        for(long i = 0; i < timedim.Size(); ++i)
            particles.push_back(TimeStep(gz4d::Time(i*qlength)));
    }
    loadedParticles.first = particles.end();
    loadedParticles.second = particles.end();
    lastLoadedParticles = loadedParticles;

    displayTime = (particles[1].time - particles[0].time).asSeconds();

    bbox.expandTime(particles.begin()->time.secondsSinceEpoch().asSeconds(),particles.rbegin()->time.secondsSinceEpoch().asSeconds());

    // set the georef point
    Point3d gr;
    gr[2] = 0.0;
    variable.Get(gr,1,1,dimensions);
    SetPosition(gr);

    particleCount = ncfile.GetDim("Count").Size();

    load(particles[0].time);

    linewidth = xml.get("<xmlattr>.linewidth",linewidth);

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.antialias"))
        aa = gz4d::IsTrue(*attribute);

    displayTime = xml.get("<xmlattr>.displayTime",displayTime);

    colorGroups.resize(1);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.headcolor"))
    {
        try
        {
            std::stringstream ss(*attribute);
            ss >> colorGroups[0].head;
        }
        catch (gz4d::ExtractionException)
        {
        }
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.tailcolor"))
    {
        try
        {
            std::stringstream ss(*attribute);
            ss >> colorGroups[0].tail;
        }
        catch (gz4d::ExtractionException)
        {
        }
    }

    colors.insert(colors.end(),particleCount,0);

    //bbox stuff
    double n,s,e,w,t,b;
    int step = 10;
    n=s=e=w=t=b=0.0;

    // now, add the bboxs or highlights
    for(bpt::ptree::const_iterator node = xml.begin(); node != xml.end(); ++node)
    {
        if(node->first =="BBox" || node->first == "Highlight")
        {
            unsigned int ci = colorGroups.size();
            colorGroups.resize(ci+1);
            if(attribute = node->second.get_optional<std::string>("<xmlattr>.headcolor"))
            {
                try
                {
                    std::stringstream ss(*attribute);
                    ss >> colorGroups[ci].head;
                }
                catch (gz4d::ExtractionException)
                {
                }
            }
            
            if(attribute = node->second.get_optional<std::string>("<xmlattr>.tailcolor"))
            {
                try
                {
                    std::stringstream ss(*attribute);
                    ss >> colorGroups[ci].tail;
                }
                catch (gz4d::ExtractionException)
                {
                }
            }
            if(node->first =="BBox")
            {
                t=b=0.0;
                std::string bname;
                
                
                bname = node->second.get("<xmlattr>.name",bname);
                n = node->second.get("<xmlattr>.north",n);
                s = node->second.get("<xmlattr>.south",s);
                e = node->second.get("<xmlattr>.east",e);
                w = node->second.get("<xmlattr>.west",w);
                t = node->second.get("<xmlattr>.top",t);
                b = node->second.get("<xmlattr>.bottom",b);
                
                
                e -= gr[0];
                w -= gr[0];
                n -= gr[1];
                s -= gr[1];
                t -= gr[2];
                b -= gr[2];
                AddBBox(bname,n,s,e,w,t,b,ci);
            }
            //highlight
            if(node->first == "Highlight")
            {
                step = node->second.get("<xmlattr>.step",step);
                AddStep(step, ci);
            }

        }
    }
    return this;
}


void Flow::AddBBox(std::string bname,double n,double s,double e,double w,double t,double b, unsigned int cg)
{
    for (std::vector<gz4d::Vector<float,3> >::iterator i = particles[0].data->begin(); i != particles[0].data->end(); ++i)
    {
        if( ((*i)[0] >= w && (*i)[0] <= e) && ((*i)[1] >= s && (*i)[1] <= n) && ((*i)[2] >= b && (*i)[2] <= t))
            colors[i-particles[0].data->begin()] = cg;
    }
}

/// Allows every nth particle to have a different color
void Flow::AddStep(int step, unsigned int cg)
{
    int i;

    for(i=0;i<particleCount;i+=step)
    {
        colors[i]=cg;
    }
}

void Flow::load(gz4d::Time timestep)
{
    loadedParticles.first = particles.end();
    //std::cerr << gz4d::Time(timestep-displayTime) << " to " << timestep << std::endl;

    for(std::vector<TimeStep>::iterator ts = particles.begin(); ts != particles.end(); ++ts)
    {
        if(ts->time < timestep-gz4d::time::Duration(displayTime) && ts+1 != particles.end() && (ts+1)->time <= timestep-gz4d::time::Duration(displayTime)) // are we before desired timespan?
        {
            ts->data.reset();
        }
        else if (ts->time > timestep && ts != particles.begin() && (ts-1)->time >= timestep) // are we after timespan of interest?
        {
            ts->data.reset();
        }
        else
        {
            if (loadedParticles.first == particles.end())
                loadedParticles.first = ts;
            loadedTimeSpan = (ts->time - loadedParticles.first->time).asSeconds();
            loadedParticles.second = ts+1;
            if (!ts->data)
            {
                ts->data = std::shared_ptr<std::vector<gz4d::Vector<float,3> > >(new std::vector<gz4d::Vector<float,3> >(particleCount));
                std::vector<double> d(particleCount*dimensions);
                std::size_t tindex = ts-particles.begin();
                variable.SetCur(tindex,0,0);
                variable.Get(d,1,particleCount,dimensions);
                gz4d::Point<double> ref = GetPosition();
                for(int i = 0; i < particleCount; ++i)
                {
                    for(int j = 0; j < dimensions; ++j)
                        (*ts->data)[i][j] = d[i*dimensions+j]-ref[j];
                    if (dimensions < 3)
                        (*ts->data)[i][2] = -ref[2];
                }
            }
        }
    }
}
