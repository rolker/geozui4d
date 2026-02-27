#ifndef S57_BUISGL_H
#define S57_BUISGL_H

#include"S57Layer.h"
#include"S57_object.h"

class S57_building : public S57_object
{
public:
	S57_building();
	~S57_building();
	DERIVE_OBJECT(S57_building);
	void DrawLowRes(float degradation,Window3D * win);
	void SetHeight(double h){height = h;};
	virtual void SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber);
	virtual void ShowS57Info(Scene3D *scene);

private:
	int function, printOffset;
};


#endif
