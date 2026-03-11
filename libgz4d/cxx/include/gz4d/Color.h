// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_COLOR_H
#define GZ4D_COLOR_H

#include <gz4d/Vector.h>
#include <boost/property_tree/ptree.hpp>

namespace gz4d
{
    /// \ingroup base
    class Color : public Vector<float,4>
    {
        Color(Vector<float,4> const &c) :Vector<float,4>(c) {}
        public:
            Color() :Vector<float,4>(1.0) {}
            Color(const boost::property_tree::ptree &e) :Vector<float,4>(1.0) {Update(e);}
            Color(float r, float g, float b, float a=1.0) :Vector<float,4>(r,g,b,a){}

            Color(Color const &c) :Vector<float,4>(c) {}

            Color const &Update(const boost::property_tree::ptree &e);

            /// Blends incoming color using incoming's alpha value.
            Color Blend(Color const &c);

            /// Blends incoming color using p as weight, ignoring alpha for blending.
            Color Blend(Color const &c, double p);
            using Vector<float,4>::operator=;
            
            Color operator*(float f) const {return Vector<float,4>::operator*(f);}
            Color operator+(Color const &other) const {return Vector<float,4>::operator+(other);}
    };

    inline Color const &Color::Update(const boost::property_tree::ptree &e)
    {
        boost::property_tree::ptree::const_assoc_iterator a = e.find("<xmlattr>");
        if (a != e.not_found())
        {
            values[0] = a->second.get("r",values[0]);
            values[1] = a->second.get("g",values[1]);
            values[2] = a->second.get("b",values[2]);
            values[3] = a->second.get("a",values[3]);
        }
        return *this;
    }

    inline Color Color::Blend(Color const &c)
    {
        float origA = values[3];
        Color ret = Blend(c,c[3]);
        ret[3] = std::max(origA,c[3]);
        return ret;
    }

    inline Color Color::Blend(Color const &c, double p)
    {
        Color ret;
        double ip = 1.0-p;
        for(std::size_t i = 0; i < 4; ++i)
            ret[i] = this->values[i]*ip+c[i]*p;
        return ret;
    }

    inline Color Black(){return Color(0.0,0.0,0.0,1.0);}
    inline Color White(){return Color(1.0,1.0,1.0,1.0);}
    inline Color Grey(float g=0.5){return Color(g,g,g,1.0);}
    inline Color Red(float r=1.0){return Color(r,0.0,0.0,1.0);}
    inline Color Green(float g=1.0){return Color(0.0,g,0.0,1.0);}
    inline Color Blue(float b=1.0){return Color(0.0,0.0,b,1.0);}
}

#endif
