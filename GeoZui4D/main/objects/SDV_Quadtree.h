// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef SDV_QUADTREE
#define SDV_QUADTREE

#include"objects/nodeTypes/SDV_Data.h"
#include"objects/nodeTypes/SDV_DataHandler.h"

#include<stdio.h>


struct SDV_QuadtreeNode  // nodes of the quadtree
{
    // The four subdivisions of the current node
    SDV_QuadtreeNode * ne;
    SDV_QuadtreeNode * se;
    SDV_QuadtreeNode * sw;
    SDV_QuadtreeNode * nw;
    // The four neighbours at same res of current node
    SDV_QuadtreeNode * n;
    SDV_QuadtreeNode * s;
    SDV_QuadtreeNode * e;
    SDV_QuadtreeNode * w;
    // Parent node
    SDV_QuadtreeNode * up;
    // Jumps to the next node if not a neighbour
    SDV_QuadtreeNode * next;
    double x;
    double y;
    unsigned char resolution;
    double value;
    // normals
    short int nx;
    short int ny;
    short int nz;
    
    SDV_DataHandler * data;
};

struct ListOfListNode  //This is the list of rows in a level
{
    SDV_QuadtreeNode * list;
    ListOfListNode * next;
};

struct ListElevatorNode  // this is a list of the levels, containing lists of line for that particular level
{
    unsigned char resolution;
    ListOfListNode * lists;
    ListElevatorNode * next;
};

class SDV_Quadtree
{
public:
    SDV_Quadtree(double res = 1.0);
    void AddNode(SDV_Data & d, double x, double y);
    void AddNodeToArea(SDV_Data & d, double minx, double miny, double maxx, double maxy);
    SDV_QuadtreeNode * GetData(double x, double y);
    void Print();
    void DrawGrid();
    void Draw(unsigned char res=0,float he = 1.0);
    int DrawLine(int eye);
    void DrawOneLine(SDV_QuadtreeNode * row,float he = 1.0);
    void DrawValues();
    unsigned char GetTopLevelResolution();    
    void writeTo(FILE * outfile);
    void writeTo(FILE * outfile, SDV_QuadtreeNode * n);
private:
    SDV_QuadtreeNode * NewNode();
    void PrintNode(SDV_QuadtreeNode * n);
    void DrawNodeGrid(SDV_QuadtreeNode * n);
    void DrawValues(SDV_QuadtreeNode * n);
    void ExpandTree(double x,double y);
    SDV_QuadtreeNode * UpdateNodeLink(unsigned char res, double x, double y, SDV_QuadtreeNode * node, char dir);
    SDV_QuadtreeNode * FindNode(unsigned char res, double x, double y);
    SDV_QuadtreeNode * CleanArea(SDV_QuadtreeNode * node, double minx, double miny, double maxx, double maxy);
    void AddNodeToSubArea(SDV_QuadtreeNode * node,SDV_Data & d, double minx, double miny, double maxx, double maxy);

    void DeleteFromList(SDV_QuadtreeNode * node);
    void UpdateValues(SDV_QuadtreeNode * node);
    void UpdateList(SDV_QuadtreeNode * tmp);
    void UpdateNormal(SDV_QuadtreeNode * node);

    SDV_QuadtreeNode * quadtree;  // root of the quadtree
    ListElevatorNode * list;    // root of the linked list structure

    double resolution[256]; // This is the lookup table for the resolutions. 

    float zscale;
    float deltaz;

    //hires
    ListOfListNode * rowlist;
    
    //debugging

    FILE * logfile;
};

#endif
