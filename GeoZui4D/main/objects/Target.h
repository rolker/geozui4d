#ifndef TARGET_H
#define TARGET_H

#include"engine/Object.h"

class Target: public Object
{
public:
	Target();
	~Target();
	DERIVE_OBJECT(Target);
	void DrawLowRes(float degradation, Window3D *win);
	void SetColor(float r, float g, float b);
	void SetSize(float s);
private:
	float color[3];
	float size;
	
};


#endif
