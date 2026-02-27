#include "X3D.h"
#include <gz4d/X3D.h>
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/x3d/X3DGeoLocation.h>

REGISTER_OBJECT(X3D, "x3d");


#include "engine/ObjectPoseState.h"
#include "MainWindow.h"

extern MainWindow *mainWindow;


X3D::X3D()
    :pointSize(1.0),x3d(mainWindow->getGloContext())
{
    ObjectPoseState * op = new ObjectPoseState(this);
    pose = op;
    op->ignoreHeightExaggeration();
}

X3D::~X3D()
{
}

void X3D::DrawLowRes(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_POINT_BIT);
    glPointSize(pointSize);
    glPushMatrix();
    glRotatef(90.0,1.0,0.0,0.0);
    x3d.Draw();
    glPopMatrix();
}


int X3D::Load()
{
    SetLabel(GetFilenameAsPath().filename().string());
    if (x3d.Load(GetFilename()))
    {
        gz4d::SFNode geoNode = x3d.GetGeoNode();
        if (geoNode)
        {
            std::shared_ptr<gz4d::X3DGeoLocation> geoLoc = std::dynamic_pointer_cast<gz4d::X3DGeoLocation>(geoNode);
            if(geoLoc)
            {
                bool isUTM = false;
                int zone = 0;
                for (gz4d::MFString::const_iterator s = geoLoc->getGeoSystem().begin(); s != geoLoc->getGeoSystem().end(); ++s)
                {
                    if(*s == "UTM")
                        isUTM = true;
                    else if ((*s)[0] == 'Z')
                    {
                        std::string zoneStr = s->substr(1);
                        zone = boost::lexical_cast<int>(zoneStr);
                    }
                }
                if (isUTM)
                {
                    setUTMProjection(zone,true); //TODO get correct north/south flag
                    SetPosition(geoLoc->getGeoCoords());
                }
            }
        }
        return 0;
    }
    return 1;
}

int X3D::perform(int argc, const char** argv, Tcl_Interp* interp)
{
    if (argc >= 1)
    {
        std::string command(argv[0]);
        if (command ==  "pointSize")
        {
            if (argc == 2)
                pointSize  = boost::lexical_cast<float>(argv[1]);
            Tcl_SetResult(interp, (char *)(boost::lexical_cast<std::string>(pointSize).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
    }
    return Object::perform(argc, argv, interp);
}

void X3D::getHtmlInfo(std::stringstream &html, short level)
{
    html << "<h" << level << ">X3D</h" << level << ">";

    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/pointSize/value\">Point Size:<input type=\"text\" name=\"value\" value=\"" << pointSize <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    Object::getHtmlInfo(html,level);
}

