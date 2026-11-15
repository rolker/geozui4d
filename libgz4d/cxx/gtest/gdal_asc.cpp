// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

// Checks that an ESRI ASCII grid (.asc) is read the way GeoZui4D's
// GdalRaster::Load reads it (GeoZui4D/main/objects/GdalRaster.cpp):
// GDALOpen, GetGeoTransform, GetProjectionRef,
// gz4d::gdal::spatialReferenceForRaster (which falls back to a sidecar .prj
// GDAL could not read), gz4d::geo::Projection and the NODATA comparison.
// The grid and its origin are made up for this test.

#include <gtest/gtest.h>

#include <gz4d/gdal/sidecarPrj.h>
#include <gz4d/geo/Projection.h>
#include <gz4d/ogr/SpatialReference.h>

#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <cpl_conv.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
    // Made-up origin; |lat| != |lon| so a swapped axis order is caught.
    const double originLat = 12.3456;
    const double originLon = -45.6789;
    const double noData = -9999.0;
    const int utmZone = 23;

    // Same form as an ellipsoidal orthographic WKT2 PROJCRS about a local
    // origin (EPSG method 9840), metres east and north.
    std::string localFrameWkt2()
    {
        const std::string deg = "ANGLEUNIT[\"degree\",0.0174532925199433]";
        return
            "PROJCRS[\"Test local frame\","
            "BASEGEOGCRS[\"WGS 84\",DATUM[\"World Geodetic System 1984\","
            "ELLIPSOID[\"WGS 84\",6378137,298.257223563,LENGTHUNIT[\"metre\",1]]],"
            "PRIMEM[\"Greenwich\",0," + deg + "],ID[\"EPSG\",4326]],"
            "CONVERSION[\"Orthographic at the origin\","
            "METHOD[\"Orthographic\",ID[\"EPSG\",9840]],"
            "PARAMETER[\"Latitude of natural origin\",12.3456," + deg + ",ID[\"EPSG\",8801]],"
            "PARAMETER[\"Longitude of natural origin\",-45.6789," + deg + ",ID[\"EPSG\",8802]],"
            "PARAMETER[\"False easting\",0,LENGTHUNIT[\"metre\",1],ID[\"EPSG\",8806]],"
            "PARAMETER[\"False northing\",0,LENGTHUNIT[\"metre\",1],ID[\"EPSG\",8807]]],"
            "CS[Cartesian,2],"
            "AXIS[\"easting (X)\",east,ORDER[1],LENGTHUNIT[\"metre\",1]],"
            "AXIS[\"northing (Y)\",north,ORDER[2],LENGTHUNIT[\"metre\",1]],"
            "USAGE[SCOPE[\"Test grid: metres east and north of the origin\"],"
            "BBOX[12.3356,-45.6889,12.3556,-45.6689]]]";
    }

    // ESRI WKT1 of WGS 84 / UTM zone 23 north, as ArcGIS (and GDAL's
    // morphToESRI) writes it.
    std::string utmEsriWkt1()
    {
        OGRSpatialReference utm;
        utm.SetWellKnownGeogCS("WGS84");
        utm.SetUTM(utmZone, true);
        utm.morphToESRI();
        char *wkt = nullptr;
        utm.exportToWkt(&wkt);
        std::string ret(wkt ? wkt : "");
        CPLFree(wkt);
        return ret;
    }

    class GdalAscTest : public ::testing::Test
    {
    protected:
        fs::path dir;
        fs::path ascPath;
        GDALDataset *ds = nullptr;

        void SetUp() override
        {
            GDALAllRegister();
            auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            dir = fs::temp_directory_path() / ("gz4d_gdal_asc_" + std::to_string(stamp));
            fs::create_directories(dir);
            ascPath = dir / "grid.asc";

            // 4 columns x 3 rows, 10 m cells, lower-left corner 20 m west and
            // 15 m south of the origin; row 1, column 2 is NODATA.
            std::ofstream asc(ascPath);
            asc << "ncols 4\n"
                << "nrows 3\n"
                << "xllcorner -20.0\n"
                << "yllcorner -15.0\n"
                << "cellsize 10.0\n"
                << "NODATA_value -9999\n"
                << "-1.5 -2.5 -3.5 -4.5\n"
                << "-5.5 -6.5 -9999 -8.5\n"
                << "-9.5 -10.5 -11.5 -12.5\n";
        }

        void TearDown() override
        {
            if(ds)
                GDALClose(ds);
            std::error_code ec;
            fs::remove_all(dir, ec);
        }

        void writeSidecar(std::string const &fileName, std::string const &contents)
        {
            std::ofstream prj(dir / fileName);
            prj << contents;
        }

        // Opens the grid as GdalRaster::Load does and checks what does not
        // depend on the CRS.
        void openGrid()
        {
            ds = static_cast<GDALDataset*>(GDALOpen(ascPath.string().c_str(), GA_ReadOnly));
            ASSERT_NE(ds, nullptr);
            EXPECT_STREQ(ds->GetDriver()->GetDescription(), "AAIGrid");
            ASSERT_EQ(ds->GetRasterXSize(), 4);
            ASSERT_EQ(ds->GetRasterYSize(), 3);
            ASSERT_GE(ds->GetRasterCount(), 1);
            // GdalRaster::Load hands byte rasters to the texture loader.
            EXPECT_NE(ds->GetRasterBand(1)->GetRasterDataType(), GDT_Byte);

            double gt[6];
            ASSERT_EQ(ds->GetGeoTransform(gt), CE_None);
            EXPECT_DOUBLE_EQ(gt[0], -20.0);
            EXPECT_DOUBLE_EQ(gt[1], 10.0);
            EXPECT_DOUBLE_EQ(gt[3], 15.0);
            EXPECT_DOUBLE_EQ(gt[5], -10.0);
        }

        std::string datasetWkt() const
        {
            const char *wkt = ds->GetProjectionRef();
            return wkt ? wkt : "";
        }

        // The call GdalRaster::Load makes.
        gz4d::ogr::SpatialReference spatialReference() const
        {
            return gz4d::gdal::spatialReferenceForRaster(datasetWkt(), ascPath);
        }

        // The grid is placed about the made-up origin by the WKT2 local frame.
        void expectLocalFramePlacement(gz4d::ogr::SpatialReference const &sr)
        {
            ASSERT_TRUE(bool(sr));
            EXPECT_FALSE(sr.IsGeographic());

            gz4d::ogr::SpatialReference wgs84;
            wgs84.SetWellKnownGeogCS("WGS84");
            gz4d::geo::Projection toGeo(sr, wgs84);

            // Projection::operator()(y, x) as GdalRaster calls it. With GDAL 3
            // and no axis-mapping strategy set, the WGS84 output is in
            // authority order: [0] is latitude and [1] is longitude.
            gz4d::Point<double> origin = toGeo(0.0, 0.0);
            EXPECT_NEAR(origin[0], originLat, 1e-9);
            EXPECT_NEAR(origin[1], originLon, 1e-9);

            // 100 m east of the origin: same latitude, longitude larger by
            // 100 m over the parallel's radius (nu cos(lat)).
            const double a = 6378137.0, f = 1.0/298.257223563, e2 = f*(2.0-f);
            const double phi = originLat * M_PI / 180.0;
            const double nu = a / std::sqrt(1.0 - e2*std::sin(phi)*std::sin(phi));
            const double dLon = std::asin(100.0 / (nu*std::cos(phi))) * 180.0 / M_PI;
            gz4d::Point<double> east = toGeo(0.0, 100.0);
            EXPECT_NEAR(east[0], originLat, 1e-7);
            EXPECT_NEAR(east[1], originLon + dLon, 1e-7);
        }

        // GdalRaster::Load maps NODATA to NaN with this comparison; it is
        // repeated here because Load cannot be called without the application.
        void expectNoDataBecomesNaN()
        {
            const int cols = ds->GetRasterXSize();
            const int rows = ds->GetRasterYSize();
            GDALRasterBand *band = ds->GetRasterBand(1);
            int hasNodataVal = 0;
            const double noDataValue = band->GetNoDataValue(&hasNodataVal);
            ASSERT_NE(hasNodataVal, 0);
            EXPECT_DOUBLE_EQ(noDataValue, noData);

            std::vector<float> elevation(cols * rows);
            ASSERT_EQ(band->RasterIO(GF_Read, 0, 0, cols, rows, elevation.data(), cols, rows,
                                     GDT_Float32, 0, 0), CE_None);
            EXPECT_FLOAT_EQ(elevation[1*cols + 2], static_cast<float>(noData));
            EXPECT_FLOAT_EQ(elevation[0], -1.5f);

            for(int i = 0; i < cols*rows; ++i)
            {
                float z = elevation[i];
                if(hasNodataVal != 0 && z == static_cast<float>(noDataValue))
                    z = std::numeric_limits<float>::quiet_NaN();
                if(i == 1*cols + 2)
                    EXPECT_TRUE(std::isnan(z)) << "cell " << i;
                else
                    EXPECT_TRUE(std::isfinite(z)) << "cell " << i;
            }
        }
    };
}

TEST_F(GdalAscTest, Wkt2PrjUsesFallback)
{
    writeSidecar("grid.prj", localFrameWkt2());
    ASSERT_NO_FATAL_FAILURE(openGrid());

    // GDAL 3.8's AAIGrid driver reads a .prj as ESRI WKT1 only, so it gives
    // no CRS for a WKT2 .prj; that is why the fallback exists. A later GDAL
    // may read it itself; the helper's result must be right either way.
    if(!datasetWkt().empty())
        RecordProperty("note", "GDAL read the WKT2 .prj itself");

    testing::internal::CaptureStderr();
    gz4d::ogr::SpatialReference sr = spatialReference();
    const std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, "");

    expectLocalFramePlacement(sr);
    expectNoDataBecomesNaN();
}

TEST_F(GdalAscTest, Wkt1PrjReadByGdal)
{
    const std::string wkt1 = utmEsriWkt1();
    ASSERT_FALSE(wkt1.empty());
    writeSidecar("grid.prj", wkt1);
    ASSERT_NO_FATAL_FAILURE(openGrid());

    // GDAL reads an ESRI WKT1 .prj itself; the helper uses its result.
    EXPECT_FALSE(datasetWkt().empty());

    testing::internal::CaptureStderr();
    gz4d::ogr::SpatialReference sr = spatialReference();
    const std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, "");

    ASSERT_TRUE(bool(sr));
    EXPECT_FALSE(sr.IsGeographic());

    // Compared with the CRS the test wrote, not with GDAL's own string, so
    // the round trip through the .prj is what is checked.
    gz4d::ogr::SpatialReference expected;
    expected.SetWellKnownGeogCS("WGS84");
    expected.SetUTM(utmZone, true);
    EXPECT_TRUE(sr.IsSame(expected));

    expectNoDataBecomesNaN();
}

TEST_F(GdalAscTest, NoPrjHandledCleanly)
{
    ASSERT_NO_FATAL_FAILURE(openGrid());
    EXPECT_EQ(datasetWkt(), "");

    testing::internal::CaptureStderr();
    gz4d::ogr::SpatialReference sr;
    EXPECT_NO_THROW(sr = spatialReference());
    const std::string err = testing::internal::GetCapturedStderr();

    EXPECT_FALSE(bool(sr));
    EXPECT_EQ(err, "");
}

TEST_F(GdalAscTest, UnparsablePrjReported)
{
    writeSidecar("grid.prj", "this is not a coordinate reference system");
    ASSERT_NO_FATAL_FAILURE(openGrid());

    testing::internal::CaptureStderr();
    gz4d::ogr::SpatialReference sr = spatialReference();
    const std::string err = testing::internal::GetCapturedStderr();

    EXPECT_FALSE(bool(sr));
    // GDAL/PROJ may print their own text too; the helper's message names
    // the file.
    EXPECT_NE(err.find("grid.prj"), std::string::npos) << err;
}

TEST_F(GdalAscTest, UppercasePrj)
{
    writeSidecar("grid.PRJ", localFrameWkt2());
    if(fs::exists(dir / "grid.prj"))
        GTEST_SKIP() << "case-insensitive file system: .PRJ and .prj are the same file";

    ASSERT_NO_FATAL_FAILURE(openGrid());
    if(!datasetWkt().empty())
        RecordProperty("note", "GDAL read the WKT2 .PRJ itself");

    gz4d::ogr::SpatialReference sr = spatialReference();
    expectLocalFramePlacement(sr);
}
