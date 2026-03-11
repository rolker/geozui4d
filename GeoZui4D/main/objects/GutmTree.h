// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef GUTMTREE_H
#define GUTMTREE_H

#include "engine/ObjectCollection.h"
#include "utilities/TimeSequence.h"

#define TREE_NODE_SIZE 10
#define GUTM_SIZE 100

class Gutm;
/**GutmTree performs a similar function as the quadtree, but uses gutms for rendering.
  *@author roland
  */
  
struct NodeType
{
	Gutm * gutm;
	NodeType * next;
};

class GutmTree : public ObjectCollection
{
public: 
	GutmTree(bool altData = false);
	~GutmTree();
        void Initialize(std::string pt, double resolution);
	void AddPoint(Point3d,double radius, double t=std::numeric_limits<double>::quiet_NaN(), float alt=std::numeric_limits<double>::quiet_NaN());
	void SaveObject(int id, char * fn);

private:
	NodeType * gutms;
	NodeType * gcursor;
	
	Gutm * GetGutm(double x, double y, int reset = 1);
	void AddGutm(double x, double y);
	
	double res;
        std::string ptype;
    bool hasAltData; ///< Flag indicating if alternate data will be used in addition to 3D points
};



#endif
