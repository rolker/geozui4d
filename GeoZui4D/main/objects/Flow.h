/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef FLOW_H
#define FLOW_H

#include"engine/Object.h"
#include <gz4d/TimeSequence.h>
#include <gz4d/Vector.h>
#include <gz4d/Color.h>
#include <gz4d/data/Raster.h>
#include <gz4d/glo/Texture.h>
#include <gz4d/Pixel.h>

class NcDim;

class Flow : public Object
{
    struct ColorGroup;
    public:
        Flow();
        ~Flow();
    
        DERIVE_OBJECT(Flow);
        //Display-related functions
        void DrawLowRes(float degradation, Window3D *win);
    
        virtual void Idle(double time);
    
        Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    
        void AddBBox(std::string bname,double n,double s,double e,double w,double t,double b, unsigned int cg);
        void AddStep(int step, unsigned int cg);
    private:
        void load(gz4d::Time timestep);

        struct TimeStep
        {
            gz4d::Time time;
            std::shared_ptr<std::vector<gz4d::Vector<float,3> > > data;

            TimeStep(gz4d::Time t):time(t){}
            TimeStep(){}
        };

        std::vector<TimeStep> particles;
        std::pair<std::vector<TimeStep>::iterator, std::vector<TimeStep>::iterator> loadedParticles;
        std::pair<std::vector<TimeStep>::iterator, std::vector<TimeStep>::iterator> lastLoadedParticles;

        double loadedTimeSpan;

        long particleCount;
        int dimensions; ///< 2d or 3d?

        gz4d::netcdf::Var variable;

        // the fading color of a particle
        struct ColorGroup
        {
                gz4d::Color head;
                gz4d::Color tail;
                gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> > raster;
                std::shared_ptr<gz4d::glo::Texture> texture;
                GLuint displayList;
                ColorGroup():displayList(0){raster.Resize(2048,1);}
        };

        // array of ptrs to color groups, one per particle
        std::vector<unsigned int> colors;
        std::vector<ColorGroup> colorGroups;

        float linewidth;
        bool aa;
        double displayTime; ///< The amount of time the particle is displayed.
        bool firstDraw;
        bool haveMultiDraw;
};


#endif
