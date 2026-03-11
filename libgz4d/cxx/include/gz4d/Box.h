// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_BOX_H
#define GZ4D_BOX_H

#include <gz4d/Vector.h>
#include <gz4d/util/Utilities.h>
#include <gz4d/Exception.h>

namespace gz4d
{
    /// \ingroup base
    template <typename T> class Box
    {
    public:
        typedef T value_type;
    private:
        value_type _min;
        value_type _max;
    public:
        Box(Box const &b):_min(b._min),_max(b._max){}
        Box(){_min +=1;}
        //:min(T(typename T::value_type(1))),max(typename T::value_type(0)){}

        Box(T const &min, T const &max)
        :_min(min),_max(max){}

        bool operator!=(Box<T> const &other) const
        {
            return _min != other._min || _max != other._max;
        }
        
        bool operator==(Box<T> const &other) const
        {
            return !((*this)==other);
        }
        
        T const &getMin() const {return _min;}
        T const &getMax() const {return _max;}

        void setMin(T const &m) {_min = m;}
        void setMax(T const &m) {_max = m;}

        bool empty() const
        {
            return IsNan(_min[0]) || _min[0] > _max[0];
        }

        T getCenter() const
        {
            if(empty())
                return _min;
            T ret;
            for(std::size_t i = 0; i < T::size(); ++i)
                ret[i] = _min[i] + (_max[i] - _min[i]) *0.5;
            return ret;
        }

        T getSizes() const
        {
            if(empty())
                throw(Exception("Empty box doesn't have a size"));

            T ret;
            for(std::size_t i = 0; i < T::size(); ++i)
                ret[i] = _max[i] - _min[i];
            return ret;
        }

        typename T::value_type getMaxLength() const
        {
            if(empty())
                throw(Exception("Empty box doesn't have a length"));

            typename T::value_type ret = _max[0]-_min[0];
            for(std::size_t i = 1; i < T::size(); ++i)
                ret = std::max(ret,_max[i] - _min[i]);
            return ret;
        }

        typename T::value_type getMinLength() const
        {
            if(empty())
                throw(Exception("Empty box doesn't have a length"));

            typename T::value_type ret = _max[0]-_min[0];
            for(std::size_t i = 1; i < T::size(); ++i)
                ret = std::min(ret,_max[i] - _min[i]);
            return ret;
        }
        
        typename T::value_type getVolume() const
        {
            if(empty())
                throw(Exception("Empty box doesn't have a volume"));
            return getSizes().volume();
        }
        

        Box &expand(T const &p)
        {
            if(empty())
            {
                _min = p;
                _max = p;
            }
            else
                for(std::size_t i = 0; i < T::size(); ++i)
                {
                    if(p[i] < _min[i])
                    {
                        if(p[i] > _max[i])  // for types that can wrap, such as Angles
                        {
                            if(_min[i]-p[i]<p[i]-_min[i])
                                _min[i] = p[i];
                            else
                                _max[i] = p[i];
                        }
                        else
                            _min[i] = p[i];
                    }
                    else if(p[i] > _max[i])
                        _max[i] = p[i];
                }
            return *this;
        }

        Box &expand(Box<T> const &other)
        {
            expand(other._min);
            expand(other._max);
        }

        typename T::value_type distance(T const &p) const
        {
            if(contains(p))
                return 0;
            typename T::value_type d2 = 0;
            for(std::size_t i = 0; i < T::size(); ++i)
            {
                if(p[i] > _max[i])
                    d2 += (p[i]-_max[i])*(p[i]-_max[i]);
                else if (p[i] < _min[i])
                    d2 += (_min[i]-p[i])*(_min[i]-p[i]);
            }
            return sqrt(d2);
        }

        template <typename OT> bool contains(OT const &p) const
        {
            assert(T::size() == OT::size());
            if(empty())
                return false;
            for(std::size_t i = 0; i < T::size(); ++i)
                if(p[i] < _min[i] || p[i] > _max[i])
                    return false;
            return true;
        }

        bool contains(Box<T> const &other) const
        {
            if(other.empty())
                return false;
            return contains(other._min) && contains(other._max);
        }

        bool intersects(Box<T> const &other) const
        {
            for(std::size_t i = 0; i < T::size(); ++i)
                if(_min[i] > other._max[i] || _max[i] < other._min[i])
                    return false;
            return true;
        }

        void setSizesFromCenter(T const &s)
        {
            T c = getCenter();
            _min = c-s/2.0;
            _max = c+s/2.0;
        }

        void setSizesFromMin(T const &s)
        {
            if(empty())
                _min = T(0);
            _max = _min + s;
        }
        
        Box operator&(Box const &o) const
        {
            if(intersects(o))
            {
                T newMin, newMax;
                for(std::size_t i = 0; i < T::size(); ++i)
                {
                    newMin[i] = std::max(_min[i],o._min[i]);
                    newMax[i] = std::min(_max[i],o._max[i]);
                }
                return Box(newMin,newMax);
            }
            return Box();
        }
        
        Box operator|(Box const &o) const
        {
            return Box(*this).expand(o);
        }
        
        Box &operator+=(const value_type &v)
        {
            _min += v;
            _max += v;
            return *this;
        }
        
        Box &operator-=(const value_type &v)
        {
            return this->operator+=(-v);
        }
        
        Box operator+(const value_type &v) const
        {
            return Box(*this)+=v;
        }

        Box operator-(const value_type &v) const
        {
            return Box(*this)-=v;
        }
    };

    typedef Box< Vector<float,3> > Box3f;
    typedef Box< Vector<double,3> > Box3d;
    typedef Box< Vector<float,2> > Box2f;
    typedef Box< Vector<double,2> > Box2d;
}

#endif
