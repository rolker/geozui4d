#ifndef S57_BCN_H
#define S57_BCN_H

#include"S57Layer.h"
#include"S57_object.h"
#include <vector>

class S57_supplemental;

// handles BCN* objects
class S57_beacon : public S57_object
{
    public:
        S57_beacon();
        ~S57_beacon();
        DERIVE_OBJECT(S57_beacon);

        virtual void SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber);
        virtual void resolveReferences(LnamTable &lnam_table, std::string const &s57file);
        virtual void ShowS57Info(Scene3D *scene);
        virtual void ShowS57ShapeInfo(Scene3D *scene);

    protected:
        enum {SHP_DEFAULT, SHP_POST, SHP_WITHY, SHP_TOWER, SHP_LATTICE, SHP_PILE, SHP_CAIRN, SHP_BUOYANT};
        int shape;
        std::vector<S57_supplemental *> children;
};


#endif
