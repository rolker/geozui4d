// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef S57_BRIDGE_H
#define S57_BRIDGE_H

#include"S57Layer.h"
#include"S57_object.h"

class Scene3D;

class S57_bridge : public S57_object
{
public:
	S57_bridge();
	~S57_bridge();
	DERIVE_OBJECT(S57_bridge);

	virtual void DrawLowRes(float degradation, Window3D * win);

	virtual void SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber);
	virtual void ShowS57Info(Scene3D *scene);

private:
	enum BridgeCategory {Unknown, Fixed, Opening, Swing, Lifting, Bascule, Pontoon, Drawbridge,
						 Transporter, Footbridge, Viaduct, Aqueduct, Suspension};

	int category;
	float verticalClearanceClosed, verticalClearanceOpen;

	static const char *bridgeCategoryString[];
};


#endif
