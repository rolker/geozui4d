#include <gz4d/data/Grib2/File.h>
#include <gz4d/util/Xml.h>
#include <string>
#include <algorithm>
#include <gz4d/data/Image.h>
#include <gz4d/ColorMap.h>

using namespace gz4d;
using namespace gz4d::xml;

std::string GetProp(Node const &n, std::string const &prop, int argc, char *argv[])
{
    Attribute a = n.GetFirstAttribute(prop);
    if(a)
    {
        std::string ret = a.Value();
        if(ret[0] == '@')
        {
            int argno = boost::lexical_cast<int>(ret.substr(1));
            argno += 1; // first arg is argv[2] so @1 maps to 2
            if (argno < argc)
                ret = argv[argno];
            else
                throw(gz4d::Exception("config file expects more parameters than are supplied"));
        }
        return ret;
    }
    throw(gz4d::Exception("attribute missing"));
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cerr << "usage: grib2image config.xml [param1] [param2] ...\n";
        exit(1);
    }

    Doc doc;
    doc.ParseFile(argv[1]);
    Node root = doc.GetFirstNode("Grib2Image");

    std::map<std::string,ColorMap> maps;
    for(Node n = root.GetFirstNode("ColorMap"); n; n.Next("ColorMap"))
    {
        Attribute a = n.GetFirstAttribute("label");
        if(a)
            maps[a.Value()] = ColorMap(n);
    }

    for(Node n = root.GetFirstNode("Image"); n; n.Next("Image"))
    {
        std::string fname = GetProp(n,"filename",argc,argv);
        grib2::File grib(GetProp(n,"grib",argc,argv));

        ColorMap map = maps[GetProp(n,"colormap",argc,argv)];

        int message = 0;
        int parameter = 0;
        try
        {
            message = boost::lexical_cast<int>(GetProp(n,"message",argc,argv));
            parameter = boost::lexical_cast<int>(GetProp(n,"parameter",argc,argv));
        }
        catch(gz4d::Exception)
        {}

        double offset = 0.0;
        double scale = 1.0;

        try
        {
            offset = boost::lexical_cast<double>(GetProp(n,"mapOffset",argc,argv));
        }
        catch(gz4d::Exception)
        {}

        try
        {
            scale = boost::lexical_cast<double>(GetProp(n,"mapScale",argc,argv));
        }
        catch(gz4d::Exception)
        {}

        map.SetOffset(offset);
        map.SetScale(scale);

        gz4d::grib2::Parameter::Ptr p = (*grib[message])[parameter];
        gz4d::grib2::GridDefinition grid(p->GetGridDefinition());

        std::vector<float> data;
        p->GetData(data);

        float min = *std::min_element(data.begin(),data.end());
        float max = *std::max_element(data.begin(),data.end());

        float mapMin = min;
        float mapMax = max;

        try
        {
            mapMin = boost::lexical_cast<float>(GetProp(n,"mapMin",argc,argv));
        }
        catch(gz4d::Exception)
        {}

        try
        {
            mapMax = boost::lexical_cast<float>(GetProp(n,"mapMax",argc,argv));
        }
        catch(gz4d::Exception)
        {}

        scale = (mapMax-mapMin)/(max-min);
        offset = (mapMin/scale)-min;

        Image image;
        image.GetRaster()->Resize(grid.cols,grid.rows);


        for(int row = 0; row < grid.rows; ++row)
            for(int col = 0; col < grid.cols; ++col)
            {
                unsigned long dataIndex;
                if(grid.rowMajor)
                {
                    if(grid.yIncreases)
                    {
                        dataIndex = row*grid.cols;
                    }
                    else
                    {
                        dataIndex = (grid.rows-1-row)*grid.cols;
                    }
                    if((grid.xIncreases && !grid.scanDirectionAlternates) || (grid.scanDirectionAlternates && (grid.xIncreases != (row&0x1) )))
                    {
                        dataIndex += col;
                    }
                    else
                    {
                        dataIndex += grid.cols-1-col;
                    }
                }
                else
                {
                    if(grid.xIncreases)
                    {
                        dataIndex = col*grid.rows;
                    }
                    else
                    {
                        dataIndex = (grid.cols-1-col)*grid.rows;
                    }
                    if((grid.yIncreases && !grid.scanDirectionAlternates) || (grid.scanDirectionAlternates && (grid.yIncreases != (col&0x1) )))
                    {
                        dataIndex += row;
                    }
                    else
                    {
                        dataIndex += grid.rows-1-row;
                    }
                }
                image.GetRaster()->GetRow(row)[col] = map[(data[dataIndex]+offset)*scale];
            }
        image.Save(fname);
    }
    return 0;
}
