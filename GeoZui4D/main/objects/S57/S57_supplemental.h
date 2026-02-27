#ifndef S57_SUPPLEMENTAL_H
#define S57_SUPPLEMENTAL_H

#include"S57Layer.h"
#include"S57_object.h"

class S57_supplemental : public S57_object
{
public:
	S57_supplemental();
	~S57_supplemental();
	DERIVE_OBJECT(S57_supplemental);
	virtual void DrawLowRes(float degradation, Window3D * win);
	virtual void InitializeFor(const char *acronym);
	virtual void SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber);
	virtual void ShowInfo(GLuint *names = NULL, int numNames = 0);
	virtual void ShowS57Info(Scene3D *scene);

    void setParentS57(S57_object *newParent);

    //Scripting
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);

private:
    enum {NONE, DAYMAR, FOGSIG, LIGHTS, RADRFL, RETRFL, RTPBCN, TOPMAR};
    int type;
    int shape;
    S57_object *parentS57;

    int sigGen;
    std::string sigGroup, sigSequence, radarWavelength;
    float sigPeriod;

    float range;

    int exhibition;
    int lightChar;
    float sector1, sector2;
    std::vector<int> lightVisibilityList;
};

#endif
