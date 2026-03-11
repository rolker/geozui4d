// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
// ObjectCollection.h: Base class for objects containing other objects.
//
//////////////////////////////////////////////////////////////////////

#ifndef OBJECTCOLLECTION_H
#define OBJECTCOLLECTION_H

#include "engine/Object.h"

class TextureMap;

class ObjectCollection: public Object
{
protected:
    struct Node;

public:

    DERIVE_OBJECT(ObjectCollection);

    ObjectCollection(GLboolean allocatePosable = GL_TRUE, int pickingTypes = PICK_CLICK1 | PICK_PRESS3);

    virtual void DrawLowRes(float degradation, Window3D *win);
    void DrawOrtho();
    virtual void DrawObjectLowRes(Object *ob, float degradation, Window3D *win);
    virtual int DrawHighRes(int eye, Window3D *win); //mdp 3/7/2000 -- return 1 if done, 0 if not done.
    virtual int DrawObjectHighRes(Object *ob, int eye, Window3D *win);
    virtual void Idle(double time);
    virtual void getHtmlInfo(std::stringstream &html, short level);

    void ResetHighRes();
    void DontUseTextures();
    void UseTextures();
    virtual int AddObject(Object* object,int publish = 1); //mdp made virtual 9/25/2008
    virtual int RemoveObject(Object* object); //mdp made virtual 9/25/2008
    virtual int ReloadObject(Object* object); //mdp made virtual 9/25/2008
    virtual Object * LoadObject(std::string const &fn); //mdp made virtual 9/25/2008
    Object * GetObjectPtr();
    Object * Next();
    int Reset();
    Object * GetObjectPtr(int id);
    Object * GetObjectPtr(std::string l);
    void setHeightExaggeration(GLfloat heightEx);
    void Fuse(Object * other);
    void UpdateCustomTextures();
    void UpdateObjectsUsing(std::shared_ptr<TextureMap> &texture);
    void SaveObject(int id, char * fn);
    void SaveEditedObjects();
    void SetInvisible(int id = -1);
    void SetVisible(int id);
    void PostMenu(int id);
    void SetCustomTexture(int oid,std::shared_ptr<TextureMap> &t);
    void SetCustomTexture(std::shared_ptr<TextureMap> &t);
    void SetDefaultTexture(int id);
    virtual void WriteToScene(std::string scenepath); //mdp made virtual 9/25/2008
    void ResetAllHighRes(Node * start);
    int IsEmpty();
    std::string GetExtension(int id);
    virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);
  
    virtual void publish(int parent);
    virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits) {if (pickType == PICK_PRESS2) return GL_FALSE; else return Object::pickAction(pickType, inputInfo, names, hits);}
  
    virtual Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile); //mdp made virtual 9/25/2008
    virtual Object * LoadXML(std::string const &fname);
    virtual Object * LoadXMLObject(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile); //mdp made virtual 9/25/2008
    virtual void SaveXML(FILE *out, std::string const &path);
    virtual void SaveXMLChildren(FILE *out, std::string const &path); //mdp made virtual 9/25/2008
    
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);

    
    //Bounding box stuff
    void setBBoxStale();
    virtual BBox GetBoundingBox();
    virtual void expandBBoxTime(double t);

    virtual double GetHeightAt(double x, double y, Point3d coord = NULL, GLboolean secondary = GL_FALSE);
    virtual double GetHeightNear(double x, double y, float rangeMeters = 1.0, int heightType = height_max);


protected:

    virtual void preTransformChild() {}
    virtual void postTransformChild() {}

    BBox * localBBox; // this is the object collection's local bbox, it does not take children's bbox into consideration
    
    struct Node
    {
        Object* object;
        Node* next;
        Node* prev;
    };

    Node * nodeList;
    Node * cursor;
    Node * current;
    Node * last;
    static int globalFirstObject;
    
    int bboxStale;
};


#endif
