#ifndef OGR_LAYER_H
#define OGR_LAYER_H

#include "engine/SimpleObjectCollection.h"
#include <gz4d/ogr.h>
#include "objects/ogr/OGR_Limits.h"

class OGR_Layer : public SimpleObjectCollection
{
    GLuint displayList;
    public:
        OGR_Layer():displayList(0){}
        OGR_Layer(gz4d::ogr::Layer l, OGR_Limits const &limits, float minSpan = 0.0);

        DERIVE_OBJECT(OGR_Layer);
        void DrawLowRes(float degradation, Window3D *win);

};

#endif
