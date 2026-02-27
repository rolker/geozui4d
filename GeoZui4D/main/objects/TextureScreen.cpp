/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// TextureScreen.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is a screen that can be placed in a scene to display a texture
 *
 *	Created by rja Jan 15, 2002
 */
/*---------------------------------------------------------------------------*/
#include "TextureScreen.h"
#include "engine/ObjectPoseState.h"
#include "widgets/Sv_Standards.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "engine/InputMediator.h"
#include "GZincludes.h"
#include "interactors/DoubleClickInteractor.h"
#include "tclLinkage.h"
#include <boost/filesystem.hpp>
#include "objects/gutm.h"
#include "objects/NetCDF.h"
#include "engine/TextureMaps/ImageMap.h"
#include "engine/Interactor.h"

#include "utilities/Utilities.h"
#include <gz4d/time.h>
#include <boost/algorithm/string.hpp>
#include "engine/QuadTreeTile.h"

extern int dominantEye;
extern MainWindow *mainWindow;

using namespace std;

REGISTER_OBJECT_WITH_MENU(TextureScreen, "screen") = "{"
DEFAULT_OBJECT_MENU_ITEMS
"{Play} play "
"{Stop} stop "
"{Pause} pause "        
"}";

const char *ScreenNames[] = {OBJECT_NAMES, "SCREEN_WIDTH",
							 	"SCREEN_HEIGHT","YAW","PITCH","ROLL",
								"LOAD", "PANELS", "PANEL_ANGLE", "BORDER", ""};

const EventType *TextureScreen::textureScreenInputType =
				Interactor::newEventType("textureScreen", Interactor::DataAsString);

float TextureScreen::emphasisFactor = 1.0;
int TextureScreen::quadCornerIndex[4] = {4, 6, 10, 12};

/*---------------------------------------------------------------------------*/
TextureScreen::TextureScreen()
: QuadTreeObject(GL_FALSE),numNybblesPerColor(2),currentDGTimeStep(dgTimes.end())
{
    SetTypeAndVersion("SCREEN", 1, 0);
    SetKeywordTable(ScreenNames);

    dgWidth = 0;
    dgHeight = 0;


    corners = NULL;
    texCoords = NULL;
    numPanels = 1;
    angleOffset = 0.0;
    
    pose = new ObjectPoseState(this);
    
    load = 0;

    orient.axis[Yaw]=0.0;
    orient.axis[Pitch]=0.0;
    orient.axis[Roll]=0.0;

    screen_width = 1.0;
    screen_height = 1.0;
    pixelSize = eastScale = northScale = gz4d::Nan<double>();
    quality = 0.0;

    bgColor[0] = 0.7;
    bgColor[1] = 0.7;
    bgColor[2] = 0.7;    

    borderWidth = 2.0;

    borderColor[0] = 1.0;
    borderColor[1] = 0.0;
    borderColor[2] = 0.0;

    seeTru = 0;
    showBorder = 1;
    fuseable = 0;
    twoFaced = false;

    fusionTransparency = 0.0;
    generatedInternally = false;

    generatePanelCoordinates();

    // The distortion grid is not available (yet).
    screen_height = 0;
    screen_width = 0;
    initialized = false;

}


TextureScreen::~TextureScreen()
{
    delete [] corners;
    delete [] texCoords;
}

/*---------------------------------------------------------------------------*/
// documented in Object
int TextureScreen::readFrom(FILE *infile)
{
    int item;
    char **fields = NULL;
    int headerdone = 0;
    int i;
    
    char filename[256];

    Point3d Position; // georefrence
  //int pixw, pixh;
    	
	//Default values

    numPanels = 1;
    Position[X] = Position[Y] = Position[Z] = 0.0;

    screen_width = 0.0;
    screen_height = 0.0;
    
    for(fields = NextLine(); !headerdone && fields != NULL; fields = NextLine())
    {
        i = 0;
        item = Lookup(fields[i++]);
        switch(item)
        {
            case SCREEN_WIDTH:
                screen_width = ConvertFloat(i);
                break;
            case SCREEN_HEIGHT:
                screen_height = ConvertFloat(i);
                break;
            case YAW:
                orient.axis[Yaw]=	ConvertFloat(i);
                break;			
            case PITCH:
                orient.axis[Pitch]=	ConvertFloat(i);
                break;			
            case ROLL:
                orient.axis[Roll]=	ConvertFloat(i);
                break;			
            case LOAD:
                load = 1;
                break;			
            case PANELS:
                numPanels = ConvertInt(i);
                break;			
            case PANEL_ANGLE:
                angleOffset = ConvertFloat(i);
                break;			
            case BORDER:
                if (Lookup(fields[i++]) == ON)
                    showBorder = GL_TRUE;
                else
                    showBorder = GL_FALSE;
                break;
            case LOCAL_ORIGIN:
                Position[0] = ConvertFloat(i);
                Position[1] = ConvertFloat(i);
                Position[2] = ConvertFloat(i);
                headerdone = 1;
                break;
            case -1:
                return 1;
                break;
        }
        if(headerdone)
            break;
    }

    SetPosition(Position);

    if(	fgets(filename, 255, infile))
    {
        
        bfs::path fname(GetFilenameAsPath().parent_path()/boost::algorithm::trim_copy(std::string(filename)));
        if (!setTextureFile(fname, leftTexture))
            return 1;
        adjustSize(leftTexture);
        if (!showBorder)
        {
            std::shared_ptr<ImageMap> imap = std::dynamic_pointer_cast<ImageMap>(leftTexture);
            if (imap)
                imap->SetNoBorder();
        }
        if(load)
        {
            leftTexture->Load();
        }
    }

    if(fgets(filename, 255, infile) && strlen(filename) > 3) //strlen check there in case of spurious \n's at end of file
    {
        bfs::path fname(GetFilenameAsPath().parent_path()/filename);
        if (!setTextureFile(fname, rightTexture))
            return 1;
        if(load)
            rightTexture->Load();
    }        

    // set pose
    pose->setOrientation(orient,ChangeRotate);

    generatePanelCoordinates();
    return 0;
}

/*---------------------------------------------------------------------------*/
bool TextureScreen::setTextureFile(bfs::path fname, std::shared_ptr<TextureMap> &texture)
{
    std::string suffix = extension(fname);

    for (int j = 0; j <= 3; j++)
        suffix[j] = tolower(suffix[j]);

    texture = CreateTexture(suffix);
    if (!texture)
        return false;

    if (quadtreeStatus) //part of a quadtree, so no mipmaps (added by mdp 3/20/2009)
    {
        std::shared_ptr<ImageMap> imap = std::dynamic_pointer_cast<ImageMap>(texture);
        if (imap)
            imap->UseMipmaps(false);
    }

    if(texture->SetFilename(fname.string()))
    {
        cerr << "Problem assigning name to texture...\n";
        texture.reset();
        return false;
    }

    texture->setColorFormat(convertFormat());
    return true;
}

/*---------------------------------------------------------------------------*/
TextureMap::ColorFormat TextureScreen::convertFormat()
{
    switch (numNybblesPerColor)
    {
        case 4: case 3:
            return TextureMap::RGBA16;
        case 2:
            return TextureMap::RGBA8;
            //return TextureMap::RGB5;
        case 1:
            if (useTransparency)
                return TextureMap::RGBA4;
            else
                return TextureMap::RGB5;
        case 0:
            return TextureMap::RGB_byte;
    }
    //default:
    //return TextureMap::RGBA16;
    return TextureMap::RGBA8;
}

/*---------------------------------------------------------------------------*/
void TextureScreen::setColorDepth(int nybblesPerColor, bool transparency)
{
    numNybblesPerColor = nybblesPerColor;
    useTransparency = transparency;
    if (leftTexture)
        leftTexture->setColorFormat(convertFormat());
    if (rightTexture)
        rightTexture->setColorFormat(convertFormat());
}

/*---------------------------------------------------------------------------*/
void TextureScreen::adjustSize(std::shared_ptr<TextureMap> &texture)
{
    int pixw, pixh;

    // figure out width and height, if one is missing
    if((screen_width==0.0) || (screen_height == 0.0))
    {
   	    if(screen_width == screen_height)  // they must be 0 to be ==
        {
            screen_width = 1;
        }
        if (!generatedInternally)
            texture->InitDims();
        pixw = texture->GetWidth();
        pixh = texture->GetHeight();
        if (quality <= 0.0)
            quality = pixw * pixh / 10000.0;
        if (gz4d::IsNan(eastScale)) //no header info
        {
            if(screen_width==0.0)
            {
                screen_width = ((double)pixw/(double)pixh)*screen_height;
            }
            else
            {
                screen_height = ((double)pixh/(double)pixw)*screen_width;
            }
        }
        else
        {
            screen_width = pixw * eastScale;
            if(screen_width < 0.0)
                screen_width *= -1;
            screen_height = pixh * northScale;
            if(screen_height < 0.0)
                screen_height *= -1;
        }
    }
}

/*---------------------------------------------------------------------------*/
bool TextureScreen::readWorldFile(std::shared_ptr<TextureMap> &texture, bfs::path fname)
{
    float rot1, rot2;
    int i;
    FILE * hfile;
    std::string suffix = extension(fname);
    double position[3] = {0.0, 0.0, 0.0};

    for (int j = 0; j <= 3; j++)
        suffix[j] = tolower(suffix[j]);

    if(suffix == ".jgw" || suffix == ".tfw" || suffix == ".pgw")
    {
        hfile = fopen(fname.string().c_str(),"r");
        if(hfile)
        {
            i = fscanf(hfile, "%f %f %f %f %lf %lf", &eastScale, &rot1, &rot2, &northScale, &position[X], &position[Y]);
            pixelSize = (fabs(eastScale) < fabs(northScale))? fabs(eastScale) : fabs(northScale);
            SetPosition(position);
            if(i == 6 || rot1 == 0 || rot2 == 0)	// must have all 6 entries, and 0 rots
            {
                fuseable = 1;
                numPanels = 1;
                if (setTextureFile(fname, texture))
                {
                    adjustSize(texture);
                    return true;
                }
                return false;
            }
            else
                eastScale = northScale = gz4d::Nan<double>();
        }
    }
    if (setTextureFile(fname, texture))
    {
        adjustSize(texture);
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/
bool TextureScreen::loadTexture(bfs::path fname)
{
    leftTextureName = fname;
    if (!setTextureFile(leftTextureName, leftTexture))
        return 1;
    adjustSize(leftTexture);
    initialized = true;
    loadLeftTexture();
    return 0;
}

void TextureScreen::buildTexture()
{
    if (leftTexture)
        leftTexture->Build();
    if (rightTexture)
        rightTexture->Build();
}

/*---------------------------------------------------------------------------*/
void TextureScreen::generatePanelCoordinates()
{
	int i, nVertices;
	double minHeight=0.0, maxHeight=0.0, minWidth=0.0, maxWidth=0.0, a, r=0.0, radAngle=0.0;

    if (corners)
   	    delete [] corners;
    if (texCoords)
	    delete [] texCoords;

    //define corners of screen
	nVertices = (numPanels + 1) * 2;
	corners = new Point3d[nVertices];
	texCoords = new Point2f[nVertices];

	minHeight = -screen_height / 2.0;
	maxHeight = screen_height / 2.0;

	if (angleOffset == 0.0)
	{
		minWidth = -screen_width / 2.0;
		maxWidth = screen_width / 2.0;
	}
	else
	{
		radAngle = angleOffset * DEGTORAD;
		r = (screen_width / numPanels) / (2 * sin(radAngle / 2.0)); //radius of circle to use for curving the panels
		/*
		if (numPanels & 1)
			yOffset = (screen_width / numPanels) / (2 * cos(radAngle / 2.0));  //numPanels is odd
		else
			yOffset = r; //numPanels is even
		*/
	}

	for (i = 0; i < nVertices; i += 2)
	{
		if (angleOffset == 0.0)
		{
			corners[i + 1][X] = corners[i][X] = minWidth + (maxWidth - minWidth) * (i / (nVertices - 2.0));
			corners[i + 1][Y] = corners[i][Y] = 0;
		}
		else
		{
			a = (i - numPanels) * radAngle / 2.0;
			corners[i + 1][X] = corners[i][X] = r * sin(a);
			corners[i + 1][Y] = corners[i][Y] = r * cos(a);
		}
		corners[i][Z] = maxHeight;
		corners[i + 1][Z] = minHeight;

		texCoords[i + 1][X] = texCoords[i][X] = i / (nVertices - 2.0);
		//mdp changed to use GetTexT to respect format being read in 3/5/2009
        if (leftTexture)
        {
		    texCoords[i][Y] = leftTexture->GetTexT(0.0);
		    texCoords[i + 1][Y] = leftTexture->GetTexT(1.0);
        }
        else
        {
		    texCoords[i][Y] = 1.0;
		    texCoords[i + 1][Y] = 0.0;
        }
	}
}

/*---------------------------------------------------------------------------*/
int TextureScreen::writeTo(FILE *file)
{
	float scale = pose->getScale();
	string rfname;

	orient = *pose->getOrientation();

    fprintf(file,"SCREEN_WIDTH %f\n", (float)(screen_width * scale));
    fprintf(file,"SCREEN_HEIGHT %f\n", (float)(screen_height * scale));
    fprintf(file,"YAW %f\n",orient.axis[Yaw]);
    fprintf(file,"PITCH %f\n",orient.axis[Pitch]);
    fprintf(file,"ROLL %f\n",orient.axis[Roll]);
    if(load)
    {
        fprintf(file,"LOAD\n");
    }
	if (numPanels > 1)
		fprintf(file, "PANELS %i\n", numPanels);
	if (angleOffset != 0.0)
		fprintf(file, "PANEL_ANGLE %f\n", angleOffset);
	Object::writeTo(file);
	fprintf(file,"%s\n",leftTexture->GetFilenameAsPath().filename().string().c_str());
	if (rightTexture)
	{
		rfname = rightTexture->GetFilenameAsPath().filename().string();
        fprintf(file,"%s\n",rfname.c_str());
	}
    return 0;
}

void TextureScreen::Idle(double time)
{
	Object::Idle(time);

        if(!dgTextureCoordinates.empty())
        {
            IndexTimeSequence::const_iterator next_step = dgTimes.GetLatest(gz4d::Time(time));
            if(currentDGTimeStep != next_step)
            {
                if(next_step != dgTimes.end() && dgVertexCoordinates[next_step->second].empty())
                {
                    dgNcVar.SetCur(next_step->second,0,0,0);
                    dgVertexCoordinates[next_step->second].Resize(dgWidth,dgHeight);
                    dgNcVar.Get(reinterpret_cast<float*>(dgVertexCoordinates[next_step->second].raw()), 1, dgHeight, dgWidth, 2);
                }
                currentDGTimeStep = next_step;
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
            }
        }

	if(leftTexture && leftTexture->IsPlaying())
	{
            if(leftTexture->TextureIdle(time))
            {
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
            }
            else
                Stop();
    }
    if(rightTexture && rightTexture->IsPlaying())
    {
        if(rightTexture->TextureIdle(time))
			globalInputMediator->Trigger(Scene3D::objectChangedEvent());
		else
			Stop();
    }
}


void TextureScreen::DrawLowRes(float degradation, Window3D *win)
{
    std::shared_ptr<TextureMap> texture;
    int i, nVertices;
    ViewManager* viewInfo = NULL;
//	const GLfloat *pos;

    init();
    if (win)
    {
        viewInfo = win->getViewInfo();
        if (scalePriority)
        {
            float scaleFactor = 1.0;
            gz4d::Point<double> winPos = viewInfo->getEyePoint(); //we should do these things w.r.t the eyepoint!
            gz4d::Point<double> myPos = pose->getPosition();
            double invscale = 1.0 / viewInfo->getPosable()->getScale();

            //find the distance to the eyepoint
            float distanceFromWin, x, y;
            x = myPos[X] - winPos[X];
            y = myPos[Y] - winPos[Y];
            distanceFromWin = sqrt(x*x + y*y);
            if (invscale > 133.3)
                distanceFromWin += invscale - 133.3;

            float pixSize = viewInfo->getPixelSize();
            float blendEdge, threshold = 4000 * pixSize * 133.3 / invscale;
            if (distanceFromWin > threshold) //only do this if far enough away
            {
                //find the difference between the eyepoint's heading and its heading toward us.
                float heading = atan2(myPos[X] - winPos[X], myPos[Y] - winPos[Y]);
                float winHeading = viewInfo->getPosable()->getOrientationAxis()[Yaw];
                heading = fabs(RAD2DEG(heading) - winHeading);
                while (heading >= 180.0)
                    heading -= 360.0;
                heading = fabs(heading);

                scaleFactor = log(1.0 + distanceFromWin + viewInfo->getPosable()->getScale()) / 6.0;
                if (scaleFactor < 1.0)
                    scaleFactor = 1.0;
                if (heading <= 1.0)
                    scaleFactor *= 1.0 + scalePriority / 100.0;
                else if (heading <= 11.0)
                    scaleFactor *= 1.0 + scalePriority / (100.0 + (heading - 1.0) * 10.0);
                else
                    scaleFactor *= 1.0 + scalePriority / 200.0;

                blendEdge = 20000.0 * pixSize;
                if (distanceFromWin < blendEdge)
                    scaleFactor = (distanceFromWin - threshold)*scaleFactor/(blendEdge - threshold);
                //magic to get little change close up (invscale = 130), but a fair rescaling at invscale == 2000...
                if (invscale > 133.3)
                {
                    invscale /= 130.0;
                    scaleFactor *= invscale - (pow(invscale, 1.5) / 15) + 0.045; 
                }
                scaleFactor *= emphasisFactor;
            }

            if (scaleFactor > 1.0)
            {
                glTranslatef(0.0, 0.0, (scaleFactor * screen_height - screen_height)/2.0);
                glScalef(scaleFactor, scaleFactor, scaleFactor);
            }
         }
    }

	doneFlag = 0;
	nVertices = (numPanels + 1) * 2;

	if(!renderingForPicking)
	{
		glDisable(GL_LIGHTING);
		glLineWidth(1.0);
		glColor4f(1.0,1.0,1.0,1.0);

		if(!seeTru)
		{
                    /*if(currentImage)
                    {
                        glEnable(GL_TEXTURE_2D);
                        if(!sequenceTexture)
                            sequenceTexture.Generate(*currentImage);
                        else
                            sequenceTexture.Update(*currentImage);
                        sequenceTexture.Bind();
                    }
                    else*/ 
                        if(leftTexture!=NULL)
			{
				if(rightTexture!=NULL)
				{
					// stereo texture
					if(viewInfo->getEye()==GL_BACK_RIGHT || viewInfo->getEye()==GL_FRONT_RIGHT)
					{
						drewLastTime = rightTexture->Enable();
						texture = rightTexture;
					}
					else
					{
						drewLastTime = leftTexture->Enable();
						texture = leftTexture;
					}
				}
				else
				{

					drewLastTime = leftTexture->Enable();
                                        texture = leftTexture;
				}
			}
			else
			{
				EnableTexture(TEXTURE_COORD_NONE);
				texture = GetCurrentTexture();
			}

			if(!twoFaced)
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			}

			if (!dgIndecies.empty())
			{
				// Draw the distortion grid.
				drawDistortionGrid(texture);
			}
			else
			{
                if (childrenDrawn) //added by mdp 4/23/2009 for use in QuadTreeCollections
                {
                    int child, end;
                    for (child = 0; child < 4; child++)
                    {
                        if (childrenDrawn[child])
                            continue;
                        //otherwise, draw a square!
                        if (drewLastTime)
                        {
				            glBegin(GL_QUAD_STRIP);
					            glNormal3f(1.0, 1.0, 1.0);
                                end = quadCornerIndex[child] + nVertices;
					            for (i = quadCornerIndex[child]; i < end; i++)
					            {
                                    glTexCoord2d(texCoords[i][X],texCoords[i][Y]);
						            glVertex3f(corners[i][X],corners[i][Y],corners[i][Z]);
					            }
				            glEnd();
                        }
                    }
                }
                else if (drewLastTime)
                {
				    glBegin(GL_QUAD_STRIP);
					    glNormal3f(1.0, 1.0, 1.0);
					    for (i = 0; i < nVertices; i++)
					    {
						    //glTexCoord2d(texture->GetTexS(texCoords[i][X]),texture->GetTexT(texCoords[i][Y]));
                            glTexCoord2d(texCoords[i][X],texCoords[i][Y]);
						    glVertex3f(corners[i][X],corners[i][Y],corners[i][Z]);
					    }
				    glEnd();
                }
			}

			glDisable(GL_CULL_FACE);

/*                        if(currentImage)
                        {
                            sequenceTexture.Unbind();
                            glDisable(GL_TEXTURE_2D);

                        }*/
                        if(leftTexture!=NULL)
			{
				leftTexture->Disable();
			}
			else
			{
				DisableTexture();
			}
		}

		glEnable(GL_LIGHTING);        

		if (showBorder)
		{
			glColor3dv(borderColor);
			glLineWidth(borderWidth);
            glNormal3f(0.0, 0.0, 1.0);

			if (!dgIndecies.empty())
			{
				// Draw the distortion grid outline.
				drawDistortionGridOutline();
			}
			else
			{
				glBegin(GL_LINE_LOOP);
				for (i = 0; i < nVertices; i += 2)
				{
					glVertex3f(corners[i][X],corners[i][Y],corners[i][Z]);
				}
				for (i = nVertices - 1; i > 0; i -= 2)
				{
					glVertex3f(corners[i][X],corners[i][Y],corners[i][Z]);
				}
				glEnd();
			}
		}
		
		glLineWidth(1.0);
	}
	else  // we are rendering for picking
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		if (!dgIndecies.empty())
		{
			// Draw the distortion grid.
			drawDistortionGrid(texture);
		}
		else
		{
			glBegin(GL_QUAD_STRIP);
			for (i = 0; i <= nVertices; i++)
			{
				glVertex3f(corners[i][X],corners[i][Y],corners[i][Z]);
			}
			glEnd();
		}

		glDisable(GL_CULL_FACE);
	}    
}


/*---------------------------------------------------------------------------*/
int TextureScreen::pickAction (Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
    if(pickType == PICK_CLICK1 && MainWindow::getInterfaceType() != GeoCoastPilot)
    {
        doubleClickInteractor->reset();
        if(leftTexture!=NULL)
        {
            if(!leftTexture->isLoaded())
            {
                leftTexture->Load();
                seeTru = 0;
            }
            else
            {
                if(seeTru)
                {
                    seeTru = 0;
                }
                else
                {
                    seeTru = 1;
                }
            }
            if(rightTexture!=NULL)
            {
                if(!rightTexture->isLoaded())
                    rightTexture->Load();
            }
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
        }
    }
    return Object::pickAction(pickType, inputInfo, names, hits);
}

/*---------------------------------------------------------------------------*/
int TextureScreen::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	const char *message = argv[0];
	char retval[64];

	if (argc >= 1)
	{
		if (!strcmp(message, "play"))
		{
			Play();
			return TCL_OK;
		}
		if (!strcmp(message, "stop"))
		{
			Stop();
			return TCL_OK;
		}
		if (!strcmp(message, "pause"))
		{
			Pause();
			return TCL_OK;
		}
		if (!strcmp(message, "loop"))
		{
			if (argc >= 2)
				Loop(atoi(argv[1]));
			else
				Loop();
			return TCL_OK;
		}
		if (!strcmp(message, "showborder"))
		{
			if (argc >= 2)
				showBorder = atoi(argv[1]);
			sprintf(retval, "%i", showBorder);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if (!strcmp(message, "fusionTransparency"))
		{
			if (argc >= 2)
				fusionTransparency = (float) atof(argv[1]);
			sprintf(retval, "%f", fusionTransparency);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if (!strcmp(message, "refresh"))
		{
			if(leftTexture)
				leftTexture->Reload();
			if(rightTexture)
				rightTexture->Reload();
			return TCL_OK;
		}
		if (!strcmp(message, "screenWidth"))
		{
			if (argc >= 2)
				screen_width = (float) atof(argv[1]);
			sprintf(retval, "%f", screen_width);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if (!strcmp(message, "screenHeight"))
		{
			if (argc >= 2)
				screen_height = (float) atof(argv[1]);
			sprintf(retval, "%f", screen_height);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if (!strcmp(message, "seeThrough"))
		{
			if (argc >= 2)
				seeTru = atoi(argv[1]);
			sprintf(retval, "%i", seeTru);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if (!strcmp(message, "borderColor"))
		{
			if (argc >= 4)
            {
				borderColor[0] = atof(argv[1]);
				borderColor[1] = atof(argv[2]);
				borderColor[2] = atof(argv[3]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
			sprintf(retval, "%f %f %f", borderColor[0], borderColor[1], borderColor[2]);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if (!strcmp(message, "borderWidth"))
		{
			if (argc >= 2)
            {
				borderWidth = atoi(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
			sprintf(retval, "%f", borderWidth);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
        if (!strcmp(message, "emphasisFactor"))
        {
            if (argc >= 2)
            {
                emphasisFactor = atof(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            sprintf(retval, "%f", emphasisFactor);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "ignoreHeightExaggeration"))
        {
            dynamic_cast<ObjectPoseState*>(pose)->ignoreHeightExaggeration();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
        }
	return Object::perform(argc, argv, interp);

}

/*---------------------------------------------------------------------------*/
int TextureScreen::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "play stop pause loop showborder fusionTransparency refresh screenWidth screenHeight seeThrough borderColor borderWidth emphasisFactor ", NULL);
	return Object::list(interp);
}

void TextureScreen::Play()
{
    if(leftTexture && leftTexture->isLoaded())
    {
        leftTexture->Play();
        if(rightTexture && rightTexture->isLoaded())
        {
            rightTexture->Play();
        }
    }
    else
    {
        if (!customtextureptr)
        {
            (*defaulttextureptr)->Play();
        }
        else
        {
            customtextureptr->Play();
        }
    }
    highLevelEventMediator->Trigger(textureScreenInputType->value, this, (void *)"play");
}

void TextureScreen::Stop()
{
    if(leftTexture && leftTexture->isLoaded())
    {
        leftTexture->Stop();
        if(rightTexture && rightTexture->isLoaded())
        {
            rightTexture->Stop();
        }
    }
    else
    {
        if (!customtextureptr)
        {
            (*defaulttextureptr)->Stop();
        }
        else
        {
            customtextureptr->Stop();
        }
    }
    highLevelEventMediator->Trigger(textureScreenInputType->value, this, (void *)"stop");
}

void TextureScreen::Pause()
{
    if(leftTexture && leftTexture->isLoaded())
    {
        leftTexture->Pause();
        if(rightTexture && rightTexture->isLoaded())
        {
            rightTexture->Pause();
        }
    }
    else
    {
        if (!customtextureptr)
        {
            (*defaulttextureptr)->Pause();
        }
        else
        {
            customtextureptr->Pause();
        }
    }
    highLevelEventMediator->Trigger(textureScreenInputType->value, this, (void *)"pause");
}

void TextureScreen::Loop(GLboolean repeat)
{
    if(leftTexture && leftTexture->isLoaded())
    {
        leftTexture->Loop(repeat);
        if(rightTexture && rightTexture->isLoaded())
        {
            rightTexture->Loop(repeat);
        }
    }
    else
    {
        if (!customtextureptr)
        {
            (*defaulttextureptr)->Loop(repeat);
        }
        else
        {
            customtextureptr->Loop(repeat);
        }
    }
}

void TextureScreen::SaveXML(FILE * out, const char * path)
{
	fprintf(out,"<Object type=\"TextureScreen\" ");
	SaveXMLattributes(out);
	fprintf(out,"texturefile=\"%s\" ",(path/leftTextureName).string().c_str());
	fprintf(out,"local_origin=\"%lf %lf %lf\" ",GetPosition(0),GetPosition(1),GetPosition(2));
	fprintf(out,"screen_width=\"%f\" screen_height=\"%f\" ",screen_width,screen_height);
	fprintf(out,"yaw=\"%f\" pitch=\"%f\" roll=\"%f\" ",pose->getOrientation()->axis[Yaw],pose->getOrientation()->axis[Pitch],pose->getOrientation()->axis[Roll]);
	fprintf(out,"load=\"%i\" ",load);
	fprintf(out,"/>\n");
}

Object * TextureScreen::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    //Filename * fname = NULL;
    double position[3] = {0.0, 0.0, 0.0}; // georefrence
    bool pbuff;

    Object::LoadXML(xml, path, ncfile);

    bfs::path fspath(path);

    // read values from XML
    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.texturefile"))
        leftTextureName = fspath/(*attribute);
    if(attribute = xml.get_optional<std::string>("<xmlattr>.distortion_grid_file"))
        dgFilename = fspath/(*attribute);
    if(attribute = xml.get_optional<std::string>("<xmlattr>.local_origin"))
    {
        std::stringstream ss(*attribute);
        ss >> position[0];
        ss >> position[1];
        ss >> position[2];
    }

    screen_width = xml.get("<xmlattr>.screen_width",screen_width);
    screen_height = xml.get("<xmlattr>.screen_height",screen_height);

    orient.axis[Yaw] = xml.get("<xmlattr>.yaw",orient.axis[Yaw]);
    orient.axis[Pitch] = xml.get("<xmlattr>.pitch",orient.axis[Pitch]);
    orient.axis[Roll] = xml.get("<xmlattr>.roll",orient.axis[Roll]);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.load"))
        load = gz4d::IsTrue(*attribute);
    if(attribute = xml.get_optional<std::string>("<xmlattr>.twoFaced"))
        twoFaced = gz4d::IsTrue(*attribute);
    if(attribute = xml.get_optional<std::string>("<xmlattr>.border"))
        showBorder = gz4d::IsTrue(*attribute);

    numPanels = xml.get("<xmlattr>.panels",numPanels);
    angleOffset = xml.get("<xmlattr>.panel_angle",angleOffset);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.pbuffer"))
        pbuff = gz4d::IsTrue(*attribute);

    numNybblesPerColor = xml.get("<xmlattr>.colorNybbles",numNybblesPerColor);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.allowTransparency"))
        useTransparency = gz4d::IsTrue(*attribute);

    return this;
}

void TextureScreen::actAsTemplate()
{
    QuadTreeObject::actAsTemplate();
    init();
}

void TextureScreen::init()
{
    if(!initialized)
    {
        if (!leftTexture)
        {
            leftTexture = mainWindow->getScene3D()->GetRTVideo();
        }

        if(!leftTextureName.empty() && readWorldFile(leftTexture, leftTextureName))
        {
            pose->setOrientation(orient,ChangeRotate);
            loadLeftTexture();
        }

    // Call the distortion grid initialization code.
        initDistortionGrid();
    }
    initialized = true;
}

/*---------------------------------------------------------------------------*/
bool TextureScreen::loadLeftTexture()
{
    int nVertices;
    double minHeight, maxHeight, minWidth, maxWidth;
    int i;

    if(load)
    {
        leftTexture->Load();
    }

    //define corners of screen
    nVertices = (numPanels + 1) * 2;
    if (quadtreeStatus != Quadtree_Not)
    {
        corners = new Point3d[16];
        texCoords = new Point2f[16];
    }
    else
    {
        corners = new Point3d[nVertices];
        texCoords = new Point2f[nVertices];
    }
    if(!gz4d::IsNan(eastScale))
    {
        minWidth = maxWidth = 0;
        if(eastScale > 0)
            maxWidth = screen_width;
        else
            minWidth = -screen_width;

        corners[0][X] = corners[1][X] = minWidth;
        corners[2][X] = corners[3][X] = maxWidth;

        minHeight = maxHeight = 0;
        if(northScale > 0)
            maxHeight = screen_height;
        else
            minHeight = -screen_height;

        corners[2][Y] = corners[0][Y] = maxHeight;
        corners[3][Y] = corners[1][Y] = minHeight;

        corners[0][Z] = corners[1][Z] = corners[2][Z] = corners[3][Z] = 0;

        bbox.w = pose->getPosition()[0]+minWidth;
        bbox.e = pose->getPosition()[0]+maxWidth;
        bbox.n = pose->getPosition()[1]+maxHeight;
        bbox.s = pose->getPosition()[1]+minHeight;
        bbox.t = bbox.b = 0.0;

        for (i = 0; i < nVertices; i += 2)
        {
            texCoords[i + 1][X] = texCoords[i][X] = i / (nVertices - 2.0);
		    //mdp changed to use GetTexT to respect format being read in 3/5/2009
            if (leftTexture)
            {
		        texCoords[i][Y] = leftTexture->GetTexT(0.0);
		        texCoords[i + 1][Y] = leftTexture->GetTexT(1.0);
            }
        }

        if (quadtreeStatus != Quadtree_Not) //figure out the rest of the points
        {
            double centerX, centerY;
            double ctX, ctY;
            centerX = minWidth + childWidth;
            if (centerX > maxWidth) //happens in some border cases
                centerX = maxWidth;
            corners[4][X] = corners[5][X] = corners[10][X] = corners[11][X] = minWidth;
            corners[6][X] = corners[7][X] = corners[12][X] = corners[13][X] = centerX;
            corners[8][X] = corners[9][X] = corners[14][X] = corners[15][X] = maxWidth;

            centerY = maxHeight - childHeight;
            if (centerY < minHeight) //happens in some border cases
                centerY = minHeight;
            corners[4][Y] = corners[6][Y] = corners[8][Y] = maxHeight;
            corners[5][Y] = corners[7][Y] = corners[9][Y] = centerY;
            corners[10][Y] = corners[12][Y] = corners[14][Y] = centerY;
            corners[11][Y] = corners[13][Y] = corners[15][Y] = minHeight;

            corners[4][Z] = corners[5][Z] = corners[6][Z] = corners[7][Z] = 0;
            corners[8][Z] = corners[9][Z] = corners[10][Z] = corners[11][Z] = 0;
            corners[12][Z] = corners[13][Z] = corners[14][Z] = corners[15][Z] = 0;

            ctX = childWidth / screen_width;
            if (ctX > 1.0f)
                ctX = 1.0f;
            texCoords[4][X] = texCoords[5][X] = texCoords[10][X] = texCoords[11][X] = 0.0f;
            texCoords[6][X] = texCoords[7][X] = texCoords[12][X] = texCoords[13][X] = ctX;
            texCoords[8][X] = texCoords[9][X] = texCoords[14][X] = texCoords[15][X] = 1.0f;

            ctY = childHeight / screen_height;
            if (ctY > 1.0f)
                ctY = 1.0f;
            if (texCoords[0][Y] != 0.0)
                ctY = 1.0 - ctY;
            texCoords[4][Y] = texCoords[6][Y] = texCoords[8][Y] = texCoords[0][Y];
            texCoords[5][Y] = texCoords[7][Y] = texCoords[9][Y] = ctY;
            texCoords[10][Y] = texCoords[12][Y] = texCoords[14][Y] = ctY;
            texCoords[11][Y] = texCoords[13][Y] = texCoords[15][Y] = texCoords[1][Y];
        }
    }
    else
        generatePanelCoordinates();
    return true;
}

/*---------------------------------------------------------------------------*/
QuadTreeObject *TextureScreen::offsetCopy(float xOffset, float yOffset, float scaleFactor)
{
    TextureScreen *newguy = new TextureScreen;
    copyObjectComponents(newguy, xOffset, yOffset);
    gz4d::Point<double> myGeoRef = pose->getPosition();
    newguy->SetPosition(myGeoRef[X] + xOffset, myGeoRef[Y] + yOffset, myGeoRef[Z]);

    newguy->eastScale = eastScale * scaleFactor;
    newguy->northScale = northScale * scaleFactor;
    newguy->childWidth = screen_width * scaleFactor / 2.0;
    newguy->childHeight = screen_height * scaleFactor / 2.0;
    newguy->screen_width = screen_width * scaleFactor;
    newguy->screen_height = screen_height * scaleFactor;
    newguy->pixelSize = pixelSize * scaleFactor;
    newguy->numPanels = numPanels;
    newguy->angleOffset = angleOffset;
    newguy->numNybblesPerColor = numNybblesPerColor;
    newguy->useTransparency = useTransparency;
    newguy->load = load;
    //newguy->screen_width = screen_width;
    //newguy->screen_height = screen_height;
    newguy->orient = orient;
    newguy->quality = quality;
    newguy->seeTru = seeTru;
    COPY_POINT(newguy->borderColor, borderColor);
    newguy->borderWidth = borderWidth;
    COPY_POINT(newguy->bgColor, bgColor);
    newguy->fusionTransparency = fusionTransparency;
    newguy->fuseable = fuseable;
    newguy->showBorder = showBorder;
    newguy->twoFaced = twoFaced;

    newguy->generatePanelCoordinates();
    return newguy;
}

/*---------------------------------------------------------------------------*/
bool TextureScreen::setXMLattribute(std::string const &name, std::string const &value, std::string const &path)
{
    if (name == "texturefile")
    {
        if(value[0] == '/')
            leftTextureName = value;
        else
            leftTextureName = bfs::path(path)/value;
        return true;
    }
    return QuadTreeObject::setXMLattribute(name, value, path);
}

/*---------------------------------------------------------------------------*/
bool TextureScreen::readContents()
{
    ImageMap *lt;

    if (setTextureFile(leftTextureName, leftTexture))
    {
        if (quadtreeStatus == Quadtree_Not ||
            (quadtreeStatus & Quadtree_Template) ||
            (quadtreeStatus & Quadtree_Potential_Edge) ||
            (quadtreeStatus & Quadtree_Root))
        {
            screen_width = 0.0;
            screen_height = 0.0;
            adjustSize(leftTexture);
            if (quadtreeStatus != Quadtree_Not)
            {
                DYNAMIC_CAST(lt, ImageMap *, leftTexture.get());
                if (quadtreeStatus & Quadtree_Root)
                {
                    if (lt)
                        lt->allowCacheing(false);
                    load = true;
                }
                else
                {
                    if (lt)
                        lt->boostCachePriority(9);
                }
            }
        }
        else
        {
            leftTexture->SetDims((screen_width / eastScale) + 0.5f, fabs(screen_height / northScale) + 0.5f);
            DYNAMIC_CAST(lt, ImageMap *, leftTexture.get());
            if (lt)
            {
                lt->allowCacheing();
                lt->boostCachePriority(10);
            }
        }
        initialized = true;
        return loadLeftTexture();
    }
    return false;
}

/*---------------------------------------------------------------------------*/
bool TextureScreen::combineChildrenOfTile()
{
    std::shared_ptr<QuadTreeTile> tile(myTile);
    std::weak_ptr<QuadTreeTile> w_childTile;
    std::shared_ptr<QuadTreeTile> childTile;
    ImageMap *childTextures[4] = {NULL};
    ImageMap *childImage;
    TextureScreen *childScreen;
    int i, p = 0, wide = 2, tall = 2;

    for (i = QuadTreeTile::FIRST_CHILD; i <= QuadTreeTile::LAST_CHILD; i++)
    {
        w_childTile = tile->getChild(QuadTreeTile::ChildDirection(i));
        if (w_childTile.expired())
        {
            switch (i)
            {
                case QuadTreeTile::SOUTHEAST:
                    if (wide == 1)
                        break;
                    //else fall through
                case QuadTreeTile::NORTHWEST:
                    cerr << "Non-rectangular shape--cannot combine" << endl;
                    return false; //invalid ones not to have!
                case QuadTreeTile::NORTHEAST:
                    wide = 1;
                    continue;
                case QuadTreeTile::SOUTHWEST:
                    tall = 1;
                    break;
            }
            if (tall == 1 || (wide == 1 && i == QuadTreeTile::SOUTHEAST)) //don't go on to try the last corner.
                break;
            continue; //skip any assignments, we already have a NULL in place.
        }
        childTile = std::shared_ptr<QuadTreeTile>(w_childTile);
        DYNAMIC_CAST(childScreen, TextureScreen *, childTile->getObject());
        if (!childScreen->leftTexture)
            childScreen->combineChildrenOfTile();
        DYNAMIC_CAST(childImage, ImageMap *, childScreen->leftTexture.get());
        childTextures[p++] = childImage;
    }

    //create the texture we want!
    std::shared_ptr<ImageMap> im;
    gz4d::Point<double> myGeoRef = pose->getPosition();
    bool oldLoad = load;

    leftTexture = im = std::shared_ptr<ImageMap>(new ImageMap);
    //leftTexture->SetFilename("something");
    generatedInternally = true;
    im->UseMipmaps(false);
    im->setColorFormat(convertFormat());
    im->CombineTextures(childTextures, wide, tall);
    im->SetRef(myGeoRef[X], myGeoRef[Y], eastScale, northScale);
    screen_width = 0.0;
    screen_height = 0.0;
    adjustSize(leftTexture);
    load = false;
    initialized = true;
    loadLeftTexture(); //put the image in the right place
    load = oldLoad;

    return true;
}

void TextureScreen::Fuse(Object * other)
{
	Gutm * Other;
	
	if(fuseable)
	{
    if(!other || other->getRegistrationEntry()->className != "Gutm")
    	return;
    Other = ((Gutm *)other);
    if(GetBoundingBox().overlapps(Other->GetBoundingBox()))
    {
    	if(leftTexture)
     	{
      		leftTexture->Fuse(Other, fusionTransparency);
			if (fusionTransparency > 0.0 && transparencyLevel <= 0)
				transparencyLevel = 1;
      }
    }
	}
}

void TextureScreen::setLocationOffset(gz4d::Point<double> const &pos)
{
    gz4d::Point<double> newPosition = pos;
    if (parent)
    {
        gz4d::Point<double> oldPosition = pose->getPosition();
        if (oldPosition.IsValid())
            newPosition += oldPosition;
    }
    pose->setPosition(newPosition[X], newPosition[Y], newPosition[Z]);
}

/**
 * This method initializes the distortion grid variables.  If the
 * distortion grid filename is not null, then the first distortion grid
 * in the file is loaded.
 */
void TextureScreen::initDistortionGrid()
{
    if (!dgFilename.empty())
    {
        if (extension(dgFilename) == ".nc")
        {
            // Create and check the netCDF file object.
            dgNcFile = gz4d::netcdf::File(dgFilename.string());
            if(dgNcFile)
            {
                gz4d::netcdf::Dim rowd = dgNcFile.GetDim("row");
                gz4d::netcdf::Dim cold = dgNcFile.GetDim("column");

                if(rowd && cold)
                {
                    dgWidth = cold.Size();
                    dgHeight = rowd.Size();
                    dgTextureCoordinates.Resize(dgWidth,dgHeight);

                    // calculate the texture coordinates
                    for(int r=0; r<dgHeight; ++r)
                    {
                        float t = r/(dgHeight-1.0);
                        TexCoordType* row = dgTextureCoordinates.GetRow(r);
                        for(int c=0; c<dgWidth; ++c)
                            row[c]=TexCoordType(c/(dgWidth-1.0),t);
                    }

                    // calculate the indecies
                    dgIndecies.resize(dgHeight-1);
                    dgIndexPointers.resize(dgHeight-1);
                    dgElementCounts.resize(dgHeight-1);
                    for(int r=0; r<dgHeight-1; ++r)
                    {
                        for(int c=0; c<dgWidth; ++c)
                        {
                            dgIndecies[r].push_back(r*dgWidth+c);
                            dgIndecies[r].push_back((r+1)*dgWidth+c);
                        }
                        dgIndexPointers[r] = &dgIndecies[r].front();
                        dgElementCounts[r] = dgIndecies[r].size();
                    }

                    for(int r = 0; r < dgHeight; ++r)
                        dgOutlineIndecies.push_back(r*dgWidth);
                    for(int c = 0; c < dgWidth; ++c)
                        dgOutlineIndecies.push_back((dgHeight-1)*dgWidth+c);
                    for(int r = dgHeight-1; r >= 0; --r)
                        dgOutlineIndecies.push_back(r*dgWidth+dgWidth-1);
                    for(int c = dgWidth-1; c >= 0; --c)
                        dgOutlineIndecies.push_back(c);


                    // create empty time index.
                    gz4d::netcdf::Dim timed = dgNcFile.GetDim("time");
                    gz4d::netcdf::Var timevar = dgNcFile.GetVar("time");
                    if (timed && timevar)
                    {
                        std::vector<double> times(timed.Size());
                        dgVertexCoordinates.resize(timed.Size());
                        timevar.SetCur(0);
                        timevar.Get(times);
                        for(std::size_t t = 0; t < times.size(); ++t)
                            dgTimes[gz4d::Time(times[t])]=t;
                    }
                    currentDGTimeStep = dgTimes.end();
                    dgNcVar = dgNcFile.GetVar("grids");
                }
                else
                {
                    cerr << "Error finding row or column dimension in netcdf file.\n";
                    dgNcFile.Close();
                }
            }
            else
            {
                cerr << "Error opening netcdf file\n";
                dgNcFile.Close();
            }
        }
    }
}

/**
 * Draws the distortion grid.
 */
void TextureScreen::drawDistortionGrid(std::shared_ptr<TextureMap> &texture)
{
    if(currentDGTimeStep != dgTimes.end())
    {
        gz4d::glo::PushClientAttrib cattribs;
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2,GL_FLOAT,0,dgVertexCoordinates[currentDGTimeStep->second].raw());
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2,GL_FLOAT,0,dgTextureCoordinates.raw());
        glMultiDrawElements(GL_QUAD_STRIP,&dgElementCounts.front(),GL_UNSIGNED_INT,&dgIndexPointers.front(),dgIndexPointers.size());
    }

}

/**
 * This method draws the outline of the distortion grid.
 */
void TextureScreen::drawDistortionGridOutline()
{
    if(currentDGTimeStep != dgTimes.end())
    {
        gz4d::glo::PushClientAttrib cattribs;
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2,GL_FLOAT,0,dgVertexCoordinates[currentDGTimeStep->second].raw());
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2,GL_FLOAT,0,dgTextureCoordinates.raw());
        glDrawElements(GL_LINE_LOOP,dgOutlineIndecies.size(),GL_UNSIGNED_INT,&dgOutlineIndecies.front()); 
    }
}

void TextureScreen::setScale(float newScale)
{
    pose->setScale(newScale);
}

void TextureScreen::applyChanges()
{
    pose->setOrientation(orient);
    generatePanelCoordinates();
}

unsigned int TextureScreen::textureScreenEvent()
{
    return textureScreenInputType->value;
}

// void TextureScreen::ImageStream::UseSocket(gz4d::Socket const &s)
// {
//     socket = s;
// }
// 
// void TextureScreen::ImageStream::Main()
// {
//     std::string data;
//     data.resize(4096);
//     while(true)
//     {
//         socket.Recv(data);
//         if (data[8] == 'I')
//         {
//             gz4d::Time t = ((double*)data.data())[0];
//             unsigned short r = *(unsigned short *)&(data.data()[10]);
//             unsigned short rows = *(unsigned short *)&(data.data()[12]);
//             unsigned short cols = *(unsigned short *)&(data.data()[14]);
//             if (t != time)
//             {
//                 if (image)
//                 {
//                     Lock();
//                     push(gz4d::ImageSequence::value_type(time,image));
//                     UnLock();
//                 }
//                 time = t;
//                 image = std::shared_ptr<gz4d::Image>(new gz4d::Image);
//                 image->Resize(cols,rows);
//             }
//             memcpy(image->GetRow(r),&data[16],cols*4);
//         }
//     }
// }
