// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef S57LAYER_H
#define S57LAYER_H

#include<gz4d/glo.h>
#include<gz4d/ogr.h>
#include"engine/ObjectCollection.h"

#include <map>
class S57_object;
struct S57_LayerInfo;

class S57Layer : public ObjectCollection
{
public:
    typedef std::map<std::string, S57_object *> LnamTable;
    S57Layer(gz4d::ogr::Layer l, bool publishObjects, LnamTable &lnam_table);
    ~S57Layer();
    int GetObjectCount(){return objectCount;};
protected:
    void DumpAttributes(gz4d::ogr::Feature item) const;

    int objectCount;
};


#endif
