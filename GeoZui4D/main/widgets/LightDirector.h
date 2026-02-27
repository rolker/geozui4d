/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// LightDirector.h
///////////////////////////////////////////////////////////////////////////////
/**
 *	This class implements a light direction interactor that displays light
 *	direction and allows the user to modify this direction interactively.
 *	This class implements the following responsibilities:
 *	*	Implement a light direction interactor as described above
 *
 *	Created by mdp 7/16/2001
 */
///////////////////////////////////////////////////////////////////////////////

class LightDirector; //this line connects the above comments with this class.

#ifndef LIGHTDIRECTOR_H
#define LIGHTDIRECTOR_H

#include "engine/Interactor.h"
#include "interactors/Pickable.h"

class SDV_Light;

class LightDirector : public Interactor, public Pickable
{
	public:
		enum {Invisible, Visible, Dragging, DraggingVisible};
		LightDirector(Window3D *myParent, Point3d loc);
		~LightDirector();

		void DrawIn(Window3D *inWindow);
		void setVisible(GLboolean vis = GL_TRUE);

		int HandleInput(unsigned int inputType, InputInfo *info);
		virtual void RemovalNotify(InputMediator *fromList);

                virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);
	    virtual void renderPickableObject(Window3D *inWindow);
		virtual bool pickAsObject() {return false;}

	private:
		SDV_Light *light;
		Window3D *parent;
		Point3d location;
		float targetRadius, sourceRadius, distanceBetween;
		int state;
		GLdouble projMatrix[16];
};

#endif
