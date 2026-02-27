#ifndef GZ4D_TIMESEQUENCE_H
#define GZ4D_TIMESEQUENCE_H

#include <gz4d/time.h>

namespace gz4d
{
    /// Time ordered container.
    template <typename T> class TimeSequence
    {
        typedef std::map<Time,T> map_type;
        std::map<Time,T> values;
        public:
            typedef T value_type;
            typedef typename std::map<Time,T>::iterator iterator;
            typedef typename std::map<Time,T>::const_iterator const_iterator;
            typedef typename std::map<Time,T>::const_reverse_iterator const_reverse_iterator;
            typedef std::pair<const_iterator,const_iterator> iterator_pair;


            T const &operator[](Time t) const {return values[t];}
            T &operator[](Time t) {return values[t];}

            /// Return last iterator with time value below or equal to t.
            /// Returns end iterator if empty or all values greater than t.
            const_iterator GetLatest(Time t) const
            {
                if(empty())
                    return values.end();
                const_iterator ret = values.upper_bound(t);
                if (ret != values.begin())
                    --ret;
                else
                    return values.end();
                return ret;
            }
            
            /// Returns first iterator with time equal or greater than t
            /// Returns end iterator if none exists
            const_iterator GetNext(Time t) const {return values.lower_bound(t);}

            iterator_pair GetSpan(time::Period const & ts) const {return iterator_pair(GetLatest(ts.start()),GetNext(ts.finish()));}

            const_iterator begin() const {return values.begin();}
            iterator begin() {return values.begin();}
            
            const_iterator end() const {return values.end();}
            const_reverse_iterator rbegin() const {return values.rbegin();}
            const_reverse_iterator rend() const {return values.rend();}

            std::size_t count(Time t) const {return values.count(t);}
            bool InInterval(Time t) const {return (!values.empty() && t >= values.begin()->first && t <= values.rbegin()->first);}
            T GetInterpolated(Time t) const
            {
                if(values.empty())
                    throw gz4d::Exception("invalid interpolation of empty TimeSequence");
                if(!InInterval(t))
                {
                    if(t < values.begin()->first)
                        return values.begin()->second;
                    return values.rbegin()->second;
                }
                const_iterator start = GetLatest(t);
                if (start->first == t)
                    return start->second;
                const_iterator end = GetNext(t);
                double p = time::Period(start->first,end->first).progress(t);
                return interpolate(start->second,end->second,p);
            }
            
            time::Period period() const
            {
                if (!values.empty())
                    return time::Period(values.begin()->first,values.rbegin()->first);
                return time::Period();
            }
            
            bool empty() const {return values.empty();}
            std::size_t size() const {return values.size();}
            
            /// Trim container by removing oldest elements.
            /// Extra elements are removed until size <= max_size.
            /// @param max_size: Target container size.
            /// @return: Number of removed elements.
            std::size_t trim_back(std::size_t max_size)
            {
                std::size_t ret = std::max(0,values.size());
                typename map_type::iterator fi = values.rend();
                for(std::size_t i = 0; i < ret; ++i)
                    ++fi;
                typename map_type::reverse_iterator ri = fi;
                values.erase(ri,values.rend());
                return ret;
            }
    };

}

#endif
