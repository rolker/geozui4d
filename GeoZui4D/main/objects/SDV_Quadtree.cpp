// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include "objects/SDV_Quadtree.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include<gz4d/glo.h>
#include <GL/glut.h>
#include"SDV_types.h"

extern int dominantEye;

using namespace std;

SDV_Quadtree::SDV_Quadtree(double res)
{
    int i;
    resolution[0] = res/2.0;
    quadtree = NULL;
    list = NULL;

    for (i = 1; i < 256; i++)
    {
        resolution[i] = resolution[i-1]*2.0;
    }
    cerr << "Node size: " << sizeof(SDV_QuadtreeNode) << "\n";

    zscale = 2;
    deltaz = 0.0;

    rowlist = NULL;
}

SDV_QuadtreeNode * SDV_Quadtree::NewNode()
{
SDV_QuadtreeNode * tmp;

tmp = new SDV_QuadtreeNode;
tmp->ne = NULL;
tmp->se = NULL;
tmp->sw = NULL;
tmp->nw = NULL;
tmp->n = NULL;
tmp->s = NULL;
tmp->e = NULL;
tmp->w = NULL;
tmp->next = NULL;
tmp->up = NULL;
tmp->resolution = 1;
tmp->data = NULL;
tmp->value = 0.0;
tmp->x =0.0;
tmp->y =0.0;
tmp->nx =0;
tmp->ny =0;
tmp->nz =32767;

return tmp;  
}

void SDV_Quadtree::AddNode(SDV_Data &d, double x, double y)
{
    SDV_QuadtreeNode * tmp;
    SDV_QuadtreeNode * newtmp;
    SDV_QuadtreeNode * foundnode;

    if (quadtree == NULL)
    {
        tmp = NewNode();
        tmp->data = d.NewHandler(x,y);
        tmp->data->Add(d);
        tmp->value = tmp->data->GetAverage();
        tmp->x = x;
        tmp->y = y;
        quadtree = tmp;
        list = new ListElevatorNode;
        list->resolution = 1;
        list->lists = new ListOfListNode;
        list->next = NULL;
        list->lists->list = tmp;
        list->lists->next = NULL;
    }
    else
    {
        // look for an exsisting node, if it doesn't exsist
        // create a node and insert it in the tree and update the list links
        // if it does exsist, merge the new data with the old one.
        foundnode = GetData(x,y);
        if(!foundnode)
        {
            if (x < quadtree->x - resolution[quadtree->resolution-1] || x >= quadtree->x + resolution[quadtree->resolution-1] || y < quadtree->y - resolution[quadtree->resolution-1]|| y >= quadtree->y + resolution[quadtree->resolution-1])
            {
                ExpandTree(x,y);
            }
            tmp = quadtree;
            while(tmp->resolution != 1)
            {
                if(x < tmp->x)
                {
                    if(y < tmp->y)
                    {
                        if(tmp->sw == NULL)
                        {
                            newtmp = NewNode();
                            newtmp->up = tmp;
                            newtmp->resolution = tmp->resolution-1;
                            newtmp->x = tmp->x-resolution[newtmp->resolution-1];
                            newtmp->y = tmp->y-resolution[newtmp->resolution-1];
                            tmp->sw = newtmp;
                            tmp = tmp->sw;
                            UpdateList(tmp);
                        }
                        else
                        {
                            tmp = tmp->sw;
                        }
                    }
                    else
                    {
                        if(tmp->nw == NULL)
                        {
                            newtmp = NewNode();
                            newtmp->up = tmp;
                            newtmp->resolution = tmp->resolution-1;
                            newtmp->x = tmp->x-resolution[newtmp->resolution-1];
                            newtmp->y = tmp->y+resolution[newtmp->resolution-1];
                            tmp->nw = newtmp;
                            tmp = tmp->nw;
                            UpdateList(tmp);
                        }
                        else
                        {
                            tmp = tmp->nw;
                        }
                    }
                }
                else
                {
                    //if(y < tmp->y+resolution[tmp->resolution-1])
                    if(y < tmp->y)
                    {
                        if(tmp->se == NULL)
                        {
                            newtmp = NewNode();
                            newtmp->up = tmp;
                            newtmp->resolution = tmp->resolution-1;
                            newtmp->y = tmp->y-resolution[newtmp->resolution-1];
                            newtmp->x = tmp->x+resolution[newtmp->resolution-1];
                            tmp->se = newtmp;
                            tmp = tmp->se;
                            UpdateList(tmp);
                        }
                        else
                        {
                            tmp = tmp->se;
                        }
                    }
                    else
                    {
                        if(tmp->ne == NULL)
                        {
                            newtmp = NewNode();
                            newtmp->up = tmp;
                            newtmp->resolution = tmp->resolution-1;
                            newtmp->x = tmp->x + resolution[newtmp->resolution-1];
                            newtmp->y = tmp->y + resolution[newtmp->resolution-1];
                            tmp->ne = newtmp;
                            tmp = tmp->ne;
                            UpdateList(tmp);
                        }
                        else
                        {
                            tmp = tmp->ne;
                        }
                    }
                }
                tmp->data = d.NewHandler(tmp->x,tmp->y);
                tmp->data->Add(d);
                UpdateValues(tmp);
            }
        }
        else
        {
            //add data object to handler
            foundnode->data->Add(d);
            UpdateValues(foundnode);
        }
    }
}

// This function adds data to every node within the rectangle specified by the bounds
void SDV_Quadtree::AddNodeToArea(SDV_Data &d, double minx, double miny, double maxx, double maxy)
{
    SDV_QuadtreeNode * tmp;
    
//    cerr << "AddNodeToArea: " << minx << ", " << miny << ", " << maxx << ", " << maxy << "\n";
    
    // new quadtree
    if (quadtree == NULL)
    {
        // start by creating a node
        tmp = NewNode();        
        tmp->x = minx+((maxx-minx)/2.0);
        tmp->y = miny+((maxy-miny)/2.0);
        quadtree = tmp;
        list = new ListElevatorNode;
        list->resolution = 1;
        list->lists = new ListOfListNode;
        list->next = NULL;
        list->lists->list = tmp;
        list->lists->next = NULL;
//        cerr << "New quadtree: " << tmp->x << ", " << tmp->y << "\n";
    
    }        
    
    // do we need to expand?
    
    if (minx < quadtree->x-resolution[quadtree->resolution-1] || miny < quadtree->y-resolution[quadtree->resolution-1])
    {
//        cerr << "Expanding!\n";
        ExpandTree(minx,miny);
    }
    if (maxx > quadtree->x+resolution[quadtree->resolution-1] || maxy > quadtree->y+resolution[quadtree->resolution-1])
    {
//        cerr << "Expanding!\n";
        ExpandTree(maxx,maxy);
    }
    
    AddNodeToSubArea(quadtree,d,minx,miny,maxx,maxy);

    // check for empty nodes (after creating all nodes in the rectangle, some might not
    // have been touched, depending on how the data gets handled (circular pattern perhaps?)

    quadtree = CleanArea(quadtree,minx,miny,maxx,maxy);    
            
}

void SDV_Quadtree::AddNodeToSubArea(SDV_QuadtreeNode * node,SDV_Data & d, double minx, double miny, double maxx, double maxy)
{
    if(node->resolution != 1)
    {
        //not yet at the bottom
        
        // do we need to go west?
        if (minx < node->x)
        {
            //north?
            if (maxy > node->y)
            {
                if(node->nw == NULL)
                {
                    node->nw = NewNode();
                    node->nw->resolution = node->resolution-1;
                    node->nw->x = node->x - resolution[node->resolution-2];
                    node->nw->y = node->y + resolution[node->resolution-2];
                    node->nw->up = node;
                    UpdateList(node->nw);
//					cerr << "New node: " << node->nw->x << ", " << node->nw->y << " res: " << (int)node->nw->resolution << "\n";
                }
                AddNodeToSubArea(node->nw,d,minx,miny,maxx,maxy);
            }
            //south?
            if (miny < node->y)
            {
                if(node->sw == NULL)
                {
                    node->sw = NewNode();
                    node->sw->resolution = node->resolution-1;
                    node->sw->x = node->x - resolution[node->resolution-2];
                    node->sw->y = node->y - resolution[node->resolution-2];
                    node->sw->up = node;
                    UpdateList(node->sw);
//					cerr << "New node: " << node->sw->x << ", " << node->sw->y << " res: " << (int)node->sw->resolution << "\n";
                }
                AddNodeToSubArea(node->sw,d,minx,miny,maxx,maxy);
            }
        }
        // do we need to go east?
        if (maxx > node->x)
        {
            //north?
            if (maxy > node->y)
            {
                if(node->ne == NULL)
                {
                    node->ne = NewNode();
                    node->ne->resolution = node->resolution-1;
                    node->ne->x = node->x + resolution[node->resolution-2];
                    node->ne->y = node->y + resolution[node->resolution-2];
                    node->ne->up = node;
                    
                    UpdateList(node->ne);
//					cerr << "New node: " << node->ne->x << ", " << node->ne->y << " res: " << (int)node->ne->resolution << "\n";
                }
                AddNodeToSubArea(node->ne,d,minx,miny,maxx,maxy);
            }
            //south?
            if (miny < node->y)
            {
                if(node->se == NULL)
                {
                    node->se = NewNode();
                    node->se->resolution = node->resolution-1;
                    node->se->x = node->x + resolution[node->resolution-2];
                    node->se->y = node->y - resolution[node->resolution-2];
                    node->se->up = node;
                    UpdateList(node->se);
//					cerr << "New node: " << node->se->x << ", " << node->se->y << " res: " << (int)node->se->resolution << "\n";
                }
                AddNodeToSubArea(node->se,d,minx,miny,maxx,maxy);
            }
        }
    }
    else
    {
        if(node->data == NULL)
        {
            node->data = d.NewHandler(node->x,node->y);
        }
//		cerr << "Adding to: " << node->x << ", " << node->y << "\n";
        node->data->Add(d);
//		cerr << " Value: " << node->data->GetAverage() << "\n";
        UpdateValues(node);
    }
}

SDV_QuadtreeNode * SDV_Quadtree::CleanArea(SDV_QuadtreeNode * node, double minx, double miny, double maxx, double maxy)
{
    // do we need to go west?
//    cerr << "Clean area " << minx << ", " << miny << "  " << maxx << ", " << maxy << "\n";
//	cerr << "At node: " << node->x << ", " << node->y << " res: " << (int)node->resolution << "\n";
    if (minx < node->x)
    {
//        cerr << "West\n";
        //north?
        if (maxy > node->y)
        {
//            cerr << "North\n";
            if(node->nw != NULL)
            {
                node->nw = CleanArea(node->nw,minx,miny,maxx,maxy);
            }
        }
        //south?
        if (miny < node->y)
        {
//            cerr << "South\n";
            if(node->sw != NULL)
            {
                node->sw = CleanArea(node->sw,minx,miny,maxx,maxy);
            }
        }
    }

    // do we need to go east?
    if (maxx > node->x)
    {
//        cerr << "East\n";
        //north?
        if (maxy > node->y)
        {
//            cerr << "North\n";
            if(node->ne != NULL)
            {
                node->ne = CleanArea(node->ne,minx,miny,maxx,maxy);
            }
        }
        //south?
        if (miny < node->y)
        {
//            cerr << "South\n";
            if(node->se != NULL)
            {
                node->se = CleanArea(node->se,minx,miny,maxx,maxy);
            }
        }
    }
    
    if(node->data != NULL)
    {
        if(gz4d::IsNan(node->data->GetAverage()))
        {
//            cerr << "Deleting data handler: " << node->x << ", " << node->y << "\n";
            delete node->data;
            node->data = NULL;
        }
        else
        {
//            cerr << "NOT Deleting data handler: " << node->x << ", " << node->y << "\n";
        }
    }
    
    if(node->data == NULL && node->nw == NULL && node->sw == NULL && node->ne == NULL && node->se == NULL)
    {
        //ok to delete this node!
//        cerr << "Deleting node: " << node->x << ", " << node->y << "\n";
        if(node->e != NULL)
        {
            node->e->w = NULL;
        }
        if(node->w != NULL)
        {
            node->w->e = NULL;
        }
        if(node->s != NULL)
        {
            node->s->n = NULL;
        }
        if(node->n != NULL)
        {
            node->n->s = NULL;
        }
        
        if(node->w != NULL)
        {
            node->w->next=node->next;
        }
        else
        {
            DeleteFromList(node);
        }

        delete node;
        node = NULL;
    }
    
    return node;    
}

void SDV_Quadtree::DeleteFromList(SDV_QuadtreeNode * node)
{
    ListElevatorNode * tmplist;
    ListOfListNode * tmplol, * tmplol2;
    SDV_QuadtreeNode * tmpnode;

    tmplist = list;
    // go down to the proper res level
    while(tmplist->resolution != node->resolution)
    {
        tmplist = tmplist->next;
    }
    tmplol = tmplist->lists;
    // find the correct row
    while(tmplol->next != NULL && tmplol->list->y != node->y)
    {
        tmplol = tmplol->next;
    }
    
    if (tmplol->list == node)
    {
        tmplol->list = node->next;
        if(tmplol->list == NULL)
        {
            tmplol = tmplist->lists;
            if(tmplist->lists->list == NULL)
            {
                tmplist->lists = tmplist->lists->next;
                delete tmplol;
            }
            else
            {
                while(tmplol->next != NULL)
                {
                    if (tmplol->next->list == NULL)
                    {
                        tmplol2 = tmplol->next;
                        tmplol->next = tmplol->next->next;
                        delete tmplol2;
                    }
                    else
                    {
                        tmplol = tmplol->next;
                    }
                }
            }
        }
    }
    else
    {
        tmpnode = tmplol->list;
        while((tmpnode->next != NULL) && (tmpnode->next != node))
        {
            tmpnode = tmpnode->next;
        }
        if(tmpnode->next == node)
        {
            tmpnode->next = tmpnode->next->next;
        }
    }
}

void SDV_Quadtree::UpdateList(SDV_QuadtreeNode * tmp)
{
    ListElevatorNode * tmplist;
    ListOfListNode * tmplol;
    SDV_QuadtreeNode * tmpnode;

    tmp->n = UpdateNodeLink(tmp->resolution, tmp->x, tmp->y+resolution[tmp->resolution],tmp,'n');
    tmp->e = UpdateNodeLink(tmp->resolution, tmp->x+resolution[tmp->resolution], tmp->y,tmp,'e');
    tmp->s = UpdateNodeLink(tmp->resolution, tmp->x, tmp->y-resolution[tmp->resolution],tmp,'s');
    tmp->w = UpdateNodeLink(tmp->resolution, tmp->x-resolution[tmp->resolution], tmp->y,tmp,'w');

    // update the list elevator
    // are we the westmost node in the row?

    
//    cerr << "Updating the list structure for node " << tmp->x << ", " << tmp->y << " at res: " << (int)tmp->resolution << "\n";
    
    tmplist = list;
//    cerr << "Going down the elevator starting at: " << (int)tmplist->resolution << "\n";
    
    while(tmplist->resolution != tmp->resolution)
    {
        tmplist = tmplist->next;
//        cerr << (int)tmplist->resolution << "...\n";
    }
    tmplol = tmplist->lists;
//	cerr << "Going through lol starting with y=" << tmplol->list->y << "\n";
    while(tmplol->next != NULL && tmplol->list->y != tmp->y)
    {
        tmplol = tmplol->next;
//        cerr << tmplol->list->y << "...\n";
    }
    if(tmplol->list->y != tmp->y)
    {
//        cerr << "list not found, creating new one!\n";
        tmplol->next = new ListOfListNode;
        tmplol = tmplol->next;
        tmplol->list = tmp;
        tmplol->next = NULL;
    }
    else
    {
        tmpnode = tmplol->list;
//        cerr << "list found, starting at x=" << tmpnode->x << "\n";
        if (tmpnode->x > tmp->x)
        {
//            cerr << "New node is west, so inserting at head of the list\n";
            tmp->next = tmpnode;
            tmplol->list = tmp;
        }
        else
        {
            while(tmpnode->x < tmp->x)
            {
                if(tmpnode->next == NULL)
                {
//                    cerr << "At the end of list, appending\n";
                    tmpnode->next = tmp;
                }
                else
                {
                    if(tmpnode->next->x > tmp->x)
                    {
//                        cerr << "next node, x=" << tmpnode->next->x << " is east, insert here\n";
                        tmp->next = tmpnode->next;
                        tmpnode->next = tmp;
                    }
                }
                tmpnode = tmpnode->next;
//                cerr << tmpnode->x << "...\n";
            }
        }
    }
}

SDV_QuadtreeNode * SDV_Quadtree::GetData(double x, double y)
{
    return FindNode(1,x,y);
}

unsigned char SDV_Quadtree::GetTopLevelResolution()
{
    if(quadtree != NULL)
    {
        return quadtree->resolution;
    }
    return 0;
}

void SDV_Quadtree::ExpandTree(double x, double y)
{
    SDV_QuadtreeNode * tmp;
    ListElevatorNode * tmplist;
    
    tmp = NewNode();
    tmp->resolution = quadtree->resolution+1;
    tmp->value = quadtree->value;
    
    quadtree->up = tmp;
    
    // Add a node on top of the list elevator
    tmplist = new ListElevatorNode;
    tmplist->resolution = tmp->resolution;
    tmplist->next = list;
    tmplist->lists = new ListOfListNode;
    tmplist->lists->list = tmp;
    tmplist->lists->next = NULL;
    list = tmplist;
    
    if(x<quadtree->x)
    {
        if(y<quadtree->y)
        {
            //current root becomes ne
            tmp->ne = quadtree;
            tmp->x = quadtree->x - resolution[quadtree->resolution-1];
            tmp->y = quadtree->y - resolution[quadtree->resolution-1];
        }
        else
        {
            // current root becomes se
            tmp->se = quadtree;
            tmp->x = quadtree->x - resolution[quadtree->resolution-1];
            tmp->y = quadtree->y + resolution[quadtree->resolution-1];
        }
    }
    else
    {
        if(y<quadtree->y)
        {
            //current root becomes nw
            tmp->nw = quadtree;
            tmp->x = quadtree->x + resolution[quadtree->resolution-1];
            tmp->y = quadtree->y - resolution[quadtree->resolution-1];
        }
        else
        {
            // current root becomes sw
            tmp->sw = quadtree;
            tmp->x = quadtree->x + resolution[quadtree->resolution-1];
            tmp->y = quadtree->y + resolution[quadtree->resolution-1];
        }
    }    
    quadtree = tmp;
    if(x < quadtree->x - resolution[quadtree->resolution-1] || x >= quadtree->x+resolution[quadtree->resolution-1] || y < quadtree->y - resolution[quadtree->resolution-1]|| y >= quadtree->y+resolution[quadtree->resolution-1])
    {
        ExpandTree(x,y);
    }
}

void SDV_Quadtree::Print()
{
    PrintNode(quadtree);   
}

void SDV_Quadtree::PrintNode(SDV_QuadtreeNode * n)
{
    if (n->data != NULL)
    {
//        cerr << "Data present!\n";
    }
    if (n->ne != NULL)
    {
        PrintNode(n->ne);
    }
    if (n->nw != NULL)
    {
        PrintNode(n->nw);
    }
    if (n->se != NULL)
    {
        PrintNode(n->se);
    }
    if (n->sw != NULL)
    {
        PrintNode(n->sw);
    }
}

void SDV_Quadtree::DrawGrid()
{
    if (quadtree != NULL)
    {
        zscale += deltaz;
        if (zscale > 5 || zscale <= 0)
        {
            deltaz = -deltaz;
        }
        DrawNodeGrid(quadtree);
    }
}

void SDV_Quadtree::DrawNodeGrid(SDV_QuadtreeNode * n)
{
    
/*	zscale += deltaz;
    if (zscale > 10 || zscale <= 0)
    {
        deltaz = -deltaz;
    }*/

    glColor3f(1.0,1.0,1.0);
    glBegin(GL_LINE_LOOP);
        glVertex3f(n->x-resolution[n->resolution-1],n->y-resolution[n->resolution-1],zscale*resolution[n->resolution]);
        glVertex3f(n->x+resolution[n->resolution-1],n->y-resolution[n->resolution-1],zscale*resolution[n->resolution]);
        glVertex3f(n->x+resolution[n->resolution-1],n->y+resolution[n->resolution-1],zscale*resolution[n->resolution]);
        glVertex3f(n->x-resolution[n->resolution-1],n->y+resolution[n->resolution-1],zscale*resolution[n->resolution]);
    glEnd();
    
    
//    glBegin(GL_LINES);
    
    if (n->ne != NULL)
    {
//        glVertex3f(n->x,n->y,zscale*resolution[n->resolution]);
//        glVertex3f(n->ne->x,n->ne->y,zscale*resolution[n->ne->resolution]);
        DrawNodeGrid(n->ne);
    }
    if (n->nw != NULL)
    {
// 		  glVertex3f(n->x,n->y,zscale*resolution[n->resolution]);
//		  glVertex3f(n->nw->x,n->nw->y,zscale*resolution[n->nw->resolution]);
        DrawNodeGrid(n->nw);
    }
    if (n->se != NULL)
    {
//        glVertex3f(n->x,n->y,zscale*resolution[n->resolution]);
//        glVertex3f(n->se->x,n->se->y,zscale*resolution[n->se->resolution]);
        DrawNodeGrid(n->se);
    }
    if (n->sw != NULL)
    {
//        glVertex3f(n->x,n->y,zscale*resolution[n->resolution]);
//        glVertex3f(n->sw->x,n->sw->y,zscale*resolution[n->sw->resolution]);
        DrawNodeGrid(n->sw);
    }
    
//   glEnd();
}

SDV_QuadtreeNode * SDV_Quadtree::UpdateNodeLink(unsigned char res, double x, double y, SDV_QuadtreeNode * node, char dir)
{
    SDV_QuadtreeNode * tmp;

    tmp = FindNode(res,x,y);
    
    if (tmp == NULL)
    {
        return NULL;
    }
    else
    {
        switch (dir)
        {
        case 'n':
            tmp->s = node;
            break;
        case 'e':
            tmp->w = node;
            break;
        case 's':
            tmp->n = node;
            break;
        case 'w':
            tmp->e = node;
            break;
        }
        return tmp;
    }
}


SDV_QuadtreeNode * SDV_Quadtree::FindNode(unsigned char res, double x, double y)
{
    SDV_QuadtreeNode * cursor;
    // traverse the tree until the correct node is found, if it exsists
    // return a ptr to the node, otherwise return NULL
    cursor = quadtree;
    
    if(x >= cursor->x-resolution[cursor->resolution-1] && x < cursor->x+resolution[cursor->resolution-1] && y >= cursor->y-resolution[cursor->resolution-1] && y < cursor->y+resolution[cursor->resolution-1])
    {
        while(cursor != NULL)
        {
            if(cursor->resolution == res)
            {
                return cursor;
            }
            else
            {
                if (x < cursor->x)
                {
                    if(y < cursor->y)
                    {
                        cursor = cursor->sw;
                    }
                    else
                    {
                        cursor = cursor->nw;
                    }
                }
                else
                {
                    if(y < cursor->y)
                    {
                        cursor = cursor->se;
                    }
                    else
                    {
                        cursor = cursor->ne;
                    }
                }
            }            
        }
        return NULL;
    }
    else
    {
        return NULL;
    }
}

void SDV_Quadtree::Draw(unsigned char res,float he)
{
    ListElevatorNode * le;
    ListOfListNode *lol;
    SDV_QuadtreeNode * row;

    rowlist = NULL;
    glShadeModel(GL_SMOOTH);
    if(res<1)
        res=1;

    if(quadtree != NULL)
    {    
        le = list;
        if (res >= le->resolution)
        {
/*			if (le->resolution-1 < 1)
            {
                return;
            }
            else
            {*/
                res = le->resolution-1;
            //}
        }
        while (le->resolution != res)
        {
            le = le->next;
        }
        lol = le->lists;
        while(lol != NULL)
        {
            row = lol->list;
            DrawOneLine(row,he);
            lol = lol->next;
        }
    }
}

int SDV_Quadtree::DrawLine(int eye)
{
    ListElevatorNode * le;

    SDV_QuadtreeNode * row;

    if(quadtree != NULL)
    {
        if(rowlist == NULL)
        {
            le = list;
            while (le->resolution != 1)
            {
                le = le->next;
            }
            rowlist = le->lists;
        }
        if(rowlist != NULL)
        {
            row = rowlist->list;
            DrawOneLine(row);
            if (eye == dominantEye || eye == MONO_EYE) //update our idea of the current row unless we're on the first of two eyes.
            {
                rowlist = rowlist->next;
            }
            if(rowlist != NULL)
            {
                return 0;
            }
        }
        return 1;
    }
    return 1;
}

void SDV_Quadtree::DrawOneLine(SDV_QuadtreeNode * row, float he)
{
    glBegin(GL_TRIANGLES);
    while(row != NULL)
    {
        if(row->e != NULL)
        {
            if(row->e->s != NULL)
            {
                glNormal3s(row->nx,row->ny,(short)(row->nz / he));
                glVertex3f(row->x,row->y,row->value);
                glNormal3s(row->e->nx,row->e->ny,(short)(row->e->nz / he));
                glVertex3f(row->e->x,row->e->y,row->e->value);
                glNormal3s(row->e->s->nx,row->e->s->ny,(short)(row->e->s->nz / he));
                glVertex3f(row->e->s->x,row->e->s->y,row->e->s->value);
            }
            else if(row->s != NULL) // far corner not available, so link the closer corners
            {
                glNormal3s(row->nx,row->ny,(short)(row->nz / he));
                glVertex3f(row->x,row->y,row->value);
                glNormal3s(row->s->nx,row->s->ny,(short)(row->s->nz / he));
                glVertex3f(row->s->x,row->s->y,row->s->value);
                glNormal3s(row->e->nx,row->e->ny,(short)(row->e->nz / he));
                glVertex3f(row->e->x,row->e->y,row->e->value);

            }
        }
        if(row->s != NULL)
        {
            if(row->s->e != NULL)
            {
                glNormal3s(row->nx,row->ny,(short)(row->nz / he));
                glVertex3f(row->x,row->y,row->value);
                glNormal3s(row->s->nx,row->s->ny,(short)(row->s->nz / he));
                glVertex3f(row->s->x,row->s->y,row->s->value);
                glNormal3s(row->s->e->nx,row->s->e->ny,(short)(row->s->e->nz / he));
                glVertex3f(row->s->e->x,row->s->e->y,row->s->e->value);
            }
            if(row->w == NULL && row->s->w != NULL)
            {
                glNormal3s(row->nx,row->ny,(short)(row->nz / he));
                glVertex3f(row->x,row->y,row->value);
                glNormal3s(row->s->nx,row->s->ny,(short)(row->s->nz / he));
                glVertex3f(row->s->x,row->s->y,row->s->value);
                glNormal3s(row->s->w->nx,row->s->w->ny,(short)(row->s->w->nz / he));
                glVertex3f(row->s->w->x,row->s->w->y,row->s->w->value);
            }
        }
        row = row->next;
    }
    glEnd();
}

void SDV_Quadtree::DrawValues()
{
    if (quadtree != NULL)
    {
        DrawValues(quadtree);
    }
}

void SDV_Quadtree::DrawValues(SDV_QuadtreeNode * n)
{
    if (n->data != NULL)
    {
        n->data->Draw();
    }
            
    if (n->ne != NULL)
    {
        DrawValues(n->ne);
    }
    if (n->nw != NULL)
    {
        DrawValues(n->nw);
    }
    if (n->se != NULL)
    {
        DrawValues(n->se);
    }
    if (n->sw != NULL)
    {
        DrawValues(n->sw);
    }
}

void SDV_Quadtree::writeTo(FILE * outfile)
{
    if (quadtree != NULL)
    {
        writeTo(outfile, quadtree);
    }
}

void SDV_Quadtree::writeTo(FILE * outfile, SDV_QuadtreeNode * n)
{
    if (n->data != NULL)
    {
        n->data->writeTo(outfile);
    }
            
    if (n->ne != NULL)
    {
        writeTo(outfile, n->ne);
    }
    if (n->nw != NULL)
    {
        writeTo(outfile, n->nw);
    }
    if (n->se != NULL)
    {
        writeTo(outfile, n->se);
    }
    if (n->sw != NULL)
    {
        writeTo(outfile, n->sw);
    }
}

void SDV_Quadtree::UpdateValues(SDV_QuadtreeNode * node)
{
    SDV_QuadtreeNode * tmp;
    int count;
    double sum;
    tmp = node;
    
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
        {
            tmp->value = tmp->data->GetAverage();
        }
        else
        {
            count =0;
            sum = 0;
            if(tmp->ne != NULL)
            {
                count++;
                sum += tmp->ne->value;
            }
            if(tmp->se != NULL)
            {
                count++;
                sum += tmp->se->value;
            }
            if(tmp->sw != NULL)
            {
                count++;
                sum += tmp->sw->value;
            }
            if(tmp->nw != NULL)
            {
                count++;
                sum += tmp->nw->value;
            }
            tmp->value = sum/(double)count;
        }
        UpdateNormal(tmp);
        UpdateNormal(tmp->n);
        UpdateNormal(tmp->w);
        tmp = tmp->up;
    }
}

void SDV_Quadtree::UpdateNormal(SDV_QuadtreeNode * node)
{
    double v1,v2,v3;
    double dhx, dhy, dhz, l;
    
    if(node != NULL)
    {
        if(node->s != NULL && node->e != NULL)
        {
            v1 = node->value;
            v2 = node->s->value;
            v3 = node->e->value;

            dhy = (v2 - v1); // shade with the scale built in
            dhx = (v1 - v3);
            // the grid gap is cellsize
            l = sqrt(dhx*dhx + dhy*dhy + resolution[node->resolution]*resolution[node->resolution]);

            dhx /= l;
            dhy /= l;
            dhz = resolution[node->resolution]/l;

            node->nx = (short int)(32767*dhx);
            node->ny = (short int)(32767*dhy);
            node->nz = (short int)(32767*dhz);
        }
        else
        {
            node->nx = 0;
            node->ny = 0;
            node->nz = 32767;
        }
    }
}
