// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef MPITEMBASE_H
#define MPITEMBASE_H

#include "engine/ObjectCollection.h"
#include "engine/Interactor.h"

class MPItemBase: public ObjectCollection, public Interactor
{
public:
	MPItemBase();
    MPItemBase(int id, gz4d::xml::Node cmd, int pickType = PICK_CLICK1 | PICK_PRESS3);
    virtual void command(gz4d::xml::Node c, std::string const &cmd);
    int HandleInput(unsigned int inputType, InputInfo *info);
protected:
    void init();

	int itemID;
	float color[4];
	float selectedColor[4];
	float size;
	bool inConstructor;

};

#endif
