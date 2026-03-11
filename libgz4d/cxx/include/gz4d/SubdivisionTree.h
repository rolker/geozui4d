// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SUBDIVISIONTREE_H
#define GZ4D_SUBDIVISIONTREE_H

#include<gz4d/gz4d.h>
#include <boost/operators.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <map>
#include "Box.h"


namespace gz4d
{
    namespace subtree
    {

        template <typename RT> class RangeIterator
            :public boost::iterator_facade<
                  RangeIterator<RT>
                , typename RT::index_type const
                , boost::forward_traversal_tag
            >
        {
            friend class boost::iterator_core_access;
            
            typedef RT range_type;
            typedef typename range_type::index_type index_type;
            
            range_type const & _range;
            index_type _index;

            void increment()
            {
                for(std::size_t d = 0; d < index_type::coordinates_type::_size; ++d)
                {
                    _index._coordinates[d] += 1;
                    if(_index._coordinates[d] <= _range._box.getMax()[d])
                        break;
                    if(d < index_type::coordinates_type::_size-1)
                        _index._coordinates[d] = _range._box.getMin()[d];
                }
            }

            bool equal(RangeIterator<RT> const &other) const
            {
                return _index == other._index;
            }

            index_type const &dereference() const {return _index;}
            
        public:
            RangeIterator(range_type const &r, index_type const &i):_range(r),_index(i){}
        };
        
        template <typename IT> struct Range
        {
            typedef Range<IT> self_type;
            typedef IT index_type;
            typedef Box<typename index_type::coordinates_type> box_type;
            typedef RangeIterator<self_type> const_iterator;
            
            typename index_type::level_type _level;
            box_type _box;

            
            Range(typename index_type::level_type level, typename index_type::coordinates_type const &min, typename index_type::coordinates_type const &max):_level(level),_box(min,max){}

            Range(index_type const &index1, index_type const &index2):_level(index1._level)
            {
                assert(index1._level == index2._level);
                _box.expand(index1._coordinates);
                _box.expand(index2._coordinates);
            }
            
            Range child() const
            {
                return Range(_level+1,_box.getMin()*2,index_type::unit_coordinates()+_box.getMax()*2);
            }
            Range parent() const
            {
                if(_level == 0)
                    return *this;
                return Range(_level-1,_box.getMin()/2,_box.getMax()/2);
            }

            const_iterator begin() const
            {
                return const_iterator(*this,index_type(_level,_box.getMin()));
            }

            const_iterator end() const
            {
                return ++const_iterator(*this,index_type(_level,_box.getMax()));
            }
            
        };
        
        
        template <typename DT, std::size_t N, typename LT=uint8_t, typename CT=std::size_t, typename NCT=double> struct Index: public boost::less_than_comparable<Index<DT,N,LT,CT,NCT> >
        {
            typedef DT derived_type;
            typedef Index<DT,N,LT,CT,NCT> self_type;
            typedef LT level_type;
            typedef Vector<CT,N> coordinates_type;
            typedef Vector<NCT,N> normalized_coordinates_type;
            typedef Box<normalized_coordinates_type> normalized_box_type;
            typedef Range<derived_type> range_type;
            
            static const uint8_t max_levels = sizeof(coordinates_type::value_type)*8;
            static coordinates_type unit_coordinates() {return coordinates_type(1);}
            
            level_type _level;
            coordinates_type _coordinates;


            Index(Index const &i):_level(i._level),_coordinates(i._coordinates){}
            
            Index(level_type level, coordinates_type const &coordinates):_level(level),_coordinates(coordinates){}

            Index(level_type level, normalized_coordinates_type const &coordinates):_level(level)
            {
                typename coordinates_type::value_type f = factor();
                for(std::size_t i = 0; i < coordinates_type::_size; ++i)
                    _coordinates[i] = f*coordinates[i];
            }

            bool operator==(self_type const &i) const
            {
                return _level == i._level && _coordinates == i._coordinates;
            }
            
            bool operator<(self_type const &i) const
            {
                if(_level < i._level)
                    return true;
                if(_level > i._level)
                    return false;
                for(int j = 0; j < N; ++j)
                {
                    if(_coordinates[j] < i._coordinates[j])
                        return true;
                    if(_coordinates[j] > i._coordinates[j])
                        return false;
                }
                return false;
            }

            range_type children() const
            {
                return range_type(_level+1,_coordinates*2,unit_coordinates+_coordinates*2);
            }

            derived_type parent() const
            {
                if (_level == 0)
                    return *this;
                return self_type(_level-1,_coordinates/2);
            }

            
            derived_type next(uint8_t dimension) const
            {
                self_type ret(*this);
                ret.coordinates[dimension] += 1;
                return ret;
            }

            derived_type previous(uint8_t dimension) const
            {
                self_type ret(*this);
                if(_coordinates[dimension] > 0)
                    ret._coordinates[dimension] -= 1;
                return ret;
            }

            typename coordinates_type::value_type factor() const
            {
                return 1<<_level;
            }
            
            normalized_coordinates_type min() const
            {
                normalized_coordinates_type(_coordinates)/normalized_coordinates_type::value_type(factor());
            }

            normalized_coordinates_type max() const
            {
                normalized_coordinates_type(_coordinates+unit_coordinates)/normalized_coordinates_type::value_type(factor());
            }
            
            normalized_box_type box() const
            {
                return normalized_box_type(min(),max());
            }
            
        };


        template<typename T, typename IT, typename ST> class Node
        {
            typedef T value_type;
            typedef IT index_type;
            typedef ST storage_type;

            storage_type &_tree;
            index_type _index;
        public:
            Node(storage_type& t, index_type const &i):_tree(t),_index(i){}
            operator bool() const
            {
                return _tree.find(_index) != _tree.end();
            }

            value_type &operator()() const
            {
                return _tree[_index];
            }
        };
        
        
    }
    
    template <typename DT, typename T, typename IT> class SubdivisionTree
    {
    public:
        typedef SubdivisionTree<DT,T,IT> self_type;
        typedef DT derived_type;
        typedef T value_type; ///< Type stored in the tree's nodes.
        typedef IT index_type; ///< Type used to index the nodes, typically N dimension integer vector.
        typedef std::map<index_type,value_type> storage_type;
        
        typedef subtree::Node<value_type,index_type,storage_type> node_type;
        typedef subtree::Node<value_type const,index_type,storage_type const> const_node_type;

        typedef typename storage_type::const_iterator const_iterator;
    private:
        storage_type _data;
    public:

        node_type operator()(index_type const &i)
        {
            return node_type(_data,i);
        }

        const_node_type operator()(index_type const &i) const
        {
            return const_node_type(_data,i);
        }

        const_iterator begin() const {return _data.begin();}
        const_iterator end() const {return _data.end();}
        std::size_t size() const {return _data.size();}
    };
}



#endif
