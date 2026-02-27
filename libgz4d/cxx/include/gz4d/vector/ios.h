#ifndef GZ4D_VECTOR_IOS_H
#define GZ4D_VECTOR_IOS_H

#include <ostream>
#include <istream>

#include <gz4d/Vector.h>
#include <gz4d/Exception.h>

namespace gz4d
{
    template <typename T, std::size_t N> std::istream &operator>>(std::istream &str, Vector<T, N> &p)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            if (!(str >> p[i]))
            {
                if (i)
                    throw gz4d::ExtractionException(i, std::string("Incomplete vector extraction"));
                else
                    throw gz4d::ExtractionException();//throw a generic one if we didn't get anything at all.
            }
        }
        return str;
    }

    template <typename T, std::size_t N> std::ostream &operator<<(std::ostream &str, Vector<T, N> const &p)
    {
        str << "(";
        for (std::size_t i = 0; i < N; ++i)
        {
            str << p[i];
            if (i < N-1)
                str << ", ";
        }
        str << ")";
        return str;
    }
}

#endif
