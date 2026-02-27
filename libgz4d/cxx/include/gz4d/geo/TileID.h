#ifndef GZ4D_GEO_TILEID_H
#define GZ4D_GEO_TILEID_H

#include <gz4d/Box.h>
#include <boost/operators.hpp>

namespace gz4d
{
    namespace geo
    {
        struct EarthTileSpecs
        {
            static Vector<double,2> baseOrigin() {return Vector<double,2>(-180.0,-90.0);}
            static Vector<double,2> baseUnitsPerPixel() {return Vector<double,2>(0.703125,0.703125);}
            static Vector<uint32_t,2> tileSize() {return Vector<uint32_t,2>(256,256);}
        };
        
        template<typename TS=EarthTileSpecs> struct TileID: boost::partially_ordered<TileID<TS> >
        {
            int8_t level;
            typedef Vector<int32_t,2> IndexType;
            IndexType index;

            double factor() const {return 1.0/(1<<level);}

            struct Quad
            {
                TileID sw,se,nw,ne;
                Quad(TileID const &parent):sw(parent.level+1, IndexType(parent.index[0]*2,parent.index[0]*2)),se(sw.east()),nw(sw.north()),ne(nw.east()){}
            };
            
            TileID(TileID const &tid):level(tid.level),index(tid.index){}
            TileID():level(-1),index(IndexType(-1,-1)){}
            TileID(int8_t level, IndexType index):level(level),index(index){}

            TileID parent() const {return TileID(level-1,index/2);}
            Quad children() const {return Quad(*this);}

            TileID east() const {return TileID(level,IndexType(index[0]+1,index[1]));}
            TileID west() const {return TileID(level,IndexType(index[0]-1,index[1]));}
            TileID north() const {return TileID(level,IndexType(index[0],index[1]+1));}
            TileID south() const {return TileID(level,IndexType(index[0],index[1]-1));}

            Vector<double,2> size() const
            {
                double sf = factor();
                return TS::tileSize()*TS::baseUnitsPerPixel()*sf;
            }

            operator bool() const
            {
                return level >= 0;
            }
            
            Box2d boundingBox() const
            {
                Vector<double,2> s = size();
                Vector<double,2> minCoord(TS::baseOrigin()+s*index);
                return Box2d(minCoord,minCoord+s);
            }

            bool operator==(TileID const&o) const
            {
                if(!*this && !o)
                    return true;
                if(!*this || !o)
                    return false;
                return level == o.level && index == o.index;
            }

            bool operator<(TileID const &o) const
            {
                if(*this && o)
                {
                    if(level < o.level)
                        return true;
                    if(level == o.level)
                    {
                        if(index[0] < o.index[0])
                            return true;
                        if(index[0] == o.index[0])
                            return index[1] < o.index[1];
                    }
                }
                return false;
            }
        };
    }
}

#endif
