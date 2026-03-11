// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"MBGrid.h"
#include <gz4d/netcdf/File.h>
#include <gz4d/geo/Projection.h>
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include <gz4d/glo/Vertex.h>
#include <gz4d/vector/operations.h>
#include<gz4d/glo/VertexArray.h>

extern MainWindow * mainWindow;

REGISTER_OBJECT(MBGrid,"grd");

MBGrid::MBGrid()
    :firstDraw(true)
{
}

MBGrid::~MBGrid()
{
}

void MBGrid::DrawLowRes(float degradation, Window3D *win)
{
    if(firstDraw)
    {
        abo = std::shared_ptr<gz4d::glo::Buffer>(new gz4d::glo::Buffer(win->getGloContext(),GL_ARRAY_BUFFER));
        eabo = std::shared_ptr<gz4d::glo::Buffer>(new gz4d::glo::Buffer(win->getGloContext(),GL_ELEMENT_ARRAY_BUFFER));
        std::vector<gz4d::glo::VertexArray::N3F_V3F> v;
        v.resize(grid.GetRows()*grid.GetColumns());
        
        for(unsigned int y = 0; y < grid.GetRows(); ++y)
            for(unsigned int x = 0; x < grid.GetColumns(); ++x)
            {
                gz4d::Vector<double,3> normal;
                if(x == grid.GetColumns()-1)
                    normal = gz4d::Vector<double,3>(v[y*grid.GetColumns()+x-1].nx,v[y*grid.GetColumns()+x-1].ny,v[y*grid.GetColumns()+x-1].nz);
                else if (y == grid.GetRows()-1)
                    normal = gz4d::Vector<double,3>(v[(y-1)*grid.GetColumns()+x].nx,v[(y-1)*grid.GetColumns()+x].ny,v[(y-1)*grid.GetColumns()+x].nz);
                else
                {
                    normal =  cross(grid.GetRow(y)[x+1] - grid.GetRow(y)[x], grid.GetRow(y+1)[x] - grid.GetRow(y)[x]);
                    if(!gz4d::IsNan(normal[0]) && !gz4d::IsNan(normal[1]))
                    {
                        normal = normalize(normal);
                    }
                }
                v[y*grid.GetColumns()+x].nx = normal[0];
                v[y*grid.GetColumns()+x].ny = normal[1];
                v[y*grid.GetColumns()+x].nz = normal[2];
                v[y*grid.GetColumns()+x].x = grid.GetRow(y)[x][0];
                v[y*grid.GetColumns()+x].y = grid.GetRow(y)[x][1];
                v[y*grid.GetColumns()+x].z = grid.GetRow(y)[x][2];
            }

        abo->Bind();
        abo->Data(v.size()*sizeof(gz4d::glo::VertexArray::N3F_V3F),&v.front());
        abo->Unbind();

        std::vector<uint32_t> indecies;

        for(unsigned int y = 0; y < grid.GetRows()-1; ++y)
        {
            for(unsigned int x = 0; x < grid.GetColumns(); ++x)
            {
                indecies.push_back(y*grid.GetColumns()+x);
                indecies.push_back((y+1)*grid.GetColumns()+x);
            }
            counts.push_back(2*grid.GetColumns());
            indexOffsets.push_back((const GLvoid*)(y*2*grid.GetColumns()*sizeof(uint32_t)));
        }
        eabo->Bind();
        eabo->Data(indecies.size()*sizeof(uint32_t),&indecies.front());
        eabo->Unbind();

        firstDraw = false;
    }

    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    
    EnableTexture();
    glColor3f(1.0,1.0,1.0);
    glPointSize(5.0);

    abo->Bind();
    eabo->Bind();
    glInterleavedArrays(GL_N3F_V3F,0,NULL);
    glMultiDrawElements(GL_QUAD_STRIP,&counts.front(),GL_UNSIGNED_INT,&indexOffsets.front(),counts.size());
    glDisableClientState(GL_VERTEX_ARRAY);
    abo->Unbind();
    eabo->Unbind();
    DisableTexture();

//     glVertexPointer(3,GL_FLOAT,0,grid.raw());
//     glNormalPointer(GL_FLOAT,0,normals.raw());
//     glEnableClientState(GL_VERTEX_ARRAY);
//     glEnableClientState(GL_NORMAL_ARRAY);
//     //glDrawArrays(GL_POINTS,0,grid.GetColumns()*grid.GetRows());
//     glMultiDrawElements(GL_QUAD_STRIP,&counts.front(),GL_UNSIGNED_INT,&indexOffsets.front(),counts.size());


    
}

// int MBGrid::DrawHighRes(int eye, Window3D *win)
// {
//     return true;
// }

int MBGrid::Load()
{
    if(!GetFilename().empty())
    {
        gz4d::netcdf::File ncf = gz4d::netcdf::File(GetFilename());
        if(ncf)
        {
            gz4d::netcdf::Dim d = ncf.GetDim("x");
            std::vector<double> xs(d.Size());
            ncf.GetVar("x").Get(xs);

            d = ncf.GetDim("y");
            std::vector<double> ys(d.Size());
            ncf.GetVar("y").Get(ys);

            std::vector<float> zs(xs.size()*ys.size());
            gz4d::netcdf::Var zv = ncf.GetVar("z");
            zv.SetCur(0,0);
            zv.Get(zs,ys.size(),xs.size());

            grid.Resize(xs.size(),ys.size());
            //normals.Resize(xs.size(),ys.size());

            autoProjection(xs[0],ys[0]);
            gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
            gz4d::Point<double> geo = proj(xs[0],ys[0]);
            SetPosition(geo[0],geo[1],0.0);
            for(unsigned int y = 0; y < ys.size(); ++y)
                for(unsigned int x = 0; x < xs.size(); ++x)
                {
                    gz4d::Point<double> p = proj(ys[y],xs[x],-zs[y*xs.size()+x]);
                    grid.GetRow(y)[x] = p -geo;
                }

/*            for(unsigned int y = 0; y < ys.size()-1; ++y)
                for(unsigned int x = 0; x < xs.size()-1; ++x)
                {
                    gz4d::Vector<double,3> normal =  cross(grid.GetRow(y)[x+1] - grid.GetRow(y)[x], grid.GetRow(y+1)[x] - grid.GetRow(y)[x]);
                    if(!gz4d::IsNan(normal[0]) && !gz4d::IsNan(normal[1]))
                    {
                        normal = normalize(normal);
                        normals.GetRow(y)[x] = normal;
                    }
                }

            for(unsigned int y = 0; y < ys.size()-1; ++y)
            {
                
                for(unsigned int x = 0; x < xs.size(); ++x)
                {
                    indecies.push_back(y*xs.size()+x);
                    if(x == 0)
                        indexOffsets.push_back((const GLvoid*)(&indecies.back()));
                    indecies.push_back((y+1)*xs.size()+x);
                }
                counts.push_back(xs.size()*2);
            }*/
            return 0;
        }
    }
    return 1;
}
