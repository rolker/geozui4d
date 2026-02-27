#ifndef GZ4D_RASTER_H
#define GZ4D_RASTER_H

#include <gz4d/gz4d.h>
#include <gz4d/Box.h>
#include <vector>
#include <memory>
#include <cmath>
#include <boost/iterator/iterator_adaptor.hpp>
#include <cstring>

namespace gz4d
{
    /// \ingroup container
    template <typename T> class Raster
    {
        std::vector<T> data;
        std::size_t columns;
        std::size_t rows;

        public:
            typedef T value_type;
            typedef std::shared_ptr<Raster<T> > Ptr;

            class iterator : public boost::iterator_adaptor<iterator,T*>
            {
                public:
                    iterator(T* i):boost::iterator_adaptor<iterator,T*>(i){}
            };

            class const_iterator: public boost::iterator_adaptor<const_iterator,const T*>
            {
                public:
                    const_iterator(const T* i):boost::iterator_adaptor<const_iterator,const T*>(i){}
            };

            Raster():columns(0),rows(0){}
            Raster(Raster const &other) :data(other.data),columns(other.columns),rows(other.rows) {}
            Raster(std::size_t c, std::size_t r, T const &init)
                :data(c*r,init),columns(c),rows(r)
            {
            }
            
            Raster(Vector<std::size_t,2> const &s, T const &init):data(s[0]*s[1],init),columns(s[0]),rows(s[1])
            {
                
            }

            template <typename OT> explicit Raster(Raster<OT> const &other)
            :columns(other.GetColumns()),rows(other.GetRows())
            {
                std::size_t s = columns*rows;
                data.resize(s);
                OT const * odata = &other.front();
                for(std::size_t i = 0; i < s; ++i)
                {
                    data[i] = odata[i];
                }
            }

            /// construct scaled copy.
            Raster(std::size_t c, std::size_t r, Raster const &o)
            :data(c*r),columns(c),rows(r)
            {
                double xscale = o.columns/double(columns);
                double yscale = o.rows/double(rows);
                for(std::size_t y = 0; y < rows; ++y)
                {
                    std::size_t sy0 = floor(y*yscale);
                    std::size_t sy1 = ceil(y*yscale);
                    double sy0p = sy1-y*yscale;
                    double sy1p = 1.0 - sy0p;
                    for(std::size_t x = 0; x < columns; ++x)
                    {
                        std::size_t sx0 = floor(x*xscale);
                        std::size_t sx1 = ceil(x*xscale);
                        double sx0p = sx1-x*xscale;
                        double sx1p = 1.0 - sx0p;
                        data[y*columns+x] = o.GetRow(sy0)[sx0]*sy0p*sx0p+
                                            o.GetRow(sy1)[sx0]*sy1p*sx0p+
                                            o.GetRow(sy0)[sx1]*sy0p*sx1p+
                                            o.GetRow(sy1)[sx1]*sy1p*sx1p;
                    }
                }
            }


            void Resize(std::size_t c, std::size_t r)
            {
                columns = c;
                rows = r;
                data.resize(c*r);
            }
            
            void Resize(Vector<std::size_t,2> const &s)
            {
                Resize(s[0],s[1]);
            }

            iterator operator[](std::size_t r) {return &data[r*columns];}
            iterator begin() {return &data.front();}
            iterator end() {return &data.front()+columns*rows;}

            const_iterator operator[](std::size_t r) const {return &data[r*columns];}
            const_iterator begin() const {return &data.front();}
            const_iterator end() const {return &data.front()+columns*rows;}

            std::size_t GetColumns() const {return columns;}
            std::size_t GetRows() const {return rows;}
            Vector<std::size_t,2> getSizes() const {return Vector<std::size_t,2>(columns,rows);}
            
            //std::size_t raw_size() const {return rows*columns;}
            bool empty() const {return !raw_size();}

            T const *GetRow(std::size_t r) const
            {
                return &(data.begin()[r*columns]);
            }

            T *GetRow(std::size_t r)
            {
                return &(data.begin()[r*columns]);
            }

            std::size_t GetElementSize(){return sizeof(T);}

            T &front() {return data.front();}
            T const &front() const {return data.front();}

            void* raw() const {return const_cast<void*>(reinterpret_cast<const void*>(&data.front()));}
            std::size_t raw_size() const {return rows*columns*sizeof(T);}

            void subRaster(int xOffset, int yOffset, Raster const &o)
            {
                std::size_t sr = 0, dr = 0;
                if(yOffset < 0)
                    sr = -yOffset;
                else
                    dr = yOffset;

                std::size_t sc = 0, dc = 0;
                if(xOffset < 0)
                    sc = -xOffset;
                else
                    dc = xOffset;

                std::size_t copy_height = std::min(rows-dr,o.GetRows()-sr);
                std::size_t copy_width = std::min(columns-dc,o.GetColumns()-sc);

                const_iterator sri = o.begin()+sr*o.GetColumns();
                for(iterator dri = begin()+dr; dri != begin()+(dr+copy_height)*columns; sri+=o.GetColumns(), dri+=columns)
                {
                    memcpy(reinterpret_cast<void*>(&(*(dri+dc))),reinterpret_cast<void const *>(&(*(sri+sc))),sizeof(T)*copy_width);
                }
            }
            
            Raster::Ptr subRaster(Box<Vector<std::size_t,2> > const &box) const
            {
                if(box.empty())
                    return Raster::Ptr();
                Raster::Ptr ret(new Raster);
                ret->Resize(box.getSizes());
                for(std::size_t y = 0; y < ret->rows; ++y)
                {
                    T* dest_row = ret->GetRow(y);
                    const T* src_row = GetRow(y+box.getMin()[1]);
                    for(std::size_t x = 0; x < ret->columns; ++x)
                        dest_row[x] = src_row[x+box.getMin()[0]];
                }
                //ret->subRaster(-box.getMin()[0],-box.getMin()[1],*this);
                return ret;
            }
            
            Box<Vector<std::size_t,2> > findBorders(T const &borderValue)
            {
                std::size_t minx = 0;
                for(std::size_t x = 0; x < columns; ++x)
                {
                    bool blank = true;
                    for(std::size_t y = 0; y < rows; ++ y)
                    {
                        if(data[y*columns+x] != borderValue)
                        {
                            blank = false;
                            break;
                        }
                    }
                    if(!blank)
                    {
                        minx = x;
                        break;
                    }
                }
                
                std::size_t maxx = 0;
                for(std::size_t x = columns-1; x > 0; --x)
                {
                    bool blank = true;
                    for(std::size_t y = 0; y < rows; ++ y)
                    {
                        if(data[y*columns+x] != borderValue)
                        {
                            blank = false;
                            break;
                        }
                    }
                    if(!blank)
                    {
                        maxx = x;
                        break;
                    }
                }

                std::size_t miny = 0;
                for(std::size_t y = 0; y < rows; ++y)
                {
                    bool blank = true;
                    for(std::size_t x = 0; x < columns; ++ x)
                    {
                        if(data[y*columns+x] != borderValue)
                        {
                            blank = false;
                            break;
                        }
                    }
                    if(!blank)
                    {
                        miny = y;
                        break;
                    }
                }
                
                std::size_t maxy = 0;
                for(std::size_t y = rows-1; y > 0; --y)
                {
                    bool blank = true;
                    for(std::size_t x = 0; x < columns; ++x)
                    {
                        if(data[y*columns+x] != borderValue)
                        {
                            blank = false;
                            break;
                        }
                    }
                    if(!blank)
                    {
                        maxy = y;
                        break;
                    }
                }
                
                assert(minx >= 0 && maxx < columns && miny >= 0 && maxy < rows);
                
                return Box<Vector<std::size_t,2> >(Vector<std::size_t,2>(minx,miny),Vector<std::size_t,2>(maxx,maxy));
                
            }
            
            double correlation(Raster const &other, gz4d::Vector<int, 2 > const &offset) const
            {
                double ret = 0.0;
                std::size_t j=0;
                std::size_t jo = 0;
                if(offset[1] < 0)
                    jo = -offset[1];
                else
                    j = offset[1];
                    
                for(; j<rows && jo<other.rows; ++j,++jo)
                {
                    std::size_t i=0;
                    std::size_t io = 0;
                    if(offset[0] < 0)
                        io = -offset[0];
                    else
                        i = offset[0];
                    
                    for(; i<columns && io<other.columns; ++i,++io)
                    {
                        ret += norm(GetRow(j)[i])*norm(other.GetRow(jo)[io]);
                    }
                }
                return ret;
            }
            
            
            /// Find largest box area containing only valid values.
            Box<Vector<std::size_t,2> > findNonNullBox (T const &nullValue, bool square = false) const
            {
                typedef std::pair<int,int> ValidRangeType;
                std::vector<ValidRangeType> validRanges(rows,ValidRangeType(-1,-1));
                for(int j=0; j<rows; ++j)
                {
                    const T* row = GetRow(j);
                    ValidRangeType current_range(0,0);
                    int current_max = 0;
                    for(int i = 0; i < columns; ++i)
                    {
                        if(row[i] == nullValue)
                        {
                            current_range.first = i+1;
                            current_range.second = i+1;
                        }
                        else
                        {
                            current_range.second = i+1;
                            if(current_range.second-current_range.first > current_max)
                            {
                                current_max = current_range.second-current_range.first;
                                validRanges[j] = current_range;
                            }
                        }
                    }
                }
                
                Box<Vector<std::size_t,2> > ret;
                for(int startRow = 0; startRow < rows; ++startRow)
                {
                    for(int endRow = startRow+1; endRow <= rows; ++endRow)
                    {
                        ValidRangeType widthRange(validRanges[startRow]);
                        for(int i = startRow; i < endRow && widthRange.second>widthRange.first; ++i)
                        {
                            widthRange.first = std::max(widthRange.first,validRanges[i].first);
                            widthRange.second = std::min(widthRange.second,validRanges[i].second);
                        }
                        if(widthRange.second > widthRange.first)
                            if(square)
                            {
                                int size = std::min(widthRange.second-widthRange.first,endRow-1-startRow);
                                int minx = widthRange.first;
                                int maxx = widthRange.second;
                                int miny = startRow;
                                int maxy = endRow-1;
                                if(maxx-minx > size)
                                {
                                    minx += ((maxx-minx)-size)/2;
                                    maxx = minx+size;
                                }
                                if(maxy-miny > size)
                                {
                                    miny += ((maxy-miny)-size)/2;
                                    maxy = miny+size;
                                }
                                
                                Box< Vector< std::size_t, 2 > > b = Box<Vector<std::size_t,2> >(Vector<std::size_t,2>(minx,miny), Vector<std::size_t,2>(maxx,maxy));
                                if(ret.empty() || b.getVolume() >= ret.getVolume())
                                    ret = b;
                            }
                            else
                            {
                                if(ret.empty() || (widthRange.second-widthRange.first)*(endRow-startRow-1) > ret.getVolume())
                                    ret = Box<Vector<std::size_t,2> >(Vector<std::size_t,2>(widthRange.first,startRow), Vector<std::size_t,2>(widthRange.second,endRow-1));
                            }
                    }
//                     if(!ret.empty() && ret.getVolume() > (rows-startRow)*columns)
//                         break;
                }
                return ret;
            }
            
            Box<Vector<std::size_t,2> > findNonNullBox (T const &nullValue, Raster const &other, bool square = false) const
            {
                typedef std::pair<int,int> ValidRangeType;
                std::vector<ValidRangeType> validRanges(rows,ValidRangeType(-1,-1));
                for(int j=0; j<rows; ++j)
                {
                    const T* row = GetRow(j);
                    const T* orow = other.GetRow(j);
                    ValidRangeType current_range(0,0);
                    int current_max = 0;
                    for(int i = 0; i < columns; ++i)
                    {
                        if(row[i] == nullValue || orow[i] == nullValue)
                        {
                            current_range.first = i+1;
                            current_range.second = i+1;
                        }
                        else
                        {
                            current_range.second = i+1;
                            if(current_range.second-current_range.first > current_max)
                            {
                                current_max = current_range.second-current_range.first;
                                validRanges[j] = current_range;
                            }
                        }
                    }
                }
                
                Box<Vector<std::size_t,2> > ret;
                for(int startRow = 0; startRow < rows; ++startRow)
                {
                    for(int endRow = startRow+1; endRow <= rows; ++endRow)
                    {
                        ValidRangeType widthRange(validRanges[startRow]);
                        for(int i = startRow; i < endRow && widthRange.second>widthRange.first; ++i)
                        {
                            widthRange.first = std::max(widthRange.first,validRanges[i].first);
                            widthRange.second = std::min(widthRange.second,validRanges[i].second);
                        }
                        if(widthRange.second > widthRange.first)
                            if(square)
                            {
                                int size = std::min(widthRange.second-widthRange.first,endRow-1-startRow);
                                int minx = widthRange.first;
                                int maxx = widthRange.second;
                                int miny = startRow;
                                int maxy = endRow-1;
                                if(maxx-minx > size)
                                {
                                    minx += ((maxx-minx)-size)/2;
                                    maxx = minx+size;
                                }
                                if(maxy-miny > size)
                                {
                                    miny += ((maxy-miny)-size)/2;
                                    maxy = miny+size;
                                }
                                
                                Box< Vector< std::size_t, 2 > > b = Box<Vector<std::size_t,2> >(Vector<std::size_t,2>(minx,miny), Vector<std::size_t,2>(maxx,maxy));
                                if(ret.empty() || b.getVolume() >= ret.getVolume())
                                    ret = b;
                            }
                            else
                            {
                                if(ret.empty() || (widthRange.second-widthRange.first)*(endRow-startRow-1) > ret.getVolume())
                                    ret = Box<Vector<std::size_t,2> >(Vector<std::size_t,2>(widthRange.first,startRow), Vector<std::size_t,2>(widthRange.second,endRow-1));
                            }
                    }
                    //                     if(!ret.empty() && ret.getVolume() > (rows-startRow)*columns)
                    //                         break;
                }
                return ret;
            }
            
    };

}

#endif
