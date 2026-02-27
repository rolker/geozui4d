#ifndef MPHANDLE_H
#define MPHANDLE_H

#include "MPPoint.h"

class MPHandle: public MPPoint
{
public:
    MPHandle();
    MPHandle(int id, gz4d::xml::Node cmd);
    DERIVE_OBJECT(MPHandle);
    void PoseChanged(PosableStateBase *whichPose, long whatChanged);

    virtual void command(gz4d::xml::Node c, std::string const &cmd);
private:

};

#endif
