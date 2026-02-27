#ifndef S57_BOY_H
#define S57_BOY_H

#include"S57Layer.h"
#include"S57_beacon.h"

class Glyph;
class DynamicTides;

// handles BOY* objects
class S57_buoy : public S57_beacon
{
public:
	S57_buoy();
	~S57_buoy();
	DERIVE_OBJECT(S57_buoy);
	void DrawLowRes(float degradation,Window3D * win);
	virtual void SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber);
    virtual void ShowS57ShapeInfo(Scene3D *scene);

private:
	enum {SHP_DEFAULT, SHP_CONICAL, SHP_CAN, SHP_SPHERICAL, SHP_PILLAR, SHP_SPAR, SHP_BARREL, SHP_SUPER, SHP_ICE};
	static Glyph *buoyGlyph[];
};


#endif
