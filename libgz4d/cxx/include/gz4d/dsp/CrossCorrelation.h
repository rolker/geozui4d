#ifndef GZ_DSP_CROSSCORRELATION_H
#define GZ_DSP_CROSSCORRELATION_H

#include <gz4d/gz4d.h>
#include <numeric>

namespace gz4d
{
    template <typename T> std::vector<T> CrossCorrelation(std::vector<T> const &a, std::vector<T> const &b)
    {
        std::vector<T> ret;
        for(int i = 0; i < a.size(); ++i)
        {
            T val = 0;
            for(int j = 0; j < b.size(); ++j)
                val += a[(i+j)%a.size()]*b[j];
            ret.push_back(val);
        }
        return ret;
    }
}

#endif
