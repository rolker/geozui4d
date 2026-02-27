#ifndef GZ4D_RASTERMETA_H
#define GZ4D_RASTERMETA_H

#include <gz4d/gz4d.h>
#include <gz4d/data/RasterContainer.h>
#include <gz4d/Units.h>
#include <gz4d/Exception.h>
#include <gz4d/Point.h>
#include <gz4d/SphericalPoint.h>

namespace gz4d
{
    template <typename rasterType, typename spatialType, typename valueType>
    class GZ4DEXPORT RasterMeta: public RasterContainer<rasterType>
    {
        Interval<spatialType> rowInterval;
        Interval<spatialType> colInterval;
        ValueScaler<valueType, rasterType> vScaler;
        valueType baseValue;
        public:
            RasterMeta():baseValue(0.0){}
            RasterMeta(RasterMeta const &rm):RasterContainer<rasterType>(rm),rowInterval(rm.rowInterval),colInterval(rm.colInterval),vScaler(rm.vScaler),baseValue(rm.baseValue){}

            void SetColInterval(Interval<spatialType> const &i){colInterval = i;}
            void SetRowInterval(Interval<spatialType> const &i){rowInterval = i;}
            void SetValueScaler(ValueScaler<valueType, rasterType> const &v){vScaler = v;}
            void SetValueScaler(double s, double o){vScaler = ValueScaler<valueType, rasterType>(s,o);}
            void SetBaseValue(valueType const &b){baseValue = b;}
            valueType GetBaseValue() const {return baseValue;}

            Interval<spatialType> const &GetColInterval(){return colInterval;}
            Interval<spatialType> const &GetRowInterval(){return rowInterval;}

            valueType GetValue(int col, int row) const
            {
                if(!this->raster)
                    throw(Exception("No raster associated with RasterMeta"));
                int maxCol = this->raster->GetColumns()-1;
                int maxRow = this->raster->GetRows()-1;

                if(col < 0) col = 0;
                if(row < 0) row = 0;
                if(col > maxCol) col = maxCol;
                if(row > maxRow) row = maxRow;
                return vScaler.Value(this->raster->GetRow(row)[col]);
            }

            Vector<spatialType,2> GetPoint(int col, int row) const
            {
                if(!this->raster)
                    throw(Exception("No raster associated with RasterMeta"));

                int maxCol = this->raster->GetColumns()-1;
                int maxRow = this->raster->GetRows()-1;

                Vector<spatialType,2> ret(colInterval.GetStart(),rowInterval.GetStart());
                if(maxCol > 0)
                    ret[0] = colInterval.Map((double)col/(double)maxCol);
                if(maxRow > 0)
                    ret[1] = rowInterval.Map((double)row/(double)maxRow);
                return ret;
            }
    };

    template <typename rasterType, typename valueType>
            class SphericalRaster: public RasterMeta<rasterType, valueType, valueType >
    {
        public:
            void SetThetaInterval(Interval<valueType> const &i)
            {
                RasterMeta<rasterType, valueType, valueType >::SetColInterval(i);
            }
            void SetThetaInterval(valueType s, valueType e)
            {
                SetThetaInterval(Interval<valueType>(s,e));
            }

            void SetPhiInterval(Interval<valueType> const &i)
            {
                RasterMeta<rasterType, valueType, valueType >::SetRowInterval(i);
            }
            void SetPhiInterval(valueType s, valueType e)
            {
                SetPhiInterval(Interval<valueType>(s,e));
            }

            SphericalPoint<valueType> GetPoint(int col, int row) const 
            {
                Vector<valueType,2> angle(RasterMeta<rasterType, valueType, valueType >::GetPoint(col,row));
                return SphericalPoint<valueType>(RasterMeta<rasterType, valueType, valueType>::GetValue(col,row), angle[0], angle[1]);
            }
            SphericalPoint<valueType> GetBasePoint(int col, int row) const 
            {
                Vector<valueType,2> angle(RasterMeta<rasterType, valueType, valueType>::GetPoint(col,row));
                return SphericalPoint<valueType>(RasterMeta<rasterType, valueType, valueType>::GetBaseValue(), angle[0], angle[1]);
            }
    };
}

#endif
