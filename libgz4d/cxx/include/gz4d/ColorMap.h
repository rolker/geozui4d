#ifndef GZ4D_COLORMAP_H
#define GZ4D_COLORMAP_H


#include <map>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

#include <gz4d/Color.h>

namespace gz4d
{
    /// \ingroup base
    template <std::size_t N>
    class ColorMap
    {
        typedef std::map<double,Color<N> > Range;
        std::vector<Range> ranges;
        Color<N> defaultColor;
        double scale;
        double offset;

        public:
            ColorMap():scale(1.0),offset(0.0){}
            ColorMap(boost::property_tree::ptree const &cm);
            ColorMap(ColorMap<N> const &cm);

            void Append(boost::property_tree::ptree const &cm);
            void SetScale(double s){scale = s;}
            void SetOffset(double o){offset = o;}

            Color<N> operator[](double value);

            std::string Debug();
    };
    
    template <std::size_t N>
    inline ColorMap<N>::ColorMap(boost::property_tree::ptree const &cm)
    :scale(1.0),offset(0.0)
    {
        for(std::size_t i = 0; i < N; ++i)
        {
            if(i < 3)
                defaultColor[i] = 0.0;
            else
                defaultColor[i] = 1.0;
        }
        Append(cm);
    }

    template <std::size_t N>
            inline ColorMap<N>::ColorMap(gz4d::ColorMap<N> const &cm)
    :ranges(cm.ranges),defaultColor(cm.defaultColor),scale(cm.scale),offset(cm.offset)
    {
    }


    template <std::size_t N>
    inline void ColorMap<N>::Append(boost::property_tree::ptree const &cm)
    {
        Color<N> lastColor = defaultColor.Update(cm);
        for(boost::property_tree::ptree::const_iterator r = cm.begin(); r != cm.end(); ++r)
        {
            if (r->first == "Range")
            {
                ranges.push_back(Range());
                for(boost::property_tree::ptree::const_iterator v = r->second.begin(); v != r->second.end(); ++v)
                {
                    double val = boost::lexical_cast<double>(v->second.data());
                    ranges.back()[val] = lastColor.Update(v->second);
                }
            }
        }

    }

    template <std::size_t N>
            inline  Color<N> ColorMap<N>::operator[](double value)
    {
        double val = (value+offset)*scale;
        Color<N> ret = defaultColor;
        for(std::vector<ColorMap<N>::Range>::iterator r = ranges.begin(); r != ranges.end(); ++r)
        {
            Range::iterator next = r->lower_bound(val);
            if (next != r->end())
            {
                if (next->first == val)
                    ret = ret.Blend(next->second);
                else
                    if (next != r->begin())
                {
                    Range::iterator prev = next;
                    --prev;
                    ret = ret.Blend(prev->second.Blend(next->second,(val-prev->first)/(next->first-prev->first)));
                }
            }
        }
        return ret;
    }

    template <std::size_t N>
            inline std::string ColorMap<N>::Debug()
    {
        std::stringstream ret;
        ret << ranges.size() << " ranges\n";
        for(std::vector<Range>::iterator r = ranges.begin(); r != ranges.end(); ++r)
        {
            for(Range::iterator i = r->begin(); i != r->end(); ++i)
                ret << "  " << i->first << "\t" << i->second << "\n";
            ret << "\n";
        }
        return ret.str();
    }

}

#endif
