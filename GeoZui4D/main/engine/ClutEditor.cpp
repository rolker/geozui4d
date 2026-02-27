/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ClutEditor.cpp
// 
//
//	This class implements a Color Look-Up Table editor as a ToglWindow.
//	This class implements the following responsibilities:
//		Initialize Tcl/Tk clut commands
//		Allow interactive modification of cluts
//
//	Derived from ToglWindow and commented by mdp 7/14/2000
///////////////////////////////////////////////////////////////////////////////

#include<gz4d/glo.h>
//#include "PBuffer.h"
#include <GL/glut.h>
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "engine/ClutEditor.h"
//#include <iostream.h>
#include <stdlib.h>
#include <filesystem>
#include <string.h>

int ClutEditor::clutToglInitialized = 0;
void *ClutEditor::displayFont = GLUT_BITMAP_HELVETICA_12;

#define COLOR_STRETCH 1.2f
#define COLOR_OFFSET 0.1f
#define DEFAULT_STEP 10.0f

/*-----------------------Constructors/Destructors----------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	Constructor.  Initializes the ClutEditor with the passed Clut, or a new
 *  one if the passed Clut is NULL.
 *
 */
 
ClutEditor::ClutEditor(std::string name, Tcl_Interp *interp, std::shared_ptr<Clut> &c, GLboolean asColorKey)
:ToglWindow(name)
{
	clutnode *neighbor;

	if (!c)
	{
		clut = std::shared_ptr<Clut>(new Clut);
		clut->Init();
	}
	else
	{
		clut = c;
	}
	
	min = clut->GetMinheight();
	max = clut->GetMaxheight();
	range = clut->GetHeightRange();
	zoommin = (float)(min+(range*0.4));
	zoommax = (float)(max-(range*0.4));
	
	keyStart = 0.0;
	keyStop = 0.0;
	keyStep = 10.0;
	currentKey = 0;
	keyNumSteps = 4;
	currentKind = 0;
	aligned = GL_TRUE;
	bgStrength = 0.0;
	fgStrength = 1.0;
	unitLabels = NoUnitLabels;
    widthBuffer = 60;

	ract = gact = bact = 0;
	rcb = gcb = bcb = 0;
	hsact = vact = 0;

	element = clut->GetNodePtr(0);
	elementheight = (float)element->height;
	neighbor = element->previous;
	if (neighbor)
		elementLength = elementheight - (float)neighbor->height;
	else
		elementLength = -1.0; //"infinite";
	GetHSV(element->color, hsv);
	
	scene3d = NULL;
	colorKeyMode = asColorKey;

	if (asColorKey)
		MainWindow::Tcl_Eval("colorkey .key");
	else
		MainWindow::Tcl_Eval("cluteditor .cluted");
}

/*---------------------------------------------------------------------------*/
/**
 *	Allows for child-specific initialization, sets up the Tcl interpreter if
 *	it is not already set up.  All child classes must call the parent's Init
 *	function at the top of their implementation of this procedure.
 */
void ClutEditor::Init(int argc, char *argv[])
{
  char const *newCommand[] = {"inittexture", "loadtexture", "savetexture", 
			"deletetexture",
			"defaulttexture", "usedeftexture", "usecustomtexture",
			"displaytexture", "editclut", "addclutelement",
			"deleteclutelement", "setclutelement", "modifyclutextents",
            "createclutrange", "clearclut", "keydisplay", "clutelement",
			NULL};
	int currentCommand;

	ToglWindow::Init(argc, argv);

	if (!clutToglInitialized)
	{
		for (currentCommand = 0; newCommand[currentCommand]; currentCommand++)
			Togl_CreateCommand(newCommand[currentCommand], SubCommandCallBack);
		clutToglInitialized = 1;
	}

};

/*---------------------------------------------------------------------------*/
/**
 *	Set the current Scene3D, on which subsequent operations will occur.
 */
void ClutEditor::setScene3D(Scene3D *s3d)
{
	scene3d = s3d;
}

/*---------------------------------------------------------------------------*/
/**
 *	What does this do exactly?
 */
void ClutEditor::SetClut(Tcl_Interp *interp, std::shared_ptr<Clut> &c)
{
	clutnode *neighbor;

	clut = c;
	element = clut->GetNodePtr(0);
	elementheight = (float)element->height;
	neighbor = element->previous;
	if (neighbor)
		elementLength = elementheight - (float)neighbor->height;
	else
		elementLength = -1.0; //"infinite";

	char tclstr[200];
	keyStart = clut->GetMinheight();
	keyStop = clut->GetMaxheight();
	keyStep = DEFAULT_STEP;
	while (keyStep > (keyStop - keyStart))
		keyStep /= 10.0f;
	currentKind = 0;
	sprintf(tclstr,".cluted.elementheight configure -text \"Height: %f\"",elementheight);
	Tcl_Eval(tclstr);
}

/*---------------------------------------------------------------------------*/
/**
 *	What does this do exactly?
 */
void ClutEditor::SetTargetTogl()
{
	char tclstr[200];
	sprintf(tclstr,"global mainWindow; $mainWindow makecurrent");
	Tcl_Eval(tclstr);

}

/*---------------------------------------------------------------------------*/
/**
 *	What does this do exactly?
 */
void ClutEditor::SetClutEdTogl()
{
	char tclstr[200];
	if (colorKeyMode)
		sprintf(tclstr,".key.innards.togl makecurrent");
	else
		sprintf(tclstr,".cluted.gl makecurrent");
	Tcl_Eval(tclstr);

}

/*---------------------------------------------------------------------------*/
/**
 *	What does this do exactly?
 */
void ClutEditor::SetTargetTogl(Tcl_Interp *interp)
{
	char tclstr[200];
	sprintf(tclstr,"global mainWindow; $mainWindow makecurrent");
	Tcl_Eval(tclstr);

}

/*---------------------------------------------------------------------------*/
/**
 *	What does this do exactly?
 */
void ClutEditor::SetClutEdTogl(Tcl_Interp *interp)
{
	char tclstr[200];
	if (colorKeyMode)
		sprintf(tclstr,".key.innards.togl makecurrent");
	else
		sprintf(tclstr,".cluted.gl makecurrent");
	Tcl_Eval(tclstr);

}


/*---------------------------------------------------------------------------*/
/**
 *	What does this do exactly?
 */
void ClutEditor::SetTargetToglPtr(Togl *togl)
{
	targetTogl = togl;
}

/*---------------------------------------------------------------------------*/
/**
 *	Destructor.  Does nothing right now.
 */
ClutEditor::~ClutEditor()
{
}

/*---------------------------Command Callbacks-------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	This function is the branch-point for Togl subcommands defined by
 *	ToglWindow subclasses.  All subclasses must call the parent class's
 *	SubCommand procedure after they have exhausted the subcommand
 *	possibilities in their implementation of this procedure.
 *
 *	argv[0]		Togl object name
 *	argv[1]		Togl subcommand name (known to be registered)
 *	argv[2...]	Arguments
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::SubCommand(int argc, char *argv[])
{
	if (!strcmp(argv[1], "inittexture"))
		return InitTexture(argc, argv);
	else if (!strcmp(argv[1], "loadtexture"))
		return LoadTexture(argc, argv);
	else if (!strcmp(argv[1], "savetexture"))
		return SaveTexture(argc, argv);
	else if (!strcmp(argv[1], "deletetexture"))
		return DeleteTexture(argc, argv);
	else if (!strcmp(argv[1], "defaulttexture"))
		return DefaultTexture(argc, argv);
	else if (!strcmp(argv[1], "usedeftexture"))
		return UseDefaultTexture(argc, argv);
	else if (!strcmp(argv[1], "usecustomtexture"))
		return UseCustomTexture(argc, argv);
	else if (!strcmp(argv[1], "displaytexture"))
		return DisplayTexture(argc, argv);
	else if (!strcmp(argv[1], "editclut"))
		return EditClut(argc, argv);
	else if (!strcmp(argv[1], "addclutelement"))
		return AddClutElement(argc, argv);
	else if (!strcmp(argv[1], "setclutelement"))
		return SetClutElement(argc, argv);
	else if (!strcmp(argv[1], "deleteclutelement"))
		return DeleteClutElement(argc, argv);
	else if (!strcmp(argv[1], "modifyclutextents"))
		return ModifyClutExtents(argc, argv);
	else if (!strcmp(argv[1], "createclutrange"))
		return CreateClutRange(argc, argv);
	else if (!strcmp(argv[1], "clearclut"))
		return ClearClut(argc, argv);
	else if (!strcmp(argv[1], "keydisplay"))
		return KeyDisplay(argc, argv);
	else if (!strcmp(argv[1], "clutelement"))
		return ClutElement(argc, argv);

	return ToglWindow::SubCommand(argc, argv);
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes a new clut to be added to the environment.
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::InitTexture(int argc, char *argv[])
{
	scene3d->InitTexture(Togl_Interp(myTogl));
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes a clut to be loaded and added to the environment.
 *
 *	argv[2]		Filename of clut file to be loaded
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::LoadTexture(int argc, char *argv[])
{
    if (argv[2] && argv[2][0] != '\0')
    {
        std::shared_ptr<TextureMap> t = scene3d->LoadTexture(argv[2], Togl_Interp(myTogl));
        scene3d->AddTexture(t);
    }
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes a texture in the environment to be saved.
 *
 *	argv[2]		Filename of clut file to be saved
 *	argv[3]		Texture ID #
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::SaveTexture(int argc, char *argv[])
{
	scene3d->SaveTexture(argv[2], argv[3]);
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes a texture in the environment to be deleted.
 *
 *	argv[2]			Texture ID of texture to be deleted
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::DeleteTexture(int argc, char *argv[])
{
	scene3d->DeleteTexture(atoi(argv[2]));
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes the defaultTexture to be the one at the passed index.
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::UseDefaultTexture(int argc, char *argv[])
{
	scene3d->UseDefaultTexture(atoi(argv[2]));
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes the defaultTexture to become the current Texture?
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::DefaultTexture(int argc, char *argv[])
{
	scene3d->SetGlobalTexture(atoi(argv[2]));
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes a customTexture to become the current Texture?
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::UseCustomTexture(int argc, char *argv[])
{
	scene3d->UseCustomTexture(atoi(argv[2]), atoi(argv[3]));
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes a new region of a texture to be displayed??
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::DisplayTexture(int argc, char *argv[])
{
  scene3d->DisplayTexture(Togl_Interp(myTogl), atoi(argv[2]),
			  (float)atof(argv[3]), (float)atof(argv[4]), argv[5]);
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes a clut to be the current one in place for editing.
 *
 *	argv[2]		ID of Clut to be edited
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::EditClut(int argc, char *argv[])
{
    std::shared_ptr<TextureMap> texture = scene3d->GetTexture(atoi(argv[2]));

    std::shared_ptr<Clut> c = std::dynamic_pointer_cast<Clut>(texture);
    if(!c)
        return TCL_ERROR;
    else
    {
          SetClut(Togl_Interp(myTogl), c);
          return TCL_OK;
    }
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes a new range element to be added to the current clut,
 *	with a ?-bound of argv[2]
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::AddClutElement(int argc, char *argv[])
{
	AddElement(Togl_Interp(myTogl), (float)atof(argv[2]));
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function modifies the ?-bound of the active range element in
 *	the current clut?
 *
 *	argv[2]		height of the ?-bound
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::SetClutElement(int argc, char *argv[])
{
	Update(Togl_Interp(myTogl), (float)atof(argv[2]));
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function deletes the currently active range element in the
 *	current clut.
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::DeleteClutElement(int argc, char *argv[])
{
	DeleteElement(Togl_Interp(myTogl));
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function stretches and shifts the cluts to the new min and max
 *	extents specified
 *
 *	argv[2]		new max height
 *	argv[3]		new min height
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::ModifyClutExtents(int argc, char *argv[])
{
	float newmin, newmax, newrange;
	float shift, scale;

	newmax = atof(argv[2]);
	newmin = atof(argv[3]);
	newrange = newmax - newmin;

	scale = newrange/range;
	shift = newmin-(min*scale);

	clut->ModifyExtents(scale,shift);

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function stretches and shifts the cluts to the new min and max
 *	extents specified
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ClutEditor::CreateClutRange(int argc, char *argv[])
{
	SetTargetTogl(); //added by mdp 7/2/2001
    clut->CreateRange(atof(argv[2]),atof(argv[3]),atof(argv[4]));
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
int ClutEditor::ClearClut(int argc, char *argv[])
{
    switch(argc)
    {
    case 2:
        clut->Clear();
        return TCL_OK;
    case 4:
        clut->ClearRange(atof(argv[2]),atof(argv[3]));
        return TCL_OK;
    default:
        break;
    }
    return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int ClutEditor::KeyDisplay(int argc, char *argv[])
{
	GLboolean badSyntax = GL_FALSE, needsRedisplay = GL_FALSE;
	char retval[16] = "";

	if (argc < 3)
		badSyntax = GL_TRUE;
	else
	{
		if (!strcmp(argv[2], "min"))
		{
			if (argc >= 4)
			{
				keyStart = (float)atof(argv[3]);
				keyNumSteps = -1;
				if (keyStop <= keyStart)
					keyStop = keyStart + keyStep;
				needsRedisplay = GL_TRUE;
			}
			sprintf(retval, "%f", keyStart);
		}
		else if (!strcmp(argv[2], "max"))
		{
			if (argc >= 4)
			{
				keyStop = (float)atof(argv[3]);
				keyNumSteps = -1;
				if (keyStop <= keyStart)
					keyStart = keyStop - keyStep;
				needsRedisplay = GL_TRUE;
			}
			sprintf(retval, "%f", keyStop);
		}
		else if (!strcmp(argv[2], "step"))
		{
			if (argc >= 4)
			{
				keyStep = (float)atof(argv[3]);
				keyNumSteps = -1;
				needsRedisplay = GL_TRUE;
			}
			sprintf(retval, "%f", keyStep);
		}
		else if (!strcmp(argv[2], "midtics"))
		{
			if (argc >= 4)
			{
				keyNumSteps = atoi(argv[3]);
				needsRedisplay = GL_TRUE;
			}
			sprintf(retval, "%d", (int)keyNumSteps);
		}
		else if (!strcmp(argv[2], "align"))
		{
			if (argc >= 4)
			{
				aligned = (atoi(argv[3])? GL_TRUE : GL_FALSE);
				needsRedisplay = GL_TRUE;
			}
			sprintf(retval, "%d", aligned);
		}
		else if (!strcmp(argv[2], "units"))
		{
			if (argc >= 4)
			{
				if (argv[3][0] == 'm')
					unitLabels = MeterLabels;
				else if (argv[3][0] == 'f')
                    unitLabels = FeetLabels;
                else
					unitLabels = atoi(argv[3]);
				needsRedisplay = GL_TRUE;
			}
			sprintf(retval, "%d", unitLabels);
		}
		else if (!strcmp(argv[2], "bg"))
		{
			if (argc >= 4)
			{
				bgStrength = (float)atof(argv[3]);
				needsRedisplay = GL_TRUE;
			}
			sprintf(retval, "%f", bgStrength);
		}
		else if (!strcmp(argv[2], "fg"))
		{
			if (argc >= 4)
			{
				fgStrength = (float)atof(argv[3]);
				needsRedisplay = GL_TRUE;
			}
			sprintf(retval, "%f", fgStrength);
		}
	    else if (!strcmp(argv[2], "buffer"))
	    {
		    if (argc >= 4)
		    {
			    widthBuffer = (float)atof(argv[3]);
			    needsRedisplay = GL_TRUE;
		    }
		    sprintf(retval, "%f", widthBuffer);
	    }
		else
			badSyntax = GL_TRUE;
	}
	if (badSyntax)
	{
		Tcl_AppendResult(interp, "usage:\n",
						 "<window> keydisplay {step | min | max | midtics | align | bg | fg | units | buffer} [<value>]\n", NULL);
		return TCL_ERROR;
	}
	Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
	if (needsRedisplay)
		Togl_PostRedisplay(myTogl);
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
int ClutEditor::ClutElement(int argc, char *argv[])
{
	GLboolean badSyntax = GL_FALSE, needsRedisplay = GL_FALSE;
	char retval[16] = "";
    clutnode *node, *oldElement = element;
    float val;
    int arg, idx;

	if (argc < 4)
		badSyntax = GL_TRUE;
	else
	{
        //Grab the node...
        if (!strcmp(argv[2], "end"))
            node = clut->GetNodePtrAtIndex();
        else if (argv[2][0] == '@')
            node = clut->GetNodePtr((float)atof(&argv[2][1]));
        else
            node = clut->GetNodePtrAtIndex(atoi(argv[2]));

        if (node)
        {
            //now operate on it
		    if (!strcmp(argv[3], "getHeight"))
		    {
			    sprintf(retval, "%f", node->height);
		    }
            else if (!strcmp(argv[3], "setHeight") && argc >= 5)
            {
                val = (float)atof(argv[4]);
                element = node;
	            Update(Togl_Interp(myTogl), val);
                sprintf(retval, "%f", val);
                element = oldElement;
                needsRedisplay = true;
            }
		    else if (!strcmp(argv[3], "getColor"))
		    {
			    sprintf(retval, "%i %i %i %i", node->color[0], node->color[1], node->color[2], node->color[3]);
		    }
		    else if (!strcmp(argv[3], "setColor") && argc >= 5)
		    {
                arg = 4;
                idx = 0;
                while (arg < argc && idx <= 4)
                    node->color[idx++] = (float)atof(argv[arg++]);
			    sprintf(retval, "%d %d %d %d", node->color[0], node->color[1], node->color[2], node->color[3]);
                needsRedisplay = true;
		    }
		    else if (!strcmp(argv[3], "delete"))
		    {
                element = node;
            	DeleteElement(Togl_Interp(myTogl));
                if (oldElement != node)
                    element = oldElement;
                needsRedisplay = true;
            }
            else if (!strcmp(argv[3], "index"))
            {
                sprintf(retval, "%d", clut->GetIndexOf(node));
            }
            else
                badSyntax = true;
        }
        else
        {
            Tcl_AppendResult(interp, "Could not find element ", argv[2], "\n", NULL);
            return TCL_ERROR;
        }
    }

	if (badSyntax)
	{
		Tcl_AppendResult(interp, "usage:\n",
            "<window> clutElement {<index> | @<height> | end} {getHeight | setHeight | getColor | setColor | delete | index} [<value>]...\n", NULL);
		return TCL_ERROR;
	}
	Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
	if (needsRedisplay)
		Togl_PostRedisplay(myTogl);
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	Displays the ClutEditor in the Togl window.
 */
void ClutEditor::Display()
{
	float render_hsv[3];
	int j, precision;
	float h;
	float colorBarWidth, antistretch, antioffset, check, numSteps, currentHeight, labeledHeight;

	glDrawBuffer(GL_BACK);
	glClearColor(bgStrength,bgStrength,bgStrength,0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	char tclstr[200], min_string[20], max_string[20], range_string[16];
	sprintf(tclstr,".cluted.elementheight configure -text \"Height: %f\"",elementheight);
	Tcl_Eval(tclstr);
	clutnode *currentNode;
	// draw main color table

	min = clut->GetMinheight();
	max = clut->GetMaxheight();
	range = clut->GetHeightRange();

	sprintf(tclstr,".cluted.clutmaxcurrent configure -text \"Max: %f\"",max);
	Tcl_Eval(tclstr);
	sprintf(tclstr,".cluted.clutmincurrent configure -text \"Min: %f\"",min);
	Tcl_Eval(tclstr);

	if (colorKeyMode)
	{
		colorBarWidth = width - widthBuffer;
		if (keyNumSteps < 0)
		{
			min = keyStart;
			max = keyStop;
			range = max - min;
		}
	}
	else
		colorBarWidth = 0.1f * width;

   	for(j=0;j<height; j++)
	{
		h = (float)((((float)j/(float)height)*range*COLOR_STRETCH)+(min-(range*COLOR_OFFSET)));
		glColor4ub(clut->GetComponent(h,0),clut->GetComponent(h,1),clut->GetComponent(h,2),clut->GetComponent(h,3));
		glBegin(GL_POLYGON);
		glVertex2i(0,j);
		glVertex2i((int)(colorBarWidth),j);
		glVertex2i((int)(colorBarWidth),j+1);
		glVertex2i(0,j+1);
		glEnd();
	}

	precision = 0;
	if (range > 0)
	{
		for (check = 100.0; range < check; check /= 10.0f)
			precision++;
		sprintf(range_string, "%%.%df", precision);
	}
	else
		strcpy(range_string, "%f");
	if (unitLabels == MeterLabels)
		strcat(range_string, " m");
    else if (unitLabels == FeetLabels)
    {
        if (precision >= 2)
            precision = 1;
        sprintf(range_string, "%%.%df ft", precision);
    }

	//if we're a color key right now, print out some color values.
	if (colorKeyMode)
	{
		antistretch = height / (range * COLOR_STRETCH);
		antioffset = range * COLOR_OFFSET - min;
		glColor3f(fgStrength, fgStrength, fgStrength);
		if (keyNumSteps >= 0)
		{
			numSteps = keyNumSteps + 1;
			for (j = 0; j <= numSteps; j++)
			{
				currentHeight = (j * range) / numSteps + min;
				if (aligned)
				{
					currentNode = clut->GetNodePtr(currentHeight);
					labeledHeight = currentNode->height;
				}
				else
					labeledHeight = currentHeight;
				//different meaning for h here.
				h = (labeledHeight + antioffset) * antistretch;
                if (unitLabels == FeetLabels)
                    labeledHeight *= 3.2808399;

				glBegin(GL_LINES);
				glVertex2i(0, (int)h);
				glVertex2i((int)(colorBarWidth + 4), (int)h);
				glEnd();
				sprintf(max_string, range_string, (float)fabs(labeledHeight));
				printAt(colorBarWidth + 7, h - 4, 0, max_string, GL_FALSE, displayFont);
			}
		}
		else
		{
			for (currentHeight = keyStart; currentHeight <= keyStop; currentHeight += keyStep)
			{
				if (aligned)
				{
					currentNode = clut->GetNodePtr(currentHeight);
					labeledHeight = currentNode->height;
				}
				else
					labeledHeight = currentHeight;
				h = (labeledHeight + antioffset) * antistretch;
                if (unitLabels == FeetLabels)
                    labeledHeight *= 3.2808399;
				glBegin(GL_LINES);
				glVertex2i(0, (int)h);
				glVertex2i((int)(colorBarWidth + 4), (int)h);
				glEnd();
				sprintf(max_string, range_string, (float)fabs(labeledHeight));
				printAt(colorBarWidth + 7, h - 4, 0, max_string, GL_FALSE, displayFont);
			}
		}
	}
	else
	{

		// line from main color table to zoomed table
		glColor4ub(200,200,200,255);
		glBegin(GL_LINES);
		
		j = (int)(((zoommin - min+(range*COLOR_OFFSET))/(range*COLOR_STRETCH))*height);
		glVertex2i((int)(width*0.1),j);
		glVertex2i((int)(width*0.2),0);
		
		j = (int)(((zoommax - min+(range*COLOR_OFFSET))/(range*COLOR_STRETCH))*height);
		glVertex2i((int)(width*0.1),j);
		glVertex2i((int)(width*0.2),height);
		
		glEnd();
		
		// draw zoomed color table
		
		for(j=0;j<height; j++)
		{
			h = (((float)j/(float)height)*(zoommax-zoommin))+zoommin;
			glColor4ub(clut->GetComponent(h,0),clut->GetComponent(h,1),clut->GetComponent(h,2),255);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.2,j);
			glVertex2f(width*0.3,j);
			glVertex2f(width*0.3,j+1);
			glVertex2f(width*0.2,j+1);
			glEnd();
		}

		//Draw labels...
		glColor4ub(255, 255, 255, 255);
		sprintf(min_string, range_string, zoommin);
		sprintf(max_string, range_string, zoommax);
		printAt(width * 0.3f, 0, 0, min_string, GL_FALSE, displayFont);
		printAt(width * 0.3f, height - 10, 0, max_string, GL_FALSE, displayFont);

		// draw selected color sequence entry
		if (element != NULL)
		{
			// actual color patch
			glColor4ubv(element->color);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.4,height*0.1);
			glVertex2f(width*0.6,height*0.1);
			glVertex2f(width*0.6,height*0.3);
			glVertex2f(width*0.4,height*0.3);
			glEnd();
			
			// label for color patch
			glColor4ub(255, 255, 255, 255);
			if (!element->next)
				sprintf(max_string, "(INF, ");
			else
			{
				sprintf(range_string, "[%%.%df, ", precision);
				sprintf(max_string, range_string, elementheight);
			}
			if (elementLength >= 0.0f)
			{
				sprintf(range_string, "%%.%df]", precision);
				sprintf(min_string, range_string, elementheight - elementLength); 
				strcat(max_string, min_string);
			}
			else
				strcat(max_string, "-INF)");
			printAt(width * 0.4f, height *0.07, 0, max_string, GL_FALSE, displayFont);

			// line to color patch
			glColor3ub(255,255,255);
			glBegin(GL_LINES);
			glVertex2f(width*0.3,((element->height-zoommin)*(height))/(zoommax-zoommin));
			glVertex2f(width*0.4,height*0.3);
			glEnd();


			// frame around color patch
			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.4,height*0.1);
			glVertex2f(width*0.6,height*0.1);
			glVertex2f(width*0.6,height*0.3);
			glVertex2f(width*0.4,height*0.3);
			glEnd();
			
			// red color patch
			glColor4ub(element->color[0],0,0,0);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.4,height*0.35);
			glVertex2f(width*0.45,height*0.35);
			glVertex2f(width*0.45,height*0.4);
			glVertex2f(width*0.4,height*0.4);
			glEnd();
			
			// frame around red color patch
			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.4,height*0.35);
			glVertex2f(width*0.45,height*0.35);
			glVertex2f(width*0.45,height*0.4);
			glVertex2f(width*0.4,height*0.4);
			glEnd();
			
			// green color patch
			glColor4ub(0,element->color[1],0,0);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.475,height*0.35);
			glVertex2f(width*0.525,height*0.35);
			glVertex2f(width*0.525,height*0.4);
			glVertex2f(width*0.475,height*0.4);
			glEnd();
			
			// frame around green color patch
			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.475,height*0.35);
			glVertex2f(width*0.525,height*0.35);
			glVertex2f(width*0.525,height*0.4);
			glVertex2f(width*0.475,height*0.4);
			glEnd();
			
			// blue color patch
			glColor4ub(0,0,element->color[2],0);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.55,height*0.35);
			glVertex2f(width*0.6,height*0.35);
			glVertex2f(width*0.6,height*0.4);
			glVertex2f(width*0.55,height*0.4);
			glEnd();
			
			// frame around blue color patch
			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.55,height*0.35);
			glVertex2f(width*0.6,height*0.35);
			glVertex2f(width*0.6,height*0.4);
			glVertex2f(width*0.55,height*0.4);
			glEnd();
			
			// red color selector
			glColor3ub(255,0,0);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.4,height*0.95);
			glVertex2f(width*0.45,height*0.95);
			glColor3ub(0,0,0);
			glVertex2f(width*0.45,height*0.45);
			glVertex2f(width*0.4,height*0.45);
			glEnd();
			
			// frame around red color selector
			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.4,height*0.95);
			glVertex2f(width*0.45,height*0.95);
			glVertex2f(width*0.45,height*0.45);
			glVertex2f(width*0.4,height*0.45);
			glEnd();
			
			// green color selector
			glColor3ub(0,255,0);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.475,height*0.95);
			glVertex2f(width*0.525,height*0.95);
			glColor3ub(0,0,0);
			glVertex2f(width*0.525,height*0.45);
			glVertex2f(width*0.475,height*0.45);
			glEnd();
			
			// frame around green color selector
			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.475,height*0.95);
			glVertex2f(width*0.525,height*0.95);
			glVertex2f(width*0.525,height*0.45);
			glVertex2f(width*0.475,height*0.45);
			glEnd();
			
			// blue color selector
			glColor3ub(0,0,255);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.55,height*0.95);
			glVertex2f(width*0.6,height*0.95);
			glColor3ub(0,0,0);
			glVertex2f(width*0.6,height*0.45);
			glVertex2f(width*0.55,height*0.45);
			glEnd();
			
			// frame around blue color selector
			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.55,height*0.95);
			glVertex2f(width*0.6,height*0.95);
			glVertex2f(width*0.6,height*0.45);
			glVertex2f(width*0.55,height*0.45);
			glEnd();
			
			// red color arrow
			glColor3ub(255,255,255);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.3875,height*(0.0125+((element->color[0]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.40,height*(0.0+((element->color[0]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.3875,height*(-0.0125+((element->color[0]/255.0)*(0.95-0.45))+0.45));
			glEnd();
			
			glBegin(GL_POLYGON);
			glVertex2f(width*0.4625,height*(0.0125+((element->color[0]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.45,height*(0.0+((element->color[0]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.4625,height*(-0.0125+((element->color[0]/255.0)*(0.95-0.45))+0.45));
			glEnd();
			
			// green color arrow
			glColor3ub(255,255,255);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.4625,height*(0.0125+((element->color[1]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.475,height*(0.0+((element->color[1]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.4625,height*(-0.0125+((element->color[1]/255.0)*(0.95-0.45))+0.45));
			glEnd();
			
			glBegin(GL_POLYGON);
			glVertex2f(width*0.5375,height*(0.0125+((element->color[1]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.525,height*(0.0+((element->color[1]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.5375,height*(-0.0125+((element->color[1]/255.0)*(0.95-0.45))+0.45));
			glEnd();
			
			// blue color arrow
			glColor3ub(255,255,255);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.5375,height*(0.0125+((element->color[2]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.55,height*(0.0+((element->color[2]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.5375,height*(-0.0125+((element->color[2]/255.0)*(0.95-0.45))+0.45));
			glEnd();
			
			glBegin(GL_POLYGON);
			glVertex2f(width*0.6125,height*(0.0125+((element->color[2]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.6,height*(0.0+((element->color[2]/255.0)*(0.95-0.45))+0.45));
			glVertex2f(width*0.6125,height*(-0.0125+((element->color[2]/255.0)*(0.95-0.45))+0.45));
			glEnd();
			
			// clipboard color patch
			glColor3ub(rcb,gcb,bcb);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.7,height*0.1);
			glVertex2f(width*0.9,height*0.1);
			glVertex2f(width*0.9,height*0.3);
			glVertex2f(width*0.7,height*0.3);
			glEnd();
			

			// frame around clipboard color patch
			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.7,height*0.1);
			glVertex2f(width*0.9,height*0.1);
			glVertex2f(width*0.9,height*0.3);
			glVertex2f(width*0.7,height*0.3);
			glEnd();


			// clipboard buttons

			// current to clipboard

			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.625,height*0.1);
			glVertex2f(width*0.675,height*0.1);
			glVertex2f(width*0.675,height*0.15);
			glVertex2f(width*0.625,height*0.15);
			glEnd();

			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.670,height*0.125);
			glVertex2f(width*0.655,height*0.145);
			glVertex2f(width*0.655,height*0.105);
			glEnd();

			// swap

			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.625,height*0.175);
			glVertex2f(width*0.675,height*0.175);
			glVertex2f(width*0.675,height*0.225);
			glVertex2f(width*0.625,height*0.225);
			glEnd();

			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.63,height*0.2);
			glVertex2f(width*0.645,height*0.22);
			glVertex2f(width*0.645,height*0.18);
			glEnd();

			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.67,height*0.2);
			glVertex2f(width*0.655,height*0.22);
			glVertex2f(width*0.655,height*0.18);
			glEnd();


			// clipboard to current

			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.625,height*0.25);
			glVertex2f(width*0.675,height*0.25);
			glVertex2f(width*0.675,height*0.30);
			glVertex2f(width*0.625,height*0.30);
			glEnd();

			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.63,height*0.275);
			glVertex2f(width*0.645,height*0.295);
			glVertex2f(width*0.645,height*0.255);
			glEnd();

			// HSV Area

			// V slider

			GetHSV(element->color, render_hsv);

			Point4ub c1,c2;
			render_hsv[2]=0;
			GetRGB(c1,render_hsv);
			render_hsv[2]=1.0;
			GetRGB(c2,render_hsv);

			glBegin(GL_POLYGON);
			glColor3ub(c1[0],c1[1],c1[2]);
			glVertex2f(width*0.9,height*0.45);
			glVertex2f(width*0.95,height*0.45);
			glColor3ub(c2[0],c2[1],c2[2]);
			glVertex2f(width*0.95,height*0.95);
			glVertex2f(width*0.9,height*0.95);
			glEnd();
			
			// frame around V slider
			glColor3ub(255,255,255);
			glBegin(GL_LINE_LOOP);
			glVertex2f(width*0.9,height*0.45);
			glVertex2f(width*0.95,height*0.45);
			glVertex2f(width*0.95,height*0.95);
			glVertex2f(width*0.9,height*0.95);
			glEnd();

			// V value arrow

			glColor3ub(255,255,255);
			glBegin(GL_POLYGON);
			glVertex2f(width*0.8875,height*(0.0125+(hsv[2]*(0.95-0.45))+0.45));
			glVertex2f(width*0.90,height*(0.0+(hsv[2]*(0.95-0.45))+0.45));
			glVertex2f(width*0.8875,height*(-0.0125+(hsv[2]*(0.95-0.45))+0.45));
			glEnd();
			
			glBegin(GL_POLYGON);
			glVertex2f(width*0.9625,height*(0.0125+(hsv[2]*(0.95-0.45))+0.45));
			glVertex2f(width*0.95,height*(0.0+(hsv[2]*(0.95-0.45))+0.45));
			glVertex2f(width*0.9625,height*(-0.0125+(hsv[2]*(0.95-0.45))+0.45));
			glEnd();

			// HS square

			GetHSV(element->color, render_hsv);
			glBegin(GL_QUADS);
			for(j = 0; j < height*0.5; j+=1)
			{
				render_hsv[0]=-1;
				render_hsv[1]=0;
				GetRGB(c1,render_hsv);
				render_hsv[0]=(j*360)/(height*0.5);
				render_hsv[1]=1;
				GetRGB(c2,render_hsv);
				glColor3ubv(c1);
				glVertex2f(width*0.65,(height*0.45)+j);
				glColor3ubv(c2);
				glVertex2f(width*0.85,(height*0.45)+j);
				render_hsv[0]=-1;
				render_hsv[1]=0;
				GetRGB(c1,render_hsv);
				render_hsv[0]=((j+1)*360)/(height*0.5);
				render_hsv[1]=1;
				GetRGB(c2,render_hsv);
				glColor3ubv(c2);
				glVertex2f(width*0.85,(height*0.45)+j+1);
				glColor3ubv(c1);
				glVertex2f(width*0.65,(height*0.45)+j+1);
			}
			glEnd();

			// HS Indicators
		
			glColor3f(1.0,1.0,1.0);
			glBegin(GL_LINES);
			glVertex2f(width*0.625,(height*0.45)+((hsv[0]*height*0.5)/360));
			glVertex2f(width*0.875,(height*0.45)+((hsv[0]*height*0.5)/360));
			glVertex2f((width*0.65)+(width*.2*hsv[1]),height*0.425);
			glVertex2f((width*0.65)+(width*.2*hsv[1]),height*0.975);
			glEnd();
		}
	}
	
	Togl_SwapBuffers(myTogl);
}

/*---------------------------------------------------------------------------*/
/**
 *	This function is called when the associated Togl widget has been resized.
 *	This function resets the Orthogonal projection and the modelview matricies.
 */
void ClutEditor::Reshape(int x, int y, int w, int h)
{
	ToglWindow::Reshape(x,y,w,h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*---------------------------------------------------------------------------*/
/**
 *	Processes events targeted for this ClutEditor.
 *
 *	@param	inputType	Type field of the info structure
 *	@param	info		Full event info structure
 *	@return				0=not handled / 1=handled / -1=handled,remove
 */
int ClutEditor::HandleInput(unsigned int inputType, InputInfo *info)
{
	if (inputType & EnumeratedAction)
		return PASS_EVENT_THROUGH;

    if (inputType & KeyStroke)
	{
		switch(info->val.i)
		{
//			case 'q':
//				delete this;
//				break;
			case KeyRightArrow: //Add more steps
				if (keyNumSteps >= 0)
				{
					keyNumSteps = -1;
					if (keyStart >= keyStop)
					{
						keyStart = clut->GetMinheight();
						keyStop = clut->GetMaxheight();
						keyStep = DEFAULT_STEP;
						while (keyStep > (keyStop - keyStart))
							keyStep /= 10.0f;
						currentKind = 0;
					}
				}
				else
				{
					if (currentKind & 1)
						keyStep /= 5.0;
					else
						keyStep /= 2.0;
					currentKind ^= 1;
				}
				break;
			case KeyLeftArrow: //remove steps
				if (keyNumSteps >= 0)
				{
					keyNumSteps = -1;
					if (keyStart >= keyStop)
					{
						keyStart = clut->GetMinheight();
						keyStop = clut->GetMaxheight();
						keyStep = DEFAULT_STEP;
						while (keyStep > (keyStop - keyStart))
							keyStep /= 10.0f;
						currentKind = 0;
					}
				}
				else
				{
					if (currentKind & 1)
						keyStep *= 2.0;
					else
						keyStep *= 5.0;
					currentKind ^= 1;
				}
				break;
			case KeyUpArrow: //move current range down (looks like it's moving up)
				if (currentKey)
					keyStop -= keyStep;
				else
					keyStart -= keyStep;
				break;
			case KeyDownArrow: //move current range up (looks like it's moving down)
				if (currentKey)
					keyStop += keyStep;
				else
					keyStart += keyStep;
				break;
			case '+': //work on moving upper portion of range
				currentKey = 1;
				break;
			case '-': //work on moving lower portion of range
				currentKey = 0;
				break;
			case '/': //toggle whether or not labels are aligned with color boundaries
				aligned ^= GL_TRUE;
				break;
			//how many markings are there between the top and bottom--evenly spaced
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				keyNumSteps = info->val.i - '0';
				break;
			case 'u':
                if (unitLabels)
                    unitLabels = NoUnitLabels;
                else
				    unitLabels = MeterLabels;
                break;
            case 'm':
                if (unitLabels == MeterLabels)
                    unitLabels = NoUnitLabels;
                else
				    unitLabels = MeterLabels;
                break;
            case 'f':
                if (unitLabels == FeetLabels)
                    unitLabels = NoUnitLabels;
                else
				    unitLabels = FeetLabels;
                break;
        }
		if (keyStop <= keyStart)
			keyStop = keyStart + keyStep;
		Togl_PostRedisplay(myTogl);
		return SINK_EVENT;
	}

	if (inputType & MouseButton1)
	{
		Mouse(SDV_LEFT_BUTTON, (inputType & MouseButtonDown)? SDV_DOWN : SDV_UP,
			  info->x.i, info->y.i);
		return SINK_EVENT;
	}

	if (inputType & MouseMotion)
	{
		if (zooming == 1 || ract == 1 || gact == 1 || bact == 1
			|| hsact == 1 || vact == 1)
		{
			Motion(info->x.i, info->y.i);
			return SINK_EVENT;
		}
	}

	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/**
 *	Processes a mouse-button-1 event.
 *
 *	@param	button		SDV_LEFT_BUTTON (arg is only here for if we add buttons)
 *	@param	state		SDV_DOWN / SDV_IP
 *	@param	x			Mouse x-position
 *	@param	y			Mouse y-position
 */
void ClutEditor::Mouse(int button, int state, int x, int y)
{
	y = height-y;
	float pickheight,pickh,picks;
	
	// Left button pressed down
	
	if (button == SDV_LEFT_BUTTON && state == SDV_DOWN)
	{
		if (x > 0 && x < width * 0.1)
		{
			// left button pressed down in main color table
			zooming = 1;
			zoom1 = zoom2 = y;
			zoommin = zoommax = (((float)y/(float)height)*(range*COLOR_STRETCH)) + min-(range*COLOR_OFFSET);
			Togl_PostRedisplay(myTogl);
		}
		
		
		if (x > width * 0.2 && x < width * 0.3)
		{
			// left button pressed down in zoom area
			
			pickheight = (((float)y/(float)height)*(zoommax - zoommin))+zoommin;
			
			element = clut->GetNodePtr(pickheight);
			elementheight = element->height;
			clutnode *neighbor;
			neighbor = element->previous;
			if (neighbor)
				elementLength = elementheight - (float)neighbor->height;
			else
				elementLength = -1.0; //"infinite";
			GetHSV(element->color, hsv);

			Togl_PostRedisplay(myTogl);
		}
		
		if (x > width * 0.4 && x < width * 0.45 && y < height * 0.95 && y > height * 0.45)
		{
			// left button pressed down in red color selection area
			
			pickheight = ((y-(height*0.45))*255)/(height*0.5);
			element->color[0]=(unsigned char) pickheight;
			GetHSV(element->color, hsv);
			SetTargetTogl();
			clut->Build();
			Togl_PostRedisplay(targetTogl);
			SetClutEdTogl();
			Togl_PostRedisplay(myTogl);
			ract = 1;
		}
		
		if (x > width * 0.475 && x < width * 0.525 && y < height * 0.95 && y > height * 0.45)
		{
			// left button pressed down in green color selection area
			
			pickheight = ((y-(height*0.45))*255)/(height*0.5);
			element->color[1]=(unsigned char) pickheight;
			GetHSV(element->color, hsv);
			SetTargetTogl();
			clut->Build();
			Togl_PostRedisplay(targetTogl);
			SetClutEdTogl();
			Togl_PostRedisplay(myTogl);
			gact = 1;
		}
		
		if (x > width * 0.55 && x < width * 0.6 && y < height * 0.95 && y > height * 0.45)
		{
			// left button pressed down in blue color selection area
			
			pickheight = ((y-(height*0.45))*255)/(height*0.5);
			element->color[2]=(unsigned char) pickheight;
			GetHSV(element->color, hsv);
			SetTargetTogl();
			clut->Build();
			Togl_PostRedisplay(targetTogl);
			SetClutEdTogl();
			Togl_PostRedisplay(myTogl);
			bact = 1;
		}

		if (x > width * 0.625 && x < width * 0.675 && y < height * 0.15 && y > height * 0.1)
		{
			// left button pressed down in current to clipboard button
			rcb = element->color[0];
			gcb = element->color[1];
			bcb = element->color[2];
			Togl_PostRedisplay(myTogl);
		}

		if (x > width * 0.625 && x < width * 0.675 && y < height * 0.225 && y > height * 0.175)
		{
			// left button pressed down in swap clipboard button

			unsigned char rtmp, gtmp, btmp;
			rtmp = element->color[0];
			gtmp = element->color[1];
			btmp = element->color[2];

			element->color[0] = rcb;
			element->color[1] = gcb;
			element->color[2] = bcb;
			GetHSV(element->color, hsv);

			rcb = rtmp;
			gcb = gtmp;
			bcb = btmp;

			SetTargetTogl();
			clut->Build();
			Togl_PostRedisplay(targetTogl);
			SetClutEdTogl();
			Togl_PostRedisplay(myTogl);
		}

		if (x > width * 0.625 && x < width * 0.675 && y < height * 0.3 && y > height * 0.25)
		{
			// left button pressed down in clipboard to current button
			element->color[0] = rcb;
			element->color[1] = gcb;
			element->color[2] = bcb;
			GetHSV(element->color, hsv);

			SetTargetTogl();
			clut->Build();
			Togl_PostRedisplay(targetTogl);
			SetClutEdTogl();
			Togl_PostRedisplay(myTogl);
		}

		if (x > width * 0.65 && x < width * 0.85 && y < height * 0.95 && y > height * 0.45)
		{
			// left button pressed down in hs area
			pickh = ((y-(height*0.45))*360)/(height*0.5);
			picks = (x-(width*0.65))/(width*0.2);
			if (picks == 0)
			{
				pickh = -1;
			}
			hsv[0] = pickh;
			hsv[1] = picks;
			GetRGB(element->color,hsv);
			SetTargetTogl();
			clut->Build();
			Togl_PostRedisplay(targetTogl);
			SetClutEdTogl();
			Togl_PostRedisplay(myTogl);
			hsact = 1;
		}

		if (x > width * 0.9 && x < width * 0.95 && y < height * 0.95 && y > height * 0.45)
		{
			// left button pressed down in v selection area
			
			pickheight = (y-(height*0.45))/(height*0.5);
			hsv[2]=pickheight;
			GetRGB(element->color,hsv);
			SetTargetTogl();
			clut->Build();
			Togl_PostRedisplay(targetTogl);
			SetClutEdTogl();
			Togl_PostRedisplay(myTogl);
			vact = 1;
		}


	}
	if (button == SDV_LEFT_BUTTON && state == SDV_UP)
	{
		zooming = 0;
		ract = 0;
		gact = 0;
		bact = 0;
		hsact = 0;
		vact = 0;
	}
	
}

/*---------------------------------------------------------------------------*/
/**
 *	Processes a mouse-motion event.
 *
 *	@param	x			Mouse x-position
 *	@param	y			Mouse y-position
 */
void ClutEditor::Motion(int x, int y)
{
	float pickh, picks, pickv;

	y = height-y;
	if (zooming == 1)
	{
		zoom2 = y;
		if (zoom2 > zoom1)
		{
			zoommin = (((float)zoom1/(float)height)*(range*COLOR_STRETCH)) + min-(range*COLOR_OFFSET);
			zoommax = (((float)zoom2/(float)height)*(range*COLOR_STRETCH)) + min-(range*COLOR_OFFSET);
		}
		else
		{
			zoommin = (((float)zoom2/(float)height)*(range*COLOR_STRETCH)) + min-(range*COLOR_OFFSET);
			zoommax = (((float)zoom1/(float)height)*(range*COLOR_STRETCH)) + min-(range*COLOR_OFFSET);
		}
		Togl_PostRedisplay(myTogl);
	}
	
	if (ract == 1 || gact == 1 || bact == 1)
	{
		int c = (int) (((y-(height*0.45))*255)/(height*0.5)); 
		if (c > 255)
		{
			c = 255;
		}
		else if(c < 0)
		{
			c = 0;
		}

		if (ract == 1)
			element->color[0] = c;
		if (gact == 1)
			element->color[1] = c;
		if (bact == 1)
			element->color[2] = c;

		GetHSV(element->color, hsv);
		SetTargetTogl();
		clut->Build();
		Togl_PostRedisplay(targetTogl);
		SetClutEdTogl();
		Togl_PostRedisplay(myTogl);
	}
	
	if (hsact == 1)
	{
		// left button pressed down in hs area
		pickh = ((y-(height*0.45))*360)/(height*0.5);
		picks = (x-(width*0.65))/(width*0.2);
		if (pickh >= 360 || pickh < 0)
		{
			pickh = 0;
		}
		if (picks < 0)
		{
			picks = 0;
		}
		if (picks > 1)
		{
			picks = 1;
		}
		if (picks == 0)
		{
			pickh = -1;
		}
		hsv[0] = pickh;
		hsv[1] = picks;
	}

	if(vact == 1)
	{
		pickv = (y-(height*0.45))/(height*0.5);
		if (pickv > 1)
		{
			pickv = 1;
		}
		if (pickv < 0)
		{
			pickv = 0;
		}

		hsv[2]=pickv;
	}

	if (hsact == 1 || vact == 1)
	{
		GetRGB(element->color,hsv);
		SetTargetTogl();
		clut->Build();
		Togl_PostRedisplay(targetTogl);
		SetClutEdTogl();
		Togl_PostRedisplay(myTogl);
	}
	
}

void ClutEditor::Update(Tcl_Interp * interp, float height)
{
	element->height = height;
	elementheight = height;

	clut->ReAddNode(element);

	min = clut->GetMinheight();
	max = clut->GetMaxheight();
	range = clut->GetHeightRange();

	SetTargetTogl(interp);
	clut->Build();
}
void ClutEditor::DeleteElement(Tcl_Interp * interp)
{
	element = clut->Delete(element);
	elementheight = element->height;
	GetHSV(element->color, hsv);

	min = clut->GetMinheight();
	max = clut->GetMaxheight();
	range = clut->GetHeightRange();

	SetTargetTogl(interp);
	clut->Build();

}

void ClutEditor::AddElement(Tcl_Interp * interp, float height)
{
	clutnode * temp;
	temp = new clutnode;
	temp->height = height;
	temp->color[0]=0;
	temp->color[1]=0;
	temp->color[2]=0;
	temp->color[3]=255;
	clut->AddNode(temp);

	min = clut->GetMinheight();
	max = clut->GetMaxheight();
	range = clut->GetHeightRange();

	SetTargetTogl(interp);
	clut->Build();
}

void ClutEditor::GetHSV(Point4ub color,float * hsv)
{
	float max,min;
	float r,g,b;
	float delta;

	r = color[0]/255.0;
	g = color[1]/255.0;
	b = color[2]/255.0;

	max = r;
	if (g > max)
	{
		max = g;
	}
	if (b > max)
	{
		max = b;
	}

	min = r;
	if (g < min)
	{
		min = g;
	}
	if (b < min)
	{
		min = b;
	}

	hsv[2] = max; // V
	
	if (max == 0)
	{
		hsv[1] = 0; // S
	}
	else
	{
		hsv[1] = (max-min)/max; // S
	}

	if(hsv[1] == 0)
	{
		hsv[0] = -1; //H, negative value is undefined
	}
	else
	{
		delta = max-min;
		if (r == max)
		{
			hsv[0] = (g-b)/delta;
		}
		else
		{
			if (g == max)
			{
				hsv[0] = 2.0+((b-r)/delta);
			}
			else
			{
				if(b == max)
				{
					hsv[0] = 4.0 + ((r - g)/delta);
				}
			}
		}
		hsv[0] *= 60.0;
		if(hsv[0] < 0)
		{
			hsv[0] += 360.0;
		}

	}
}

void ClutEditor::GetRGB(Point4ub color, float *hsv)
{
	int i;
	float f,p,q,t,h;

	if(hsv[1] == 0)
	{
		if(hsv[0] < 0)
		{
			color[0] = color[1] = color[2] = (unsigned char)(255*hsv[2]);
		}
	}
	else
	{
		if(hsv[0] == 360)
		{
			h = 0;
		}
		else
		{
			h = hsv[0];
		}
		h = h / 60.0;
		i = (int) h;
		f = h - i;
		p = hsv[2] * (1.0 - hsv[1]);
		q = hsv[2] * (1.0 - (hsv[1] * f));
		t = hsv[2] * (1.0 - (hsv[1] * (1.0 - f)));
		switch (i)
		{
		case 0:
			color[0] = (unsigned char)(255*hsv[2]);
			color[1] = (unsigned char)(255*t);
			color[2] = (unsigned char)(255*p);
			break;
		case 1:
			color[0] = (unsigned char)(255*q);
			color[1] = (unsigned char)(255*hsv[2]);
			color[2] = (unsigned char)(255*p);
			break;
		case 2:
			color[0] = (unsigned char)(255*p);
			color[1] = (unsigned char)(255*hsv[2]);
			color[2] = (unsigned char)(255*t);
			break;
		case 3:
			color[0] = (unsigned char)(255*p);
			color[1] = (unsigned char)(255*q);
			color[2] = (unsigned char)(255*hsv[2]);
			break;
		case 4:
			color[0] = (unsigned char)(255*t);
			color[1] = (unsigned char)(255*p);
			color[2] = (unsigned char)(255*hsv[2]);
			break;
		case 5:
			color[0] = (unsigned char)(255*hsv[2]);
			color[1] = (unsigned char)(255*p);
			color[2] = (unsigned char)(255*q);
			break;
		}
	}
}
 


