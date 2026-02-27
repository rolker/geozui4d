#include"S57_bridge.h"
#include"S57.h"
#include "engine/Scene3D.h"
#include<iostream>
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "engine/PosableStateBase.h"

using namespace std;

REGISTER_OBJECT(S57_bridge,"");

const char *S57_bridge::bridgeCategoryString[] = {"Unknown", "Fixed", "Opening",
												  "Swing", "Lifting", "Bascule",
												  "Pontoon", "Draw bridge",
												  "Transporter", "Foot bridge",
												  "Viaduct", "Aqueduct",
												  "Suspension"
												 };

/*---------------------------------------------------------------------------*/
S57_bridge::S57_bridge()
{
	category = Unknown;
	verticalClearanceClosed = verticalClearanceOpen = 0.0;
}

/*---------------------------------------------------------------------------*/
S57_bridge::~S57_bridge()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void S57_bridge::DrawLowRes(float deg,Window3D * win)
{
    if (win)
    {
        double scale = win->getViewInfo()->getPosable()->getScale();
        if (scale <= scamin) //use = because we already nudged scamax & scamin for roundoff
            return;
    }

	if(verticalClearanceClosed > 0.0)
	{
		//draw a bridge shadow...
		useColor = GL_FALSE;
		height = 0.0;
        glColor3f(S57::dimFactor()*0.5f, S57::dimFactor()*0.6f, S57::dimFactor()*0.7f);
		S57_object::DrawLowRes(deg,win);
		useColor = GL_TRUE;

		glPushMatrix();
		glTranslatef(0.0, 0.0, verticalClearanceClosed);
		lineWidth = 2.0;
		S57_object::DrawLowRes(deg,win);
		lineWidth = 1.0;
		if (verticalClearanceOpen != verticalClearanceClosed)
		{
			useColor = GL_FALSE;
            glColor3f(S57::dimFactor()*0.8f, S57::dimFactor()*0.7f, S57::dimFactor()*0.5f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			height = verticalClearanceOpen - verticalClearanceClosed;
			S57_object::DrawLowRes(deg,win);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			useColor = GL_TRUE;
		}
		glPopMatrix();
		height = 0.0;
	}
	else
		S57_object::DrawLowRes(deg,win);
}

/*---------------------------------------------------------------------------*/
void S57_bridge::SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber)
{
	S57_object::SetAttributes(item, info, objectNumber);

        if(item["CATBRG"])
        {
            category = item["CATBRG"].GetValue<int>();
            if (category >= Opening && category <= Bascule)
            {
                if(item["VERCCL"])
                    verticalClearanceClosed = (float) item["VERCCL"].GetValue<double>();
                if(item["VERCOP"])
                    verticalClearanceOpen = (float) item["VERCOP"].GetValue<double>();
            }
            else
            {
                if(item["VERCLR"])
                    verticalClearanceClosed = verticalClearanceOpen = (float) item["VERCLR"].GetValue<double>();
            }
        }
}

/*---------------------------------------------------------------------------*/
void S57_bridge::ShowS57Info(Scene3D *scene)
{
	char infoLine[512];

	sprintf(infoLine, "Category:\t%s\n", bridgeCategoryString[category]);
	scene->appendInfo(infoLine);
	if (category >= Opening && category <= Bascule)
		sprintf(infoLine, "Clearance:\t%.2fm(closed) - %.2fm(open)\n", verticalClearanceClosed, verticalClearanceOpen);
	else
		sprintf(infoLine, "Clearance:\t%.2fm\n", verticalClearanceClosed);
	scene->appendInfo(infoLine);

}
