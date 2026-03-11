// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Label.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides for drawing labels in a scale-dependent manner.
 *	This class implements the following responsibilities:
 *		* Read label description from a file
 *		* Support viewing of the label.
 *
 *	Created by mdp 8/29/2002
 */
/*---------------------------------------------------------------------------*/

#include "GZincludes.h"
//#include<GL/glew.h>
//#include <GL/glut.h>
//#include "MainWindow.h"
#include "Label.h"
#include "widgets/Sv_Standards.h"
#include "engine/ViewManager.h"
#include "engine/gzCommandInterface.h"
#include "engine/PosableStateBase.h"
#include "objects/gutm.h"
//#include <iostream>
#include <math.h>
#include "utilities/DottedAddress.h"

using namespace std;

extern int dominantEye;
extern MainWindow *mainWindow;

REGISTER_OBJECT(Label, "label");

const char *Label::LabelNames[] =
			{OBJECT_NAMES, "TEXT", "FONT", "COLOR", "LABEL",
			 "PRIORITY", "LINE", "BACKGROUND", "INTEREST",
			 "SURFACE", "ALWAYS", "AUTO", "DEFAULT", "SHADOW",
			 "BOX", "OUTLINED_BOX", "POINT", "REGION", "SCALE",
			 "TRANSIENT", "FRAGILE", "ROBUST", "OFF", "N_RECORDS",
			 "NEW_SCALE", "STANDARD_9_BY_15", "STANDARD_8_BY_13",
			 "TIMES_ROMAN_10", "TIMES_ROMAN_24", "HELVETICA_10",
			 "HELVETICA_12", "HELVETICA_18",
			 ""};

//#define FONT GLUT_BITMAP_9_BY_15
//#define FONT GLUT_BITMAP_8_BY_13
//#define FONT GLUT_BITMAP_TIMES_ROMAN_10
//#define FONT GLUT_BITMAP_TIMES_ROMAN_24
//#define FONT GLUT_BITMAP_HELVETICA_10
//#define FONT GLUT_BITMAP_HELVETICA_12
//#define FONT GLUT_BITMAP_HELVETICA_18

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------*/
Label::Label()
{
	SetTypeAndVersion("LABEL", 1, 0);
	SetKeywordTable(LabelNames);

	numSpecs = 0;
	scaleMax = -1;
	oldScreenPosValid = GL_FALSE;
}

/*--------------------------------------------------------------------------*/
Label::~Label()
{
	int i;
	if (numSpecs)
	{
		for (i = 0; i < numSpecs; i++)
		{
			if (scaleSpecs[i].text)
				delete scaleSpecs[i].text;
		}
		delete []scaleSpecs;
	}
	if (defaultSpec.text)
		delete defaultSpec.text;
}

/*--------------------------------------------------------------------------*/
Label::LabelSpec::LabelSpec()
{
	scaleMin = -1;
	text = NULL;
	font = GLUT_BITMAP_HELVETICA_10;
	color[0] = color[1] = color[2] = color[3] = 0xFF;
	backgroundColor[0] = backgroundColor[1] = backgroundColor[2] = 0x00;
	backgroundColor[3] = 0x00;
	priority = -1;
	behaviorFlags = UseDefaultMask;
	surface = NULL;
	robustness = ROBUST;
	usageFlags = Blank;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void Label::DrawLowRes(float degradation, Window3D *win)
{
	int robustVal = ROBUST;

	DetermineSpec(win);

	if (!currentSpec)
		return;
	if (currentSpec->usageFlags & UseRobustness)
		robustVal = currentSpec->robustness;
	else if (defaultSpec.usageFlags & UseRobustness)
		robustVal = defaultSpec.robustness;

	if (robustVal != OFF && robustVal != FRAGILE)
	{
		if (isEditableFlag)
			DrawEditGhosts(win, UseTransientVal);
		else
			DrawSpec(win, UseTransientVal);
	}

	ResetHighRes();
}

/*--------------------------------------------------------------------------*/
int Label::DrawHighRes(int eye, Window3D *win)
{
	int robustVal = ROBUST;

	if (!doneFlag)
	{
		DetermineSpec(win);

		if (!currentSpec)
		{
			doneFlag = 1;
			return doneFlag;
		}

		if (currentSpec->usageFlags & UseRobustness)
			robustVal = currentSpec->robustness;
		else if (defaultSpec.usageFlags & UseRobustness)
			robustVal = defaultSpec.robustness;

		if (robustVal != OFF && robustVal != TRANSIENT)
		{
			if (isEditableFlag)
				DrawEditGhosts(win, UseFragileVal);
			else
				DrawSpec(win, UseFragileVal);
		}
		else
		{
			if (isEditableFlag)
				DrawEditGhosts(win, UseFragileVal);
			doneFlag = 1;
		}

		if (eye != dominantEye && eye != MONO_EYE)
			return doneFlag;
		else
			doneFlag = 1;
	}
	else
		doneFlag = 1;
	
	return doneFlag;
}

/*--------------------------------------------------------------------------*/
void Label::DrawSpec(Window3D *win, int robustnessType, GLboolean asEditGhost)
{
	char *text;
	void *font;
	const float *minPoint, *maxPoint;
	gz4d::Point<double> pos(0.0,0.0,0.0);
	GLboolean heightAndWidthValid = GL_FALSE;
	float xOffset, yOffset;
	int usageFlags = currentSpec->usageFlags;
	int behaviorFlags = (defaultSpec.behaviorFlags & ~usageFlags) |
						(currentSpec->behaviorFlags & usageFlags);
	int backgroundRobustness = UseBackgroundRobustnessMask & robustnessType & behaviorFlags;
	GLboolean shadow = ((behaviorFlags & UseBoxOrShadowMask) == UseShadow && backgroundRobustness);
	GLboolean outline = ((behaviorFlags & UseBoxOrShadowMask) == UseBoxWithOutline && backgroundRobustness);
	GLboolean filledBox = ((behaviorFlags & UseBoxOrShadowMask) == UseBackgroundBox && backgroundRobustness);
	float scale = win->getPoseInfo()->getScale(), scaleFactor = 1.0;

	if (isEditable())
	{
		scaleFactor = scale / currentSpec->scaleMin;
		if (scaleFactor > 1.0f)
			scaleFactor = 1.0f;
	}

	glShadeModel(GL_FLAT);
	glNormal3f(0.0, 0.0, 1.0);

//	glDepthMask(GL_FALSE);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (usageFlags & UseText)
	{
		text = currentSpec->text;
		heightAndWidthValid = GL_TRUE;
	}
	else
		text = defaultSpec.text;

	if (usageFlags & UseFont)
	{
		font = currentSpec->font;
		heightAndWidthValid = GL_TRUE;
	}
	else
		font = defaultSpec.font;

	if (usageFlags & UseColor)
		glColor4ubv(currentSpec->color);
	else
		glColor4ubv(defaultSpec.color);

	if (behaviorFlags & UseInterestMask)
	{
		if (usageFlags & UseInterestMask)
			minPoint = currentSpec->interestRegionMin;
		else
			minPoint = defaultSpec.interestRegionMin;

		if (behaviorFlags & UseInterestPoint)
		{
			COPY_POINT(pos, minPoint);
		}
		else
		{
			if (usageFlags & UseInterestMask)
				maxPoint = currentSpec->interestRegionMax;
			else
				maxPoint = defaultSpec.interestRegionMax;
			//&&& project to find where to put it
		}
	}
	else
		pos[X] = pos[Y] = pos[Z] = 0.0;

	if (heightAndWidthValid)
	{
		xOffset = -currentSpec->width / 2;
		yOffset = -currentSpec->height / 2;
	}
	else
	{
		xOffset = -defaultSpec.width / 2;
		yOffset = -defaultSpec.height / 2;
	}
	if (!asEditGhost && !renderingForPicking)
		win->printAt(pos[X], pos[Y], pos[Z], text, shadow, font, (int)xOffset, (int)yOffset);

	if (outline || filledBox || asEditGhost || renderingForPicking)
	{
		ViewManager *view = win->getViewInfo();
		const GLint *viewport = view->getViewport();
                gz4d::Point<double> position = pose->getPosition();
                gz4d::Point<double> screenPos, worldPos;
                double zPos;
		GLboolean drawBackground = GL_TRUE;

		glLineWidth(1.0);
		xOffset -= 3.5; //always use a .5 number here, so line moves in tandem with text
		yOffset -= 3.5;

		worldPos = pos + position;
		view->plotWorldPoint(worldPos, screenPos); //get screen coord of center
		//translate into 2D coordinates
		screenPos[Y] = viewport[3] - screenPos[Y] - 3;
		screenPos[Z] = 1 - screenPos[Z] * 2 - 0.01;

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0.0, viewport[2], 0.0, viewport[3]);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		if (outline || isEditableFlag)
		{
			zPos = screenPos[Z];
			if (isEditableFlag)
			{
				xOffset = (xOffset + 0.5f) * scaleFactor;
				yOffset = (yOffset + 0.5f) * scaleFactor;
				if (oldScreenPosValid)
				{
					glBegin(GL_LINES);
					glVertex3d(screenPos[X], screenPos[Y], zPos);
					glVertex3d(oldScreenPos[X], oldScreenPos[Y], zPos);
					glEnd();
				}
				COPY_POINT(oldScreenPos, screenPos);
				oldScreenPosValid = GL_TRUE;
			}
			else
			{
				xOffset += 0.5f; //always remove .5 portion as done here, so box moves in tandem with text
				yOffset += 0.5f;
			}
			if (outline || asEditGhost)
			{
				glBegin(GL_LINE_LOOP);
				glVertex3d(screenPos[X] + xOffset, screenPos[Y] + yOffset, zPos);
				glVertex3d(screenPos[X] - xOffset, screenPos[Y] + yOffset, zPos);
				glVertex3d(screenPos[X] - xOffset, screenPos[Y] - yOffset, zPos);
				glVertex3d(screenPos[X] + xOffset, screenPos[Y] - yOffset, zPos);
				glEnd();
			}
		}

		if (usageFlags & UseBackgroundColor)
		{
			if (currentSpec->backgroundColor[3] <= 0.0)
				drawBackground = GL_FALSE;
			else
				glColor4ubv(currentSpec->backgroundColor);
		}
		else
		{
			if (defaultSpec.backgroundColor[3] <= 0.0)
				drawBackground = GL_FALSE;
			else
				glColor4ubv(defaultSpec.backgroundColor);
		}

		if (drawBackground || renderingForPicking)
		{
			glBegin(GL_QUADS);
			glVertex3d(screenPos[X] - xOffset, screenPos[Y] - yOffset, screenPos[Z]);
			glVertex3d(screenPos[X] + xOffset, screenPos[Y] - yOffset, screenPos[Z]);
			glVertex3d(screenPos[X] + xOffset, screenPos[Y] + yOffset, screenPos[Z]);
			glVertex3d(screenPos[X] - xOffset, screenPos[Y] + yOffset, screenPos[Z]);
			glEnd();
		}
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
//	glBlendFunc(GL_ONE, GL_ZERO);
//	glDepthMask(GL_TRUE);
}

/*--------------------------------------------------------------------------*/
void Label::DetermineSpec(Window3D *win)
{
	float scale;
	int i = -1;
	LabelSpec *nextSpec;

	scale = win->getPoseInfo()->getScale();

	currentSpec = NULL;
	nextSpec = &defaultSpec;
	if (scaleMax > 0 && scale > scaleMax)
		return;

	while (1)
	{
		if (nextSpec->scaleMin > 0 && scale < nextSpec->scaleMin)
			return;

		currentSpec = nextSpec;
		i++;
		if (i >= numSpecs)
			break;
		nextSpec = &(scaleSpecs[i]);
	}
}

/*--------------------------------------------------------------------------*/
void Label::DrawEditGhosts(Window3D *win, int robustnessType)
{
	int i = -1;
	LabelSpec *validSpec;

	validSpec = currentSpec;
	currentSpec = NULL;

	oldScreenPosValid = GL_FALSE;
	for (currentSpec = &defaultSpec; i < numSpecs; currentSpec = &(scaleSpecs[++i]))
	{
		if (currentSpec == validSpec)
			DrawSpec(win, robustnessType);
		else
			DrawSpec(win, robustnessType, GL_TRUE);
	}
}

/*--------------------------------------------------------------------------*/
int Label::processField(int option, char **fields)
{
	int i = 1;
	int val, numArgs;
	Addressable *addressable;
	Gutm *gutm;

	switch(option)
	{
		case N_RECORDS:
			if (currentSpec != &defaultSpec)
			{
				cerr << "N_RECORDS keyword not allowed after LOCAL_ORIGIN line." << endl;
				return 1;
			}
			numSpecs = ConvertInt(i);
			scaleSpecs = new LabelSpec[numSpecs];
			break;
		case SCALE:
			if (currentSpec != &defaultSpec)
			{
				cerr << "SCALE keyword not allowed after LOCAL_ORIGIN line." << endl;
				return 1;
			}
			currentSpec->scaleMin = ConvertFloat(i);
			if (currentSpec->scaleMin <= 0.0)
				currentSpec->scaleMin = -1.0;
			numArgs = getNumFields();
			if (i < numArgs)
			{
				scaleMax = ConvertFloat(i);
				if (scaleMax <= 0.0)
					scaleMax = -1.0;
			}
			break;
		case LABEL:
			currentSpec->robustness = Lookup(fields[i++]);
			currentSpec->usageFlags |= UseRobustness;
			break;
		case TEXT:
			currentSpec->text = ReleaseField(i);
			currentSpec->usageFlags |= UseText;
			determinePrintedSize();
			break;
		case FONT:
			currentSpec->font = ConvertFont(fields[i++]);
			if (defaultSpec.font == NULL)
				return 1;
			currentSpec->usageFlags |= UseFont;
			determinePrintedSize();
			break;
		case COLOR: //applies to text, line, and box outline
			ConvertFloatColor(currentSpec->color, i);
			currentSpec->usageFlags |= UseColor;
			break;
		case PRIORITY:
			currentSpec->priority = ConvertInt(i);
			currentSpec->usageFlags |= UsePriority;
			break;
		case LINE:
			//line display policy
			val = Lookup(fields[i++]);
			currentSpec->behaviorFlags &= ~UseLineMask;
			switch (val)
			{
				case ALWAYS:
					currentSpec->behaviorFlags |= UseLineAlways;
					break;
				case AUTO:
					currentSpec->behaviorFlags |= UseLineAuto;
					break;
				case OFF:
					currentSpec->behaviorFlags |= UseLineNever;
					break;
				case DEFAULT:
					currentSpec->behaviorFlags |= UseDefaultMask & UseLineMask;
					break;
				default:
					cerr << "Invalid LINE specification.  Should be" << endl;
					cerr << "one of ALWAYS, AUTO, DEFAULT, or OFF." << endl;
					return 1;
					break;
			}
			if (val != DEFAULT)
				currentSpec->usageFlags |= UseLineMask;
			//optional line robustness
			numArgs = getNumFields();
			if (numArgs <= i)
				break;
			currentSpec->behaviorFlags &= ~UseLineRobustnessMask;
			val = ConvertRobustness(fields[i++]);
			if (val == -1)
				return 1;
			if (val == -2)
				currentSpec->behaviorFlags |= UseDefaultMask & UseLineRobustnessMask;
			else
			{
				currentSpec->behaviorFlags |= (val & UseLineRobustnessMask);
				currentSpec->usageFlags |= UseLineRobustnessMask;
			}
			break;
		case BACKGROUND:
			//background display policy
			numArgs = getNumFields();
			val = Lookup(fields[i++]);
			if (val == COLOR)
			{
				ConvertFloatColor(currentSpec->backgroundColor, i);
				currentSpec->usageFlags |= UseBackgroundColor;
				if (numArgs <= i)
					break;
				val = Lookup(fields[i++]);
			}
			currentSpec->behaviorFlags &= ~UseBoxOrShadowMask;
			switch (val)
			{
				case SHADOW:
					currentSpec->behaviorFlags |= UseShadow;
					break;
				case BOX:
					currentSpec->behaviorFlags |= UseBackgroundBox;
					break;
				case OUTLINED_BOX:
					currentSpec->behaviorFlags |= UseBoxWithOutline;
					break;
				case OFF:
					break;
				case DEFAULT:
					currentSpec->behaviorFlags |= UseDefaultMask & UseBoxOrShadowMask;
					break;
				default:
					cerr << "Invalid BACKGROUND specification.  Should be" << endl;
					cerr << "one of SHADOW, BOX, OUTLINED_BOX, DEFAULT, or OFF." << endl;
					return 1;
					break;
			}
			if (val != DEFAULT)
				currentSpec->usageFlags |= UseBoxOrShadowMask;
			//optional background robustness
			if (numArgs <= i)
				break;
			currentSpec->behaviorFlags &= ~UseBackgroundRobustnessMask;
			val = ConvertRobustness(fields[i++]);
			if (val == -1)
				return 1;
			if (val == -2)
				currentSpec->behaviorFlags |= UseDefaultMask & UseBackgroundRobustnessMask;
			else
			{
				currentSpec->behaviorFlags |= (val & UseBackgroundRobustnessMask);
				currentSpec->usageFlags |= UseBackgroundRobustnessMask;
			}
			break;
		case SURFACE: //always draw above surface... if INTEREST is REGION, suface must be in region for label to show there.
			if (gzCommandInterface::findAddressIn(fields[i], ObjectNeighborhood, addressable, PosableNeighborhood) != TCL_OK)
			{
				cerr << Tcl_GetStringResult(ToglWindow::interp) << endl;
				Tcl_ResetResult(ToglWindow::interp);
				return 1;
			}
			DYNAMIC_CAST(gutm, Gutm *, addressable);
			if (!gutm)
			{
				cerr << "Invalid SURFACE: " << fields[i] << "." << endl;
				return 1;
			}
			currentSpec->surface = gutm;
			currentSpec->usageFlags |= UseSurface;
			break;
		case INTEREST:
			val = Lookup(fields[i++]);
			ConvertFloatPoint(currentSpec->interestRegionMin, i);
			currentSpec->behaviorFlags &= ~UseInterestMask;
			switch (val)
			{
				case POINT:
					currentSpec->behaviorFlags |= UseInterestPoint;
					break;
				case REGION:
					currentSpec->behaviorFlags |= UseInterestRegion;
					ConvertFloatPoint(currentSpec->interestRegionMax, i);
					break;
				default:
					cerr << "Invalid INTEREST specification.  Should be" << endl;
					cerr << "either POINT or REGION." << endl;
					return 1;
					break;
			}
			currentSpec->usageFlags |= UseInterestMask;
			break;
		case PROJECT: case REVERSED_XY:
			if (currentSpec == &defaultSpec)
			{
				cerr << fields[0] << " is not yet supported by Label." << endl;
				return 1;
			}
			break;
		default:
			if (currentSpec == &defaultSpec)
				return Object::processField(option, fields);
			break;
	}
	return 0;
}

/*--------------------------------------------------------------------------*/
// documented in Object
int Label::readFrom(FILE *infile)
{
	char **fields;
	int spec = 0, i, item, retval;
	
	//parse header
	currentSpec = &defaultSpec;
	if (Object::readFrom(infile))
		return 1;

	//parse scale-specs.
	for (fields = NextLine(); fields; fields = NextLine())
	{
		i = 0;
		item = Lookup(fields[i++]);
		if (item == NEW_SCALE) //note: these should work from most generic to most specific.
		{
			if (spec >= numSpecs) //skip any more records than N_RECORDS
				return 0;
			currentSpec = &(scaleSpecs[spec++]);
			currentSpec->scaleMin = ConvertFloat(i);
			if (currentSpec->scaleMin <= 0.0)
				currentSpec->scaleMin = -1.0;
		}
		else
		{
			retval = processField(item, fields);
			if (retval != 0)
				return retval;
		}
	}

	return 0;
}

/*---------------------------------------------------------------------------*/
int Label::writeTo(FILE *file)
{
	int i;

    fprintf(file, "%s %d\n", Keyword(N_RECORDS), numSpecs);
    fprintf(file, "%s %g %g\n", Keyword(SCALE), defaultSpec.scaleMin, scaleMax);
	currentSpec = &defaultSpec;
	writeSpecTo(file);

    Object::writeTo(file);

	for (i = 0; i < numSpecs; i++)
	{
		currentSpec = &(scaleSpecs[i]);
		fprintf(file, "%s %g\n", Keyword(NEW_SCALE), currentSpec->scaleMin);
		writeSpecTo(file);
	}

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
int Label::writeSpecTo(FILE *file)
{
	int word;

	if (currentSpec->usageFlags & UseText)
		fprintf(file, "%s \"%s\"\n", Keyword(TEXT), currentSpec->text);
	if (currentSpec->usageFlags & UseRobustness)
		fprintf(file, "%s %s\n", Keyword(LABEL), Keyword(currentSpec->robustness));
	if (currentSpec->usageFlags & UseFont)
		fprintf(file, "%s %s\n", Keyword(FONT), ConvertFont(currentSpec->font));
	if (currentSpec->usageFlags & UseColor)
	{
		fprintf(file, "%s", Keyword(COLOR));
		WriteFloatColor(file, currentSpec->color);
		fprintf(file, "\n");
	}
	if (currentSpec->usageFlags & UsePriority)
		fprintf(file, "%s %d\n", Keyword(PRIORITY), currentSpec->priority);
	if ((currentSpec->usageFlags & UseLineMask) || (currentSpec->usageFlags & UseLineRobustnessMask))
	{
		if (currentSpec->usageFlags & UseLineMask)
		{
			switch (currentSpec->behaviorFlags & UseLineMask)
			{
				case UseLineAlways:
					word = ALWAYS;
					break;
				case UseLineAuto:
					word = AUTO;
					break;
				case UseLineNever:
					word = OFF;
					break;
				default:
					word = DEFAULT;
					break;
			}
		}
		else
			word = DEFAULT;
		fprintf(file, "%s %s", Keyword(LINE), Keyword(word));
		if (currentSpec->usageFlags & UseLineRobustnessMask)
			fprintf(file, " %s", ConvertRobustness(currentSpec->behaviorFlags, UseLineRobustnessMask));
		fprintf(file, "\n");
	}
	if ((currentSpec->usageFlags & UseBackgroundColor)
		|| (currentSpec->usageFlags & UseBoxOrShadowMask)
		|| (currentSpec->usageFlags & UseBackgroundRobustnessMask))
	{
		fprintf(file, "%s", Keyword(BACKGROUND));
		if (currentSpec->usageFlags & UseBackgroundColor)
		{
			fprintf(file, " %s", Keyword(COLOR));
			WriteFloatColor(file, currentSpec->backgroundColor, GL_TRUE);
		}
		if (currentSpec->usageFlags & UseBoxOrShadowMask)
		{
			switch (currentSpec->behaviorFlags & UseBoxOrShadowMask)
			{
				case Blank:
					word = OFF;
					break;
				case UseShadow:
					word = SHADOW;
					break;
				case UseBackgroundBox:
					word = BOX;
					break;
				case UseBoxWithOutline:
					word = OUTLINED_BOX;
					break;
				default: //should be impossible
					word = DEFAULT;
					break;
			}
		}
		else
			word = DEFAULT;
		fprintf(file, " %s", Keyword(word));
		if (currentSpec->usageFlags & UseBackgroundRobustnessMask)
			fprintf(file, " %s", ConvertRobustness(currentSpec->behaviorFlags, UseBackgroundRobustnessMask));
		fprintf(file, "\n");
	}
	if ((currentSpec->usageFlags & UseSurface) && currentSpec->surface)
		fprintf(file, "%s %s\n", Keyword(SURFACE), currentSpec->surface->getAddress()->getAddress().c_str());
	if (currentSpec->usageFlags & UseInterestMask)
	{
		fprintf(file, "%s ", Keyword(INTEREST));
		if ((currentSpec->behaviorFlags & UseInterestMask) == UseInterestPoint)
			word = POINT;
		else
			word = REGION;
		fprintf(file, "%s %g %g %g", Keyword(word), currentSpec->interestRegionMin[X], currentSpec->interestRegionMin[Y], currentSpec->interestRegionMin[Z]);
		if (word == REGION)
			fprintf(file, " %g %g %g", currentSpec->interestRegionMax[X], currentSpec->interestRegionMax[Y], currentSpec->interestRegionMax[Z]);
		fprintf(file, "\n");
	}
	return TCL_OK;
}

/*--------------------------------------------------------------------------*/
void *Label::ConvertFont(char *fontName)
{
	int result = Lookup(fontName);

	switch (result)
	{
		case STANDARD_9_BY_15:
			return GLUT_BITMAP_9_BY_15;
			break;
		case STANDARD_8_BY_13:
			return GLUT_BITMAP_8_BY_13;
			break;
		case TIMES_ROMAN_10:
			return GLUT_BITMAP_TIMES_ROMAN_10;
			break;
		case TIMES_ROMAN_24:
			return GLUT_BITMAP_TIMES_ROMAN_24;
			break;
		case HELVETICA_10:
			return GLUT_BITMAP_HELVETICA_10;
			break;
		case HELVETICA_12:
			return GLUT_BITMAP_HELVETICA_12;
			break;
		case HELVETICA_18:
			return GLUT_BITMAP_HELVETICA_18;
			break;
		default:
			cerr << "Invalid FONT: " << fontName << "." << endl;
			cerr << "Should be one of STANDARD_9_BY_15, STANDARD_8_BY_13, TIMES_ROMAN_10," << endl;
			cerr << "TIMES_ROMAN_24, HELVETICA_10, HELVETICA_12, or HELVETICA_18" << endl;
			break;
	}
	return NULL;
}

/*--------------------------------------------------------------------------*/
const char *Label::ConvertFont(void *font)
{
	int result;

	if (font == GLUT_BITMAP_9_BY_15)
		result = STANDARD_9_BY_15;
	else if (font == GLUT_BITMAP_8_BY_13)
		result = STANDARD_8_BY_13;
	else if (font == GLUT_BITMAP_TIMES_ROMAN_10)
		result = TIMES_ROMAN_10;
	else if (font == GLUT_BITMAP_TIMES_ROMAN_24)
		result = TIMES_ROMAN_24;
	else if (font == GLUT_BITMAP_HELVETICA_10)
		result = HELVETICA_10;
	else if (font == GLUT_BITMAP_HELVETICA_12)
		result = HELVETICA_12;
	else if (font == GLUT_BITMAP_HELVETICA_18)
		result = HELVETICA_18;
	else
		result = HELVETICA_10;

	return Keyword(result);
}

/*--------------------------------------------------------------------------*/
int Label::ConvertRobustness(char *field)
{
	int result = Lookup(field);

	switch (result)
	{
		case TRANSIENT:
			return UseTransientVal;
			break;
		case FRAGILE:
			return UseFragileVal;
			break;
		case ROBUST:
			return UseRobustVal;
			break;
		case OFF:
			return Blank;
			break;
		case DEFAULT:
			return -2;
		default:
			cerr << "Invalid robustness value: " << field << ".  Should be" << endl;
			cerr << "one of TRANSIENT, FRAGILE, ROBUST, DEFAULT, or OFF." << endl;
			break;
	}
	return -1;
}

/*--------------------------------------------------------------------------*/
const char *Label::ConvertRobustness(int robustness, int mask)
{
	int result = DEFAULT;

	robustness &= mask;

	if (robustness == (UseTransientVal & mask))
		result = TRANSIENT;
	else if (robustness == (UseFragileVal & mask))
		result = FRAGILE;
	else if (robustness == (UseRobustVal & mask))
		result = ROBUST;
	else if (robustness == (Blank & mask))
		result = OFF;

	return Keyword(result);
}

/*--------------------------------------------------------------------------*/
void Label::determinePrintedSize()
{
	const unsigned char *text;
	void *font;
	int usageFlags = currentSpec->usageFlags;
	
	if (usageFlags & UseText)
		text = (unsigned char *)currentSpec->text;
	else
		text = (unsigned char *)defaultSpec.text;

	if (!text)
	{
		currentSpec->height = currentSpec->width = 0;
		return;
	}

	if (usageFlags & UseFont)
		font = currentSpec->font;
	else
		font = defaultSpec.font;

	currentSpec->width = glutBitmapLength(font, text);
	
	if(font == GLUT_BITMAP_9_BY_15)
	{
		currentSpec->height = 15;
	}
	else if (font == GLUT_BITMAP_8_BY_13)
	{
		currentSpec->height = 13;
	}
	else if (font == GLUT_BITMAP_TIMES_ROMAN_10)
	{
		currentSpec->height = 10;
	}
	else if (font == GLUT_BITMAP_TIMES_ROMAN_24)
	{
		currentSpec->height = 24;
	}
	else if (font == GLUT_BITMAP_HELVETICA_10)
	{
			currentSpec->height = 10;
	}
	else if (font == GLUT_BITMAP_HELVETICA_12)
	{
		currentSpec->height = 12;
	}
	else if (font == GLUT_BITMAP_HELVETICA_18)
	{
		currentSpec->height = 18;
	}
}
