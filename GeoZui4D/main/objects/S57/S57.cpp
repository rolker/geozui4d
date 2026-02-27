#include "GZincludes.h"
#include"S57.h"
#include <boost/filesystem.hpp>
#include "S57_object.h"


#include"S57Layer.h"
/*#include"S57_LIGHTS.h"
#include"S57_LNDARE.h"
#include"S57_BRIDGE.h"
#include"S57_BOY.h"
#include"S57_BUISGL.h"*/


using namespace std;

REGISTER_OBJECT(S57,"000");

extern MainWindow *mainWindow;

float S57::dim = 1.0;

S57::S57()
: ObjectCollection(GL_TRUE, 0) //dont make the collection pickable
{
    publishObjects = true;
}

S57::~S57()
{
}

Object * S57::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object::LoadXML(xml,path,ncfile);

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.publishObjects"))
        publishObjects = gz4d::IsTrue(*attribute);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.encfile"))
        LoadS57((bfs::path(path)/(*attribute)).string());

    dim = xml.get("<xmlattr>.dim",dim);

    return this;
}

int S57::Load()
{
    if(!GetFilename().empty())
    {
        return LoadS57(GetFilename());
    }
    else
    {
        return 1;
    }
}

int S57::LoadS57(std::string const &s57file)
{
/*    OGRLayer *layer;
    int i;

    RegisterOGRS57();
    enc = OGRSFDriverRegistrar::Open(s57file.string().c_str());*/
    gz4d::ogr::DataSource enc(s57file);
    
    std::string fn = bfs::path(s57file).filename().string();
    if(!enc)
    {
        std::cerr << "Error opening: " << fn << std::endl;
        return 1;
    }
    int layerCount = enc.size();
    std::cout  << fn << std::endl << layerCount << " layers\n";
/*    for (i = 0; i < layerCount; i++)
    {
        layer = enc->GetLayer(i);
        if(!strcmp(layer->GetLayerDefn()->GetName(),"M_COVR"))
        {
            extractGeoRef(layer);
        }
    }*/
    for (int i = 0; i < layerCount; ++i)
    {
        extractLayer(enc[i]);
    }


    for (LnamTable::iterator obIter = lnam_table.begin(); obIter != lnam_table.end(); obIter++ )
        if(obIter->second)
            obIter->second->resolveReferences(lnam_table, bfs::path(s57file).parent_path().string());

    return 0;
}


void S57::extractLayer(gz4d::ogr::Layer layer)
{
    S57Layer * s57layer;

    s57layer = new S57Layer(layer, publishObjects, lnam_table);
    if(s57layer)
    {
        if(s57layer->GetObjectCount())
            AddObject(s57layer);
        else
            delete s57layer;
    }
}

int S57::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    if (argc >= 1)
    {
        std::string command(argv[0]);
        if (command == "dim")
        {
            if (argc > 2)
            {
                Tcl_SetResult(interp, const_cast<char*>("usage: dim [value]"), TCL_VOLATILE);
                return TCL_ERROR;
            }
            if (argc == 2)
            {
                dim = atof(argv[1]);
            }
            Tcl_SetResult(interp, const_cast<char*>(boost::lexical_cast<std::string>(dim).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
    }
    return ObjectCollection::perform( argc, argv, interp);

}


void S57::getHtmlInfo(std::stringstream &html, short level)
{
    html << "<h" << level << ">S57</h" << level << ">";
    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/dim/dimVal\">Dim <input type=\"text\" name=\"dimVal\" value=\"" << dim <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";
    ObjectCollection::getHtmlInfo(html,level);
}
