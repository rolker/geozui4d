// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef S57_AREA_H
#define S57_AREA_H

#include"S57Layer.h"
#include"S57_object.h"

class S57_area : public S57_object
{
public:
	S57_area();
	~S57_area();
	DERIVE_OBJECT(S57_area);
	virtual void InitializeFor(const char *acronym);
	virtual void SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber);
	virtual void ShowS57Info(Scene3D *scene);

private:
    enum {NONE, BERTHS, CTNARE, RESARE, DRGARE, ISTZNE, OFSPLF, PRCARE, PYLONS, UNSARE};
    int type;
    double value;
    int restriction;
    int category;
};


#endif
