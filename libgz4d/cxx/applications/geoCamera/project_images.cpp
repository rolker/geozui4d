#include <gz4d/nav/Track.h>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <gz4d/vector/ios.h>
#include <gz4d/Image.h>
#include <gz4d/Exif.h>
#include <gz4d/geo/WGS84.h>
#include <gz4d/geo/LocalENU.h>
#include <cmath>
#include <cassert>
#include <gz4d/geo/Projection.h>
#include <gz4d/util/Xml.h>
#include <gz4d/Rotation.h>

struct PTU
{
    gz4d::Time time;
    double pan;
    double tilt;
    boost::filesystem::path image;

    PTU(boost::filesystem::path fn)
    {
        std::cout << fn << std::endl;
        std::ifstream in(fn.string().c_str());
        std::string label;
        in >> label >> pan >> label >> tilt;
        in >> time;

        pan = gz4d::Radians(pan);
        tilt = gz4d::Radians(tilt);

        std::ostringstream ss;
        ss << time << "_canon.jpg";
        image = fn.parent_path()/ss.str();
    }
};

gz4d::Point<double> hto3(gz4d::Vector<double,4> const &h)
{
    return gz4d::Point<double>(h[0]/h[3],h[1]/h[3],h[2]/h[3]);
}


int main(int argc, char * argv[])
{
    if(argc != 2)
    {
        std::cerr << "usage: project_images imagery.xml" << std::endl;
        exit(1);
    }

    gz4d::xml::Doc d;
    d.ParseFile(argv[1]);

    gz4d::xml::Node r = d.GetFirstNode("GeoImagery");

    gz4d::xml::Node n = r.GetFirstNode("Navigation");
    gz4d::xml::Attribute a = n.GetFirstAttribute("file");
    boost::filesystem::path nav = a.Value();

    std::cout << "reading nav" << std::endl;
    gz4d::nav::Track track(nav);
    std::cout << "done" << std::endl;

    gz4d::xml::Node c = r.GetFirstNode("Configuration");
    gz4d::xml::Node cam = c.GetFirstNode("Camera");
    a = cam.GetFirstAttribute("x");
    double x = a.Value<double>();
    a = cam.GetFirstAttribute("y");
    double y = a.Value<double>();
    a = cam.GetFirstAttribute("z");
    double z = a.Value<double>();

    gz4d::Translation<double> camera_position(-x,-z,y);

    gz4d::xml::Node wl = c.GetFirstNode("Waterline");
    a = wl.GetFirstAttribute("z");
    z = a.Value<double>();

    gz4d::Translation<double> cog(0.0,z,0.0);

    gz4d::xml::Node o = r.GetFirstNode("Ortho");
    a = o.GetFirstAttribute("resolution");
    double resolution = a.Value<double>();
    a = o.GetFirstAttribute("max_distance");
    double radius = a.Value<double>();
    a = o.GetFirstAttribute("path");
    boost::filesystem::path out_path = a.Value();

    if (!exists(out_path))
        boost::filesystem::create_directory(out_path);

    for(gz4d::xml::Node is = r.GetFirstNode("ImageSet"); is; is.Next("ImageSet"))
    {
        a = is.GetFirstAttribute("label");
        std::cout << "Image set: " << a.Value() << std::endl;
        boost::filesystem::path out = a.Value()+".tif";
        a = is.GetFirstAttribute("path");
        boost::filesystem::path path = a.Value();

        std::vector<PTU> files;
        for(gz4d::xml::Node i = is.GetFirstNode("Image"); i; i.Next("Image"))
        {
            a = i.GetFirstAttribute("file");
            boost::filesystem::path ptu = a.Value().substr(0,a.Value().size()-10)+"_ptu.txt";
            ptu = path/ptu;
            files.push_back(PTU(ptu));
        }
        
        gz4d::Box<double,2> bbox = track.GetBox(gz4d::Time::Span(files[0].time,files.rbegin()->time));
        std::cout << std::fixed << std::setprecision(6) << bbox.getMin() << "\t" << bbox.getMax() << std::endl;
        
        gz4d::geo::LocalENU<> ref_point(gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon>(bbox.getMin()[0],bbox.getMin()[1],0.0));
        gz4d::Point<double> nav_offset = ref_point(gz4d::geo::Point<double,gz4d::geo::WGS84::ECEF>(gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon>(bbox.getMax()[0],bbox.getMax()[1],0.0)));
        
        gz4d::Vector<int,2> anchor(radius/resolution,radius/resolution);
        gz4d::Vector<int,2> img_size = anchor*2+gz4d::Vector<int,2>(nav_offset[0]/resolution,nav_offset[1]/resolution);
        
        int zone = gz4d::geo::UTMZone(bbox.getMin()[1]);
        bool n = (bbox.getMin()[0] >= 0.0);
        gz4d::geo::Projection proj = gz4d::geo::UTMProjection(zone,n);
        gz4d::Point<double> ref_utm = proj(bbox.getMin()[0],bbox.getMin()[1]);
        
        boost::filesystem::path tfw = out_path/out;
        
        std::string e = boost::algorithm::to_lower_copy(boost::filesystem::extension(out));
        if(e == ".png")
            tfw.replace_extension(".pgw");
        else if (e == ".tif")
            tfw.replace_extension(".tfw");
        else if (e == ".jpg")
            tfw.replace_extension(".jgw");
        else
            tfw.replace_extension(boost::filesystem::extension(out)+"w");
        
        std::cout << "geo file: " << tfw.string() << std::endl;
        std::ofstream tfw_file(tfw.string().c_str());
        
        tfw_file << resolution << std::endl;
        tfw_file << 0.0 << std::endl;
        tfw_file << 0.0 << std::endl;
        tfw_file << -resolution << std::endl;
        tfw_file << ref_utm[0] - anchor[0]*resolution << std::endl;
        tfw_file << ref_utm[1] + (img_size[1]-anchor[1])*resolution << std::endl;
        
        gz4d::Color b = gz4d::Black();
        b[3] = 0.0;
        gz4d::Raster<gz4d::Color> out_raster(img_size[0],img_size[1],b);
        
        for(std::vector<PTU>::iterator p = files.begin(); p != files.end(); ++p)
        {
            std::cout << "Image " << p->image.string() << " at time: " << p->time << std::endl;
            gz4d::Exif exif(p->image);

            double f = exif.GetFloat("exif","FocalLength")/2.0;

            int x = exif.GetShort("exif","PixelXDimension");
            int y = exif.GetShort("exif","PixelYDimension");

            float xr = exif.GetFloat("exif","FocalPlaneXResolution")/ 0.0254;
            float yr = exif.GetFloat("exif","FocalPlaneYResolution")/ 0.0254;

            float w = 500.0*x/xr;
            float h = 500.0*y/yr;

            gz4d::Matrix<double,4,4> projection;
            projection(0,0) = 2.0*f/w;
            projection(0,2) = 0.0;
            projection(1,1) = 2.0*f/h;
            projection(1,2) = 0.0;
            projection(2,2) = -(radius+f)/(radius-f);
            projection(2,3) = -(2.0*radius*f)/(radius-f);
            projection(3,2) = -1.0;

            gz4d::Rotation<double> pan;
            pan.Set(-p->pan,gz4d::Point<double>(0.0,1.0,0.0));

            gz4d::Rotation<double> tilt;
            tilt.Set(-p->tilt,gz4d::Point<double>(1.0,0.0,0.0));

            gz4d::ImageRGB8 in_image;
            in_image.Load(p->image);

            gz4d::nav::Fix::Ptr fix = track(p->time);

            gz4d::Point<double> img_offset = ref_point(gz4d::geo::Point<double,gz4d::geo::WGS84::ECEF>(gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon>(fix->latitude,fix->longitude,0.0)));
            gz4d::Vector<int,2> img_ref = anchor+gz4d::Vector<int,2>(img_offset[0]/resolution,img_offset[1]/resolution);

            gz4d::Rotation<double> heading;
            heading.Set(gz4d::Radians(double(fix->heading)),gz4d::Point<double>(0.0,1.0,0.0));

            gz4d::Rotation<double> pitch;
            pitch.Set(gz4d::Radians(double(fix->pitch)),gz4d::Point<double>(1.0,0.0,0.0));

            gz4d::Rotation<double> roll;
            pitch.Set(gz4d::Radians(double(fix->roll)),gz4d::Point<double>(0.0,0.0,1.0));

            gz4d::Matrix<double,4,4> ptpcrph = projection*tilt.GetMatrix()*pan.GetMatrix()*camera_position.GetMatrix()*roll.GetMatrix()*pitch.GetMatrix()*heading.GetMatrix()*cog.GetMatrix();

            gz4d::Vector<double,4> v(0.0,0.0,0.0,1.0);


            for(int i = 0; i < img_size[1]; ++i)
            {
                v[2] = -(i-img_ref[1])*resolution;
                for(int j = 0; j < img_size[0]; ++j)
                {
                    v[0] = (j-img_ref[0])*resolution;

                    double ship_heading = gz4d::Radians(double(fix->heading));
                    double bearing = atan2(j-img_ref[0],double(i-img_ref[1]));
                    double p_bearing = bearing;
                    while(p_bearing < 0.0)
                        p_bearing += 2.0*M_PI;
                    if(fabs(p_bearing-ship_heading)<=M_PI/2.0 || fabs(p_bearing-ship_heading)>= 3.0*M_PI/2.0)
                    {
                        gz4d::Vector<double,4> cv = ptpcrph*v;
                        if (cv[3] > 0)
                        {
                            gz4d::Point<double> cp = hto3(cv);
                            if (cp[0] > -1.0 && cp[0] < 1.0 && cp[1] > -1.0 && cp[1] < 1.0 && cp[2] < 1.0 && cp[2] > 0.0)
                            {
                                int ii = ((cp[0] +1.0)/2.0)*in_image.GetRaster().GetColumns();
                                int ij = ((cp[1] +1.0)/2.0)*in_image.GetRaster().GetRows();

                                gz4d::Color c = in_image.GetRaster().GetRow(ij)[ii];
                                out_raster.GetRow(i)[j] = c;
                            }
                        }
                    }

                }
            }
        }
        gz4d::ImageRGBA8 out_image;
        out_image.Resize(img_size[0],img_size[1]);
        for(int r = 0; r < img_size[1]; ++r)
            for(int c = 0; c < img_size[0]; ++c)
            {
                if(out_raster.GetRow(r)[c][3] > 0.0)
                    out_raster.GetRow(r)[c] /= out_raster.GetRow(r)[c][3];
                out_image.GetRaster().GetRow(r)[c] = out_raster.GetRow(r)[c];
            }
        out_image.Save(out_path/out);
    }



/*    if(argc < 7)
    {
        std::cerr << "usage: project_images radius resolution height nav.nmea out.tif *_ptu.txt" << std::endl;
        exit(1);
    }
    double radius = boost::lexical_cast<double>(argv[1]);
    double resolution  = boost::lexical_cast<double>(argv[2]);
    double height = boost::lexical_cast<double>(argv[3]);
    boost::filesystem::path nav = argv[4];
    boost::filesystem::path out = argv[5];

    std::cout << "reading nav" << std::endl;
    gz4d::nav::Track track(nav);
    std::cout << "done" << std::endl;

    std::vector<PTU> files;
    for(int i = 6; i < argc; ++i)
        files.push_back(PTU(argv[i]));

    gz4d::Box<double,2> bbox = track.GetBox(gz4d::Time::Span(files[0].time,files.rbegin()->time));
    std::cout << std::fixed << std::setprecision(6) << bbox.getMin() << "\t" << bbox.getMax() << std::endl;

    gz4d::geo::LocalENU<> ref_point(gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon>(bbox.getMin()[0],bbox.getMin()[1],0.0));
    gz4d::Point<double> nav_offset = ref_point(gz4d::geo::Point<double,gz4d::geo::WGS84::ECEF>(gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon>(bbox.getMax()[0],bbox.getMax()[1],0.0)));

    gz4d::Vector<int,2> anchor(radius/resolution,radius/resolution);
    gz4d::Vector<int,2> img_size = anchor*2+gz4d::Vector<int,2>(nav_offset[0]/resolution,nav_offset[1]/resolution);

    int zone = gz4d::geo::UTMZone(bbox.getMin()[1]);
    bool n = (bbox.getMin()[0] >= 0.0);
    gz4d::geo::Projection proj = gz4d::geo::UTMProjection(zone,n);
    gz4d::Point<double> ref_utm = proj(bbox.getMin()[0],bbox.getMin()[1]);

    boost::filesystem::path tfw = out;

    std::string e = boost::algorithm::to_lower_copy(boost::filesystem::extension(out));
    if(e == ".png")
        tfw.replace_extension(".pgw");
    else if (e == ".tif")
        tfw.replace_extension(".tfw");
    else if (e == ".jpg")
        tfw.replace_extension(".jgw");
    else
        tfw.replace_extension(boost::filesystem::extension(out)+"w");

    std::cout << "geo file: " << tfw.string() << std::endl;
    std::ofstream tfw_file(tfw.string().c_str());

    tfw_file << resolution << std::endl;
    tfw_file << 0.0 << std::endl;
    tfw_file << 0.0 << std::endl;
    tfw_file << -resolution << std::endl;
    tfw_file << ref_utm[0] - anchor[0]*resolution << std::endl;
    tfw_file << ref_utm[1] + (img_size[1]-anchor[1])*resolution << std::endl;

    gz4d::Color b = gz4d::Black();
    b[3] = 0.0;
    gz4d::Raster<gz4d::Color> out_raster(img_size[0],img_size[1],b);

    for(std::vector<PTU>::iterator p = files.begin(); p != files.end(); ++p)
    {
        std::cout << p->time << std::endl;
        std::cout << p->image.string() << std::endl;
        gz4d::Exif exif(p->image);
        gz4d::Vector< float, 2 > fov = exif.GetFOV();

        fov[0] = gz4d::Radians(fov[0]);
        fov[1] = gz4d::Radians(fov[1]);

        gz4d::ImageRGB8 in_image;
        in_image.Load(p->image);

        gz4d::nav::Fix::Ptr fix = track(p->time);

        gz4d::Point<double> img_offset = ref_point(gz4d::geo::Point<double,gz4d::geo::WGS84::ECEF>(gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon>(fix->latitude,fix->longitude,0.0)));
        gz4d::Vector<int,2> img_ref = anchor+gz4d::Vector<int,2>(img_offset[0]/resolution,img_offset[1]/resolution);

        double ship_heading = gz4d::Radians(fix->heading);
        double img_heading = p->pan + ship_heading;

        for(int i = 0; i < img_size[1]; ++i)
            for(int j = 0; j < img_size[0]; ++j)
            {
                double d = sqrt( (i-img_ref[1])*(i-img_ref[1]) + (j-img_ref[0])*(j-img_ref[0]))*resolution;
                if(d <= radius)
                {
                    double bearing = atan2(j-img_ref[0],double(i-img_ref[1]));
                    double p_bearing = bearing;
                    while(p_bearing < 0.0)
                        p_bearing += 2.0*M_PI;
                    if(fabs(p_bearing-ship_heading)<=M_PI/2.0 || fabs(p_bearing-ship_heading)>= 3.0*M_PI/2.0)
                    {
                        double elevation = atan2(cos(bearing-img_heading)*d,height);

                        if(elevation > 0.0)
                        {
                            int in_i = in_image.GetRaster().GetRows()*(1.0+tan(elevation-p->tilt)/tan(fov[1]/2.0))/2.0;
                            int in_j = in_image.GetRaster().GetColumns()*(1.0+tan(bearing-img_heading)/tan(fov[0]/2.0))/2.0;

                            if(in_i >= 0 && in_i < in_image.GetRaster().GetRows() && in_j >= 0 && in_j < in_image.GetRaster().GetColumns())
                            {
                                gz4d::Color c = in_image.GetRaster().GetRow(in_i)[in_j];
                                out_raster.GetRow(i)[j] = c;
                            }
                        }
                    }
                }
            }
    }

    gz4d::ImageRGBA8 out_image;
    out_image.Resize(img_size[0],img_size[1]);
    for(int r = 0; r < img_size[1]; ++r)
        for(int c = 0; c < img_size[0]; ++c)
        {
            if(out_raster.GetRow(r)[c][3] > 0.0)
                out_raster.GetRow(r)[c] /= out_raster.GetRow(r)[c][3];
            out_image.GetRaster().GetRow(r)[c] = out_raster.GetRow(r)[c];
        }
    out_image.Save(out);*/
    return 0;
}
