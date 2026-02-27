#ifndef S57_H
#define S57_H

#include "engine/ObjectCollection.h"
#include <map>
class S57_object;

class S57_bridge;
class S57_object;
class S57_buoy;
class S57_sounding;

class S57 : public ObjectCollection
{
    public:
        S57();
        ~S57();

        DERIVE_OBJECT(S57);

        Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
        int Load();
        int LoadS57(std::string const &s57file);
        int perform(int argc, const char **argv, Tcl_Interp *interp);
        void getHtmlInfo(std::stringstream &html, short level);

        static float dimFactor() {return dim;}

    private:
        void extractLayer(gz4d::ogr::Layer layer);

        bfs::path encfile;

        typedef std::map<std::string, S57_object *> LnamTable;
        LnamTable lnam_table;

        bool publishObjects; //publish both Objects and layers, or just layers?
        static float dim; ///< Allows to reduce the charts color intensity.
};

#endif
