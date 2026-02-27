#ifndef GZ4D_SONAR_SAMPLECONTAINER_H
#define GZ4D_SONAR_SAMPLECONTAINER_H

#include <gz4d/time.h>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/bind.hpp>

namespace gz4d
{
    namespace sonar
    {
        /// CDT channel data type
        template <typename CDT> class SampleContainer
        {
            public:
                template <typename T> class sample_iterator: public boost::transform_iterator<T , typename CDT::iterator>
                {
                    public:
                        sample_iterator(){}
                        sample_iterator(typename CDT::iterator i, std::shared_ptr<CDT> &c):boost::transform_iterator<T,typename CDT::iterator>(i,T(c)){}
                        template <typename OT> sample_iterator(OT const &i):boost::transform_iterator<T, typename CDT::iterator>(i.base(),T(i.functor())){}
                        operator bool() const {return boost::transform_iterator<T, typename CDT::iterator>::functor();}
                };

                typedef sample_iterator<typename CDT::functor_types::raw_type>         iterator;
                typedef sample_iterator<typename CDT::functor_types::sv_type>       sv_iterator;
                typedef sample_iterator<typename CDT::functor_types::range_type> range_iterator;
                typedef sample_iterator<typename CDT::functor_types::ts_type>       ts_iterator;
                typedef sample_iterator<typename CDT::functor_types::ss_type>       ss_iterator;
                typedef sample_iterator<typename CDT::functor_types::xyz_type>     xyz_iterator;
                typedef sample_iterator<typename CDT::functor_types::error_type> error_iterator;
                typedef sample_iterator<typename CDT::functor_types::time_type>   time_iterator;

                typedef typename CDT::functor_types functor_types;

                SampleContainer(){}
                SampleContainer(typename CDT::iterator bi, typename CDT::iterator ei, std::shared_ptr<CDT>  &c):iterators(iterator(bi,c),iterator(ei,c)){}

                iterator begin(){return iterators.first;}
                iterator end(){return iterators.second;}
                bool empty() const {return !(iterators.first && iterators.second && iterators.first != iterators.second);}
            private:
                std::pair<iterator,iterator> iterators;

        };

    }
}

#endif
