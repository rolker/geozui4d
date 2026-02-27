/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef CURRENTLAYER_H
#define CURRENTLAYER_H

#define ARROW_SEGMENTS 7
#define ARROW_BOUNDARIES 8

#include "engine/ObjectCollection.h"
#include "engine/Interactor.h"
#include"Floater.h"

class Current;
class ImageMap;

class CurrentLayer : public ObjectCollection, public Interactor
{
public:
	CurrentLayer(Current * c = NULL);
	~CurrentLayer();
	DERIVE_OBJECT(CurrentLayer);

	Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
        int loadFromNetCDF(gz4d::netcdf::File ncfile);
	int DrawHighRes(int eye, Window3D * win);
	void DrawLowRes(float degradation, Window3D * win);
	void Idle(double time);
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);
	int HandleInput(unsigned int inputType, InputInfo * info);
	GLboolean getInterpolatedCurrent(float x, float y, double time, float &u, float &v);
	float getZ(int r, int c, int t);
	float getZ(float i, float j, int t, GLboolean includesGeoRef = GL_TRUE);
	float getZ(float i, float j, double t);
	int getTimestep(double t);

		//attribute stuff
	int getAttribute(SmartValue &val, int attributeIndex);
	int setAttribute(SmartValue &val, int attributeIndex);
	
protected:
	void setChildrenFloaterTailLength();
	void DrawBoringArrows();
	void DrawMipmappedArrows(float degradation, Window3D *win);
	void DrawMipmappedArrowsOnTheFly(float degradation, Window3D *win);
	void DrawMipmapVectors(int starti, int startj, int mipmapLevel, int t1, int t2, float p, int count, float size);
	GLboolean calcUV(float &newU, float &newV, float x, float y);
	void createVectorMipmap(int mipmapFactor);
	void destroyVectorMipmap();
	void generateSimpleArrowMipmap(int t, int mipmapFactor);
	void generateMagnitudeBiasedMipmap(int t, int mipmapFactor);
	void generateArrowFor(int t, int mipmapFactor, int level, int cellx, int celly);
	
	Current * current;

	double * times;
	
	float sigma;
	int sigmaIndex;

	float ***u;
	float ***v;
	float scale;
	int timesteps;
	int currentTimestep;

	float	cellsize;
	int rows,cols;

	float headColor[3];
	float tailColor[3];
	float arrowColor[ARROW_BOUNDARIES][3];
	float lineWidth;

	float spacing;
	
	int addingManyFloaters;
	int floaterTailLength;
	
	struct FloaterNodeType
	{
		double x,y;
		Floater * floater;
	};

	FloaterNodeType * newFloaters();
	
	int floaterGrid; //flag to indicate that we want a uniform grid of floaters rather than a normal distribution
	
	Floater * pendingFloater;
	FloaterNodeType * pendingFloaters;
	
	GLdouble cursorLocation[2]; // where is the mouse cursor?

	// the streak mode stuff

	void genPts(int sample); 
	void trace(int s,int t);
	void drawStreaks();
	int nTraces;
	float *XstartPos, *YstartPos, *ZstartPos;
	float **Xpaths;
	float **Ypaths;
	float **Zpaths;
	int deltaTime;
	int *age;
	int skipCells;  // this is the grid skip factor

	enum CurrentLayerDisplayTypes {display_streaks, display_arrows, display_tex_arrows} displayType;

	enum CurrentLayerAttributes {Attr_FloaterTailLength = NumObjectAttributes, NumCurrentLayerAttributes};

	std::shared_ptr<ImageMap> tex;

	// vector mipmap
	struct VectorMipmapCell {
		//average info
		float u;
		float v;

		//representation info
//		Point2f *arrowPoints;
//		int numPoints;
//		Point2f arrowHead[3];
	};
	
	struct VectorMipmapLevel {
		VectorMipmapCell **cell; //cell[yindex][xindex]
		int maxDim; //max x or y
		int stepSize;
		float cellSize;
	} **vectorMipmap; //vectorMipmap[timestep][level].cell[yindex][xindex]
	int numMipmapLevels;
	static int commonMipmapFactor;

};

#endif
