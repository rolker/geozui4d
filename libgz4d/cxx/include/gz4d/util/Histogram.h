#ifndef GZ4D_UTIL_HISTOGRAM_H
#define GZ4D_UTIL_HISTOGRAM_H

#include <gz4d/gz4d.h>
#include <algorithm>
#include <ostream>
#include <map>
#include <boost/iterator/transform_iterator.hpp>

namespace gz4d
{
    
    /// Collects values into a histogram.
    class Histogram
    {
        typedef std::map<int,int> HistogramType;
        HistogramType _histogram; ///< map of bins in which counts are collected.
        double _resolution; ///< step size for the bins.
        
        int _max_count; ///< keep track of the largest bin size.
        int _print_size; ///< width used to scale output when using << operator.

        public:
            typedef std::pair<double,int> value_type;
            
        private:
            struct IterTransformer
            {
                typedef value_type result_type;
                double _resolution;
                IterTransformer(double r):_resolution(r){}
                value_type operator()(HistogramType::value_type const &i) const
                {
                    return value_type(i.first*_resolution,i.second);
                }
            };
            
        public:
            /// Create an empty histogram.
            /// @param resolution bin size.
            Histogram(double resolution):_resolution(resolution),_max_count(0),_print_size(50)
            {
            }
            
            /// Add a value to the histogram.
            /// @param value value will be converted to bin# by divinding by resolution and incrementing resulting bin.
            Histogram &operator()(double value)
            {
                int bin = value/_resolution;
                ++_histogram[bin];
                _max_count = std::max(_max_count,_histogram[bin]);
            }
            
            typedef boost::transform_iterator<IterTransformer, HistogramType::const_iterator> const_iterator;
            
            const_iterator begin() const
            {
                return boost::make_transform_iterator(_histogram.begin(),IterTransformer(_resolution));
            }

            const_iterator end() const
            {
                return boost::make_transform_iterator(_histogram.end(),IterTransformer(_resolution));
            }
            
            void setPrintSize(int ps)
            {
                _print_size = ps;
            }
            
            int getPrintSize() const
            {
                return _print_size;
            }
            
            int maxCount() const
            {
                return _max_count;
            }
    };
        
    std::ostream &operator<<(std::ostream &str, Histogram const &h)
    {
        for(Histogram::const_iterator hi = h.begin(); hi != h.end(); ++hi)
        {
            str << hi->first << "\t" << hi->second << "\t";
            for(int i = 0; i < (hi->second*h.getPrintSize())/h.maxCount(); ++i)
                str << "*";
            str << std::endl;
        }
        return str;
    }
        
}

#endif
