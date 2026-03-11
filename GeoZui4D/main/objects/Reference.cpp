// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
#include "GZincludes.h"
#include "Reference.h"

#include "engine/ObjectPoseState.h"

extern MainWindow * mainWindow;

REGISTER_OBJECT(Reference, "");

Reference::Reference()
{
    reference = NULL;
    ObjectPoseState * op = new ObjectPoseState(this);
    pose = op;
}

void Reference::DrawLowRes(float degradation,Window3D *win)
{
/*	Point3d pos;
	const OrientationInfo * orient;

	GetPosition(pos);
	orient = pose->getOrientation();
*/
	if(reference)
	{
/*		reference->SetPosition(pos);
		if(orient)
			reference->getPosable()->setOrientation(*orient);*/
		reference->DrawLowRes(degradation,win);
	}
}

Object * Reference::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object::LoadXML(xml,path,ncfile);

    std::string ref;
    ref = xml.get("<xmlattr>.reference",ref);
    if(!ref.empty())
        reference = mainWindow->getScene3D()->GetObjectPtr(ref);
    return this;
}

int Reference::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    if (argc >= 1)
    {
        std::string message = argv[0];
        if (message == "reference")
        {
            if(argc == 2)
            {
                reference = mainWindow->getScene3D()->GetObjectPtr(argv[1]);
                return TCL_OK;
            }
            else
            {
                Tcl_AppendResult(interp, "usage: reference object_label", NULL);
                return TCL_ERROR;
            }
        }
    }
    return Object::perform(argc, argv, interp);
}
