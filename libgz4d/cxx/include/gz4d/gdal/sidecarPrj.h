// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GDAL_SIDECARPRJ_H
#define GZ4D_GDAL_SIDECARPRJ_H

#include <gz4d/ogr/SpatialReference.h>
#include <ogr_spatialref.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace gz4d
{
    namespace gdal
    {
        namespace detail
        {
            // Parses a CRS definition (WKT1, ESRI WKT1 or WKT2) without
            // network or file access, so the text cannot make GDAL fetch a
            // URL or read another file. Returns an empty reference when the
            // text does not parse.
            inline gz4d::ogr::SpatialReference parseCrsText(std::string const &text)
            {
                gz4d::ogr::SpatialReference ret;
                if(text.empty())
                    return ret;
                const char *const options[] = {"ALLOW_NETWORK_ACCESS=NO", "ALLOW_FILE_ACCESS=NO", nullptr};
                OGRSpatialReference check;
                if(check.SetFromUserInput(text.c_str(), options) != OGRERR_NONE)
                    return ret;
                // The text parsed; the wrapper's SetFromUserInput does not
                // report errors, so it is only given text known to be good.
                ret.SetFromUserInput(text);
                return ret;
            }
        }

        /// Spatial reference for a raster opened with GDAL.
        ///
        /// datasetWkt is the dataset's own CRS (GDALDataset::GetProjectionRef()).
        /// When it is not empty it is used as is. When it is empty, the
        /// sidecar <stem>.prj, then <stem>.PRJ, beside rasterPath is read
        /// instead: GDAL's AAIGrid driver reads only ESRI WKT1 from a .prj,
        /// so a WKT2 .prj leaves the dataset with no CRS.
        ///
        /// Returns an empty reference (operator bool false) when there is no
        /// CRS. A .prj that exists but does not parse is reported on
        /// std::cerr, naming the file.
        inline gz4d::ogr::SpatialReference spatialReferenceForRaster(std::string const &datasetWkt, std::filesystem::path const &rasterPath)
        {
            if(!datasetWkt.empty())
            {
                gz4d::ogr::SpatialReference ret = detail::parseCrsText(datasetWkt);
                if(!ret)
                    std::cerr << "spatialReferenceForRaster: could not parse the CRS GDAL gave for " << rasterPath.string() << std::endl;
                return ret;
            }

            for(const char *extension: {".prj", ".PRJ"})
            {
                std::filesystem::path prjPath = rasterPath;
                prjPath.replace_extension(extension);
                std::error_code ec;
                if(!std::filesystem::is_regular_file(prjPath, ec))
                    continue;

                std::ifstream prj(prjPath);
                std::stringstream contents;
                contents << prj.rdbuf();
                gz4d::ogr::SpatialReference ret = detail::parseCrsText(contents.str());
                if(!ret)
                    std::cerr << "spatialReferenceForRaster: could not parse the CRS in " << prjPath.string() << std::endl;
                return ret;
            }

            return gz4d::ogr::SpatialReference();
        }
    }
}

#endif
