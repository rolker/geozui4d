#ifndef S57_SOUNDG_H
#define S57_SOUNDG_H

#include"S57Layer.h"
#include"S57_object.h"

class S57_sounding : public S57_object
{
public:
	S57_sounding();
	~S57_sounding();
	DERIVE_OBJECT(S57_sounding);
	void DrawLowRes(float degradation,Window3D * win);
	virtual void SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber);
private:
	std::vector<std::string> sounding;
        std::vector<std::string> decimal;
	std::vector<int> xOffset;
};


#endif
