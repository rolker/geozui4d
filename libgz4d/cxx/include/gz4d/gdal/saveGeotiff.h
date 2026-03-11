// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GDAL_SAVEGEOTIFF_H
#define GZ4D_GDAL_SAVEGEOTIFF_H

#include <gz4d/Image.h>
#include <gz4d/geo/Image.h>
#include "Dataset.h"
#include <gdal_priv.h>
#include <ogr_spatialref.h>

namespace gz4d
{
    namespace gdal
    {
        inline bool saveGeotiff(std::string const &fname, ImageRGBA8 const &img, std::string const &projSpec, double upperLeftX, double upperLeftY, double xRes, double yRes)
        {
            GDALAllRegister();
            GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("GTiff");

            std::vector<const char *> options;

            options.push_back("COMPRESS=DEFLATE");
            options.push_back("ZLEVEL=9");
            options.push_back(NULL);
            
            GDALDataset* dataset = driver->Create(fname.c_str(),img.width(),img.height(),4,GDT_Byte,const_cast<char **>(&options.front()));

            double geoTransform[6] = {0.,0.,0.,0.,0.,0.};
            geoTransform[0] = upperLeftX;
            geoTransform[3] = upperLeftY;
            geoTransform[1] = xRes;
            geoTransform[5] = yRes;

            dataset->SetGeoTransform(geoTransform);
            OGRSpatialReference sr;
            sr.SetFromUserInput(projSpec.c_str());
            char *p;
            sr.SetWellKnownGeogCS("WGS84");
            sr.exportToWkt(&p);
            dataset->SetProjection(p);
            OGRFree(p);

            dataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, img.width(), img.height(), img.raw()+img.width()*4*(img.height()-1), img.width(), img.height(), GDT_Byte, 4, -4*img.width());
            dataset->GetRasterBand(1)->SetColorInterpretation(GCI_RedBand);
            dataset->GetRasterBand(2)->RasterIO(GF_Write,0,0,img.width(),img.height(),img.raw()+img.width()*4*(img.height()-1)+1,img.width(),img.height(),GDT_Byte,4,-4*img.width());
            dataset->GetRasterBand(2)->SetColorInterpretation(GCI_GreenBand);            
            dataset->GetRasterBand(3)->RasterIO(GF_Write,0,0,img.width(),img.height(),img.raw()+img.width()*4*(img.height()-1)+2,img.width(),img.height(),GDT_Byte,4,-4*img.width());
            dataset->GetRasterBand(3)->SetColorInterpretation(GCI_BlueBand);
            dataset->GetRasterBand(4)->RasterIO(GF_Write,0,0,img.width(),img.height(),img.raw()+img.width()*4*(img.height()-1)+3,img.width(),img.height(),GDT_Byte,4,-4*img.width());
            dataset->GetRasterBand(4)->SetColorInterpretation(GCI_AlphaBand);
            
            std::vector<int> overviewLevels;
            overviewLevels.push_back(2);
            while(img.width()/overviewLevels.back() > 4 && img.height()/overviewLevels.back() > 4)
                overviewLevels.push_back(overviewLevels.back()*2);

            dataset->BuildOverviews("GAUSS",overviewLevels.size(),&overviewLevels.front(),0,NULL,NULL,NULL);
            
            delete dataset;

            return true;
            
        }
        
        inline bool saveGeotiff(std::string const &fname, geo::Image<RGBA8> const &img)
        {
            return saveGeotiff(fname,img,img.getProjection().getParameters(),img.getBoundingBox().getMin()[0],img.getBoundingBox().getMax()[1],img.getBoundingBox().getSizes()[0]/double(img.width()),-img.getBoundingBox().getSizes()[1]/double(img.height()));
        }
        
        inline geo::Image<RGBA8> loadGeotiff(std::string const &fn)
        {
            Dataset d = Dataset(fn);
            return geo::Image<RGBA8>(d.getImage(),d.getProjection(),d.getBBox());
        }
        
    }
}

#endif
