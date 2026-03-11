// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "GutmTree.h"
#include "objects/gutm.h"

#include"objects/nodeTypes/XYZPoint.h"
#include"objects/nodeTypes/XYZPointAvg.h"
#include"objects/nodeTypes/XYZPointWA.h"
#include"objects/nodeTypes/XYZPointMin.h"

#include "GZincludes.h"
//#include<iostream>
//#include"MainWindow.h"
//#include"engine/Scene3D.h"

#define GUTM_ZREF -5000

using namespace std;

extern MainWindow * mainWindow;


GutmTree::GutmTree(bool altData)
{
    gutms = NULL;
    Gutm::useHighResZone = HYBRID_HIGHRES;
    hasAltData = altData;
}

GutmTree::~GutmTree()
{
}

void GutmTree::Initialize(string pt, double resolution)
{
    ptype = pt;
    res = resolution;
}

void GutmTree::AddPoint(Point3d p, double radius, double t, float alt)
{
    double x,y;
    int i,j;
    
    Point3d gr;
    Gutm * gutm;

    if(!gutms)
    {
        gr[0] = p[0] - GUTM_SIZE*res/2.0;
        gr[1] = p[1] - GUTM_SIZE*res/2.0;
        gr[2] = GUTM_ZREF;
        SetPosition(gr);
    }
    for(x=p[0]-radius; x <= p[0]+radius;x+=res)
        for(y=p[1]-radius; y<= p[1]+radius;y+=res)
        {
            if(!GetGutm(x,y))
                AddGutm(x,y);
        
            gutm = GetGutm(x,y);
            while(gutm)
            {
                i = (int)floor((x-gutm->GetPosition(0))/res);
                j = (int)floor((y-gutm->GetPosition(1))/res);

                if(i>100||j>100)
                    break;
            
                if(ptype == "s")
                {
                    XYZPoint s;
                    COPY_POINT(s.sounding,p);
                    if (hasAltData)
                        s.alternateData = alt;
                    if(!gutm->GetHandler(i,j))
                    {
                        gutm->SetHandler(s.NewHandler(gutm->GetPosition(0)+(i*res),gutm->GetPosition(1)+(j*res)),i,j);
                    }
                    gutm->GetHandler(i,j)->Add(s,t);
                }
                else if(ptype == "sa")
                {
                    XYZPointAvg s;
                    COPY_POINT(s.sounding,p);
                    if (hasAltData)
                        s.alternateData = alt;
                    if(!gutm->GetHandler(i,j))
                    {
                        gutm->SetHandler(s.NewHandler(gutm->GetPosition(0)+(i*res),gutm->GetPosition(1)+(j*res)),i,j);
                    }
                    gutm->GetHandler(i,j)->Add(s,t);
                    //gutm->SetValue(i,j,gutm->GetHandler(i,j)->GetAverage());
                }
                else if(ptype == "min")
                {
                    XYZPointMin s;
                    COPY_POINT(s.sounding,p);
                    if (hasAltData)
                        s.alternateData = alt;
                    if(!gutm->GetHandler(i,j))
                    {
                        gutm->SetHandler(s.NewHandler(gutm->GetPosition(0)+(i*res),gutm->GetPosition(1)+(j*res)),i,j);
                    }
                    gutm->GetHandler(i,j)->Add(s,t);
                    //gutm->SetValue(i,j,gutm->GetHandler(i,j)->GetAverage());
                }
                else if(ptype == "swa")
                {
                    XYZPointWA s;
                    COPY_POINT(s.sounding,p);
                    if (hasAltData)
                        s.alternateData = alt;
                    s.radius = radius;
                    if(!gutm->GetHandler(i,j))
                    {
                        gutm->SetHandler(s.NewHandler(gutm->GetPosition(0)+(i*res),gutm->GetPosition(1)+(j*res)),i,j);
                    }
                    gutm->GetHandler(i,j)->Add(s,t);
                    //gutm->SetValue(i,j,gutm->GetHandler(i,j)->GetAverage());
                }
                gutm = GetGutm(x,y,0);
            }
        }
        expandBBoxTime(t);
}

Gutm * GutmTree::GetGutm(double x, double y, int reset)
{
    Gutm * ret;
    
    if(reset)
        gcursor = gutms;
    
    while(gcursor)
    {
        if(gcursor->gutm->GetBoundingBox().isInX(x) && gcursor->gutm->GetBoundingBox().isInY(y))
        {
            ret = gcursor->gutm;
            gcursor = gcursor->next;
            return ret;
        }
        gcursor = gcursor->next;
    }
    return NULL;
}

void GutmTree::SaveObject(int id, char * fn)
{
    FILE * file;
    int cols,rows;
    BBox sb;
    int i,j;
    Gutm * g;
    double zref;
    double val;
    
    if(id == this->id)
    {
        file = fopen(fn,"w");
        if(!file)
        {
            cerr << "Error opening for writing: " << fn << endl;
            return;
        }
        fprintf(file,"BEGIN GUTM 1.0\n");
        sb = GetBoundingBox();
        cols = (int)((bbox.e-bbox.w)/res);
        rows = (int)((bbox.n-bbox.s)/res);
        fprintf(file,"COLS_ROWS %i %i\n",cols,rows);
        fprintf(file,"CELLSIZE %f\n",res);
        fprintf(file,"SHAPE FULLGRID\n");
        zref = 0.0;
        for(i=0;i<rows;i++) // y
            for(j=0;j<cols;j++) // x
            {
                g = GetGutm(sb.w+j*res,sb.s+i*res);
                if(g)
                    if(g->GetValue(sb.w+j*res,sb.s+i*res) && g->GetValue(sb.w+j*res,sb.s+i*res)<zref)
                        zref = g->GetValue(sb.w+j*res,sb.s+i*res);
            }
        zref -= 1.0;
        fprintf(file,"LOCAL_ORIGIN %f %f %f\n",sb.w,sb.s,zref);
        for(i=0;i<rows;i++) // y
        {
            for(j=0;j<cols;j++) // x
            {
                val = 0.0;
                g = GetGutm(sb.w+j*res,sb.s+i*res);
                while(g)
                {
                    if(g->GetValue(sb.w+j*res,sb.s+i*res))
                        val = g->GetValue(sb.w+j*res,sb.s+i*res)-zref;
                    g = GetGutm(sb.w+j*res,sb.s+i*res,0);
                }
                fprintf(file,"%f ",val);
            }
            fprintf(file,"\n");
        }
        fclose(file);
    }
    else
        ObjectCollection::SaveObject(id,fn);
}

void GutmTree::AddGutm(double x, double y)
{
    Point3d gr;
    NodeType * tmp;
    char l[100];
    Gutm * gutm;
    
    tmp = new NodeType;
    tmp->next = gutms;
    tmp->gutm = new Gutm(1);
    gr[0] = GetPosition(0)+ floor((x-GetPosition(0))/(GUTM_SIZE*res))*(GUTM_SIZE*res);
    gr[1] = GetPosition(1)+ floor((y-GetPosition(1))/(GUTM_SIZE*res))*(GUTM_SIZE*res);
    gr[2] = GUTM_ZREF;
    tmp->gutm->SetPosition(gr);
    sprintf(l,"Gutm-%f,%f",gr[0],gr[1]);
    tmp->gutm->SetLabel(l);
    tmp->gutm->Initialize(GUTM_SIZE+1,GUTM_SIZE+1,res,hasAltData);
    tmp->gutm->SetEast(GetGutm(gr[0]+GUTM_SIZE*res*1.5,gr[1]+GUTM_SIZE*res*0.5));
    tmp->gutm->SetNorth(GetGutm(gr[0]+GUTM_SIZE*res*0.5,gr[1]+GUTM_SIZE*res*1.5));
    gutm = GetGutm(gr[0]+GUTM_SIZE*res*-0.5,gr[1]+GUTM_SIZE*res*0.5);
    if(gutm)
        gutm->SetEast(tmp->gutm);
    gutm = GetGutm(gr[0]+GUTM_SIZE*res*0.5,gr[1]+GUTM_SIZE*res*-0.5);
    if(gutm)
        gutm->SetNorth(tmp->gutm);
    AddObject(tmp->gutm);
    mainWindow->getScene3D()->PostAddObject(tmp->gutm);
    gutms = tmp;
}

