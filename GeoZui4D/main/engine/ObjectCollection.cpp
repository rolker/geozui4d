/* Copyright (c) 2002 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "MainWindow.h"
#include "engine/PosableStateBase.h"
#include "engine/ObjectCollection.h"
#include "utilities/DottedAddress.h"
#include <iostream>
#include "engine/Scene3D.h"
#include <boost/filesystem.hpp>
#include "interactors/PointHandle.h"
#include "engine/ObjectPoseState.h"
#include "engine/Notes.h"
#include "engine/TextureMaps/TextureMap.h"
#include <boost/property_tree/xml_parser.hpp>

using namespace std;

REGISTER_OBJECT(ObjectCollection, "");

extern MainWindow * mainWindow;
extern PointHandle *pointHandle;

int ObjectCollection::globalFirstObject = 1;

/*--------------------------------------------------------*/
ObjectCollection::ObjectCollection(GLboolean allocatePosable, int pickingTypes)
:Object(allocatePosable, pickingTypes)
{
	nodeList = last = NULL;
	transparencyLevel = -1;
	if(id==0)//I'm root
		published = 1;
	bboxStale = 1;
	localBBox = NULL;
}


/*--------------------------------------------------------*/

int ObjectCollection::AddObject(Object * object, int publish)
{
	Node * node;
	BBox ob;
  
	if (!object)
		return TCL_ERROR;


	object->setEdited(GL_FALSE); //unfortunately, prior setting of the georef makes it think it changed!
	object->setRespondToPress2(GL_FALSE);

	object->setParent(this);
	setBBoxStale();

	object->UpdateTexture();//moved by mdp 3/6/2003 

	node = new Node;
	if (node == NULL)
	{
		return 1;
	}
	node->object = object;
	node->next = NULL;
	node->prev = last;
	if (last)
		last = last->next = node;
	else
		last = nodeList = node;
		
	if(!publish)
	{
		object->dontPublish();
	}
	else
	{
		if(published==1)
			object->publish(GetId());
	}
	
	return 0;
}

void ObjectCollection::publish(int parent)
{
  Node * cursor;
  Object * currentObject;
  
  Object::publish(parent);
  
  if(published == 1 && nodeList)
  {
 		cursor = nodeList;
 		while (cursor != NULL)
 		{
 			currentObject = cursor->object;
			currentObject->publish(GetId());
			cursor = cursor->next;
		}
  }
}



/*--------------------------------------------------------*/
int ObjectCollection::DrawHighRes(int eye, Window3D *win)
{
  Node * cursor;
  Object * currentObject;

  if (doneFlag)
		return doneFlag;

  doneFlag = 1;
  if(nodeList)
  {
 		cursor = nodeList;
 		while (cursor != NULL)
 		{
 			currentObject = cursor->object;
			doneFlag &= DrawObjectHighRes(currentObject, eye, win);
			cursor = cursor->next;
		}
  }
  return doneFlag;
}

/*--------------------------------------------------------*/
int ObjectCollection::DrawObjectHighRes(Object *ob, int eye, Window3D *win)
{
	PosableStateBase *pose;
	int doneFlag = 1; //gets unset if any objects are not done.
	int childTransparencyLevel;
	short winLevel;

	winLevel = win->getTransparencyLevel();
	childTransparencyLevel = ob->getTransparencyLevel();
	if (!ob->isReferable() && !ob->IsGarbage() && ob->isVisible(win) &&
		(childTransparencyLevel == winLevel || childTransparencyLevel < 0) && !ob->isOutsideTimeBounds())
	{
		if (childTransparencyLevel > 0) //if the child is a collection and is transparent, fool its children into drawing at level 0.
			win->setTransparencyLevel(0);

		glPushMatrix();
		glLoadIdentity();

        postTransformChild();
		pose = ob->getPosable();
		pose->applyTransforms(win->getViewInfo());
        preTransformChild();

		if (ob->asWireframe())
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			
		ob->drawObjectExtras(0, win);

		doneFlag = ob->DrawHighRes(eye, win);

		if (ob->asWireframe())
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glLoadIdentity();
		if (ob->isSelected() && ob->isEditable() && ob->getAllowBaseExternalEdit())
			pointHandle->drawIn(win, pose->getPosition(), ob->isUsingInternalHandle()? 2 : 1);
		glPopMatrix();
		if (childTransparencyLevel > 0)
			win->setTransparencyLevel(winLevel);
	}
	return doneFlag;
}


void ObjectCollection::DrawOrtho()
{
    Node * cursor;
    Object * currentObject;

    if(nodeList)
    {
        cursor = nodeList;
        while (cursor != NULL)
        {
            currentObject = cursor->object;
            currentObject->DrawOrtho();
            cursor = cursor->next;
        }
    }
}

/*--------------------------------------------------------*/
void ObjectCollection::DrawLowRes(float degradation, Window3D *win)
{
	Node * cursor;
	Object * currentObject;

	if(nodeList)
	{
		cursor = nodeList;
	    while (cursor != NULL)
		{
			currentObject = cursor->object;
			DrawObjectLowRes(currentObject, degradation, win);
			cursor = cursor->next;
		}
	}
}

/*--------------------------------------------------------*/
void ObjectCollection::DrawObjectLowRes(Object *ob, float degradation, Window3D *win)
{
	PosableStateBase *pose;
	int childTransparencyLevel;
	short winLevel;

	childTransparencyLevel = ob->getTransparencyLevel();
	winLevel = win->getTransparencyLevel();

	if (!ob->isReferable() && !ob->IsGarbage() && ob->isVisible(win) &&
		(childTransparencyLevel == winLevel || childTransparencyLevel < 0) && !ob->isOutsideTimeBounds())
	{
		if (childTransparencyLevel > 0) //if the child is a collection and is transparent, fool its children into drawing at level 0.
			win->setTransparencyLevel(0);

		glPushMatrix();
		glLoadIdentity();

        postTransformChild();
		pose = ob->getPosable();
		pose->applyTransforms(win->getViewInfo());
        preTransformChild();

		if (ob->asWireframe())
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		ob->drawObjectExtras(degradation, win);

		if (ob->getDegradationFlag() == OFF)
			ob->DrawLowRes(0.0, win);
		else
			ob->DrawLowRes(degradation, win);

		if (ob->asWireframe())
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glLoadIdentity();
		if (ob->isSelected() && ob->isEditable() && ob->getAllowBaseExternalEdit())
			pointHandle->drawIn(win, pose->getPosition(), ob->isUsingInternalHandle()? 2 : 1);

		glPopMatrix();

		if (childTransparencyLevel > 0)
			win->setTransparencyLevel(winLevel);
	}
}

void ObjectCollection::Idle(double time)
{
	Node * cursor;
	cursor = nodeList;
	Object::Idle(time);
	if(nodeList)
	{
		while (cursor != NULL)
		{
			if (!cursor->object->IsGarbage())
			{
				cursor->object->Idle(time);
			}
			cursor = cursor->next;
		}
	}
}

void ObjectCollection::ResetHighRes()
{
	Node * cursor;
	Object * currentObject;
    
	if(nodeList)
	{
		cursor = nodeList;
		while (cursor != NULL)
		{
			currentObject = cursor->object;
			if (!currentObject->IsGarbage() && currentObject->isVisible())
			{
				currentObject->ResetHighRes();
			}
			cursor = cursor->next;
		}
	}
	Object::ResetHighRes();
}

void ObjectCollection::UseTextures()
{
	Node * cursor;
	cursor = nodeList;

	useTextures = 1;
	
	if(nodeList)
	{
		while (cursor != NULL)
		{
		if (!cursor->object->IsGarbage())
			{
				cursor->object->UseTextures();
			}
			cursor = cursor->next;
		}
	}
}

/** Disables the use of textures, even if EnableTexture is called in the rendering loop. */
void ObjectCollection::DontUseTextures(){
  Node * cursor;
	cursor = nodeList;

	useTextures = 0;
	
	if(nodeList)
  {
		while (cursor != NULL)
		{
   		if (!cursor->object->IsGarbage())
			{
				cursor->object->DontUseTextures();
			}
			cursor = cursor->next;
		}
  }
}
/** No descriptions */
int ObjectCollection::RemoveObject(Object* object)
{
	Node * node, *currentNode, *previous = NULL;
	int retval = TCL_OK;
	
	if (object)
		retval = TCL_ERROR;
	
	if (nodeList == NULL)
		return TCL_OK;
	
	currentNode = nodeList;
	while (currentNode)
    {
		if (currentNode->object &&
			(currentNode->object->IsGarbage() ||
			(currentNode->object == object)))
		{
			if (ToglWindow::interp)
			{
				char tclstr[200];
				sprintf(tclstr,"removeobj %d", currentNode->object->GetId());
				mainWindow->Tcl_Eval(tclstr);
			}
			if (currentNode->object == object) //found what we were looking for
				retval = TCL_OK;
			else //found trash;
			{
				if (currentNode->object == Object::getSelectedObject())
					Object::setSelectedObject(NULL);
				delete currentNode->object;
			}
			if (previous) //attach back link to next
				previous->next = currentNode->next;
			else
				nodeList = currentNode->next;
			node = currentNode; //get ready to delete the node!
			currentNode = currentNode->next;
			if (last == node)
				last = previous; //changed by mdp 2/6/2008 (was wrong!)
			delete node;
			if (currentNode) //attach forward link to previous
				currentNode->prev = previous;
			else
			{
				if (cursor == currentNode)
					cursor = NULL;
				if (current == currentNode)
					current = NULL;
			}
		}
		else
		{
			previous = currentNode;
			currentNode = currentNode->next;
		}
  }
	if (nodeList == NULL)
	{
		globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
		//mdp 3/11/2003 added this and changed its meaning.
		//The reset checks to see if it should be reset, so we are
		//all set doing this even in non-root object collections.
		mainWindow->getScene3D()->Reset();
		bbox.reset();
	}
	else if (object != NULL)
		globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
	return TCL_OK;
	
}
/** No descriptions */
int ObjectCollection::ReloadObject(Object* object)
{
	Node *currentNode;
	int retval = TCL_OK, changeMade = GL_FALSE, ret;
	Object *newInstance, *oldInstance;

	if (object)
	{
		retval = TCL_ERROR;

		if (object->IsGarbage())
			return TCL_ERROR;
	}
	
	if (nodeList == NULL)
		return TCL_OK;
	
	currentNode = nodeList;
	while (currentNode)
    {
		if (currentNode->object &&
			(!object || (currentNode->object == object)))
		{
			if (currentNode->object == object) //found what we were looking for
				retval = TCL_OK;

			oldInstance = currentNode->object;
			if (oldInstance == Object::getSelectedObject())
				Object::setSelectedObject(NULL);
            if (oldInstance->fullReloadRequired())
            {
                newInstance = LoadObject(oldInstance->GetFilename());
                if (newInstance)
                {
                    RemoveObject(oldInstance);
                    break; //we can't continue the loop, because we've changed the node list
                }
            }
            else
            {
			    newInstance = oldInstance->getRegistrationEntry()->create();
			    newInstance->setParent(this);
			    ret = newInstance->SetFilename(oldInstance->GetFilename());
			    if (ret == 0)
			    {
    //				newInstance->SetDefaultTexture(defaulttextureptr);
				    newInstance->SetCustomTexture(oldInstance->GetCustomTexture());
				    if (oldInstance->isVisible())
					    newInstance->SetVisible();
				    else
					    newInstance->SetInvisible();
				    if (newInstance->Load() == 0)
				    {
					    string label = oldInstance->GetLabel();
					    if (!label.empty())
						    newInstance->SetLabel(label);
					    //newInstance->SetLocalOffset();
					    newInstance->replace(oldInstance);
					    currentNode->object = newInstance;
					    changeMade = GL_TRUE;
				    }
				    else
				    {
					    delete newInstance;
					    cerr << "Error reloading object!\n";
				    }
			    }
			    else
			    {
				    delete newInstance;
				    cerr << "Error finding original file!\n";
			    }
            }
		}
		currentNode = currentNode->next;
    }

	if (changeMade)
		globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
	return retval;
}

Object * ObjectCollection::LoadObject(const string& fn)
{
    Object* object = NULL;
    string ext = extension(bfs::path(fn));

    if (!ext.empty()) //Load the appropriate object based on extension
    {
        if(ext == ".gzx")
        {
            LoadXML(fn);
        }
        else
        {
            object = mainWindow->getScene3D()->LoadObject(fn,NULL);
        }
    }
    if(object)
    {
        AddObject(object);
        return object;
    }
    cerr << "Problem loading " << fn << std::endl;
    return NULL;
}

int ObjectCollection::Reset()
{
  current = nodeList;
  if (current == NULL)
    {
      return (1);
    }
  else
    {
      return (0);
    }
}

Object * ObjectCollection::Next()
{
  if (current == NULL)
    return NULL;
  else
  {
    current = current->next;
    if (current)
			return current->object;
    else
			return NULL;
  }
}

Object * ObjectCollection::GetObjectPtr()
{
  if (current == NULL)
    {
      return (NULL);
    }
  else
    {
      return current->object;
    }
}

Object * ObjectCollection::GetObjectPtr(int id)
{
	Node *cursor = nodeList;
	Object * ret = NULL;

	ret = Object::GetObjectPtr(id);
	
	while(cursor && !ret)
	{
		ret = cursor->object->GetObjectPtr(id);
		cursor = cursor->next;
	}
	
	return ret;
}

Object * ObjectCollection::GetObjectPtr(string l)
{
	Node *cursor = nodeList;
	Object * ret = NULL;


	ret = Object::GetObjectPtr(l);

	while(cursor && !ret)
	{
		ret = cursor->object->GetObjectPtr(l);
		cursor = cursor->next;
	}

	return ret;
}


/*---------------------------------------------------------------------------*/
void ObjectCollection::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
	Node * cursor;

	Object::PoseChanged(whichPose, whatChanged);

	if(whichPose == pose && nodeList)
	{
 		cursor = nodeList;
 		while(cursor)
 		{
 			cursor = cursor->next;
 		}
	}

}

/*--------------------------------------------------------------------------*/
void ObjectCollection::Fuse(Object * object)
{
	Node * cursor;
	if(nodeList)
	{
 		cursor = nodeList;
 		while(cursor)
 		{
 			if(!object)
 			{
 				mainWindow->getScene3D()->Fuse(cursor->object);
 			}
 			if(object)
 				//cerr << "Fuse: " << cursor->object->getRegistrationEntry()->className << " (" << object->getRegistrationEntry()->className << ")\n";
 			cursor->object->Fuse(object);
 			cursor = cursor->next;
 		}
	}
}

/*--------------------------------------------------------------------------*/
/**	This function returns a height summary of the specified area surrounding
*	the passed location, based on the results returned from each child object
*	of this collection.  Defaults to the maximum height (Object::height_max)
*	but can also do Object::height_min and Object::height_average.  Returns
*	-1.0 if a height is not found near this location or if the range doesn't
*	cross any cells in the GUTM.  The area is always a box with the length
*	of twice the rangeMeters parameter on each side.  Right now, the function
*	is very crude, in that it works only with cell values and does not do any
*	interpolation.  Note: height_average weights each child's contribution
*	equally, and may result in undesirable behavior.
*	Function created by mdp 1/3/2006 to support vessel shadow functionality.
*/
double ObjectCollection::GetHeightNear(double x, double y, float rangeMeters, int heightType)
{
	Node * cursor;
	Object *ob;
	float val, dval;
	int count = 0;
	val = invalidHeight;

	if(nodeList)
	{
		for (cursor = nodeList; cursor; cursor = cursor->next)
 		{
			ob = cursor->object;
			dval = ob->GetHeightNear(x, y, rangeMeters, heightType);
			if (dval <= invalidHeight)
				continue;
			if (!count++)
			{
				val = dval;
				continue;
			}
			switch(heightType)
			{
				case height_max:
				default:
					if (dval > val)
						val = dval;
					break;
				case height_min:
					if (dval < val)
						val = dval;
					break;
				case height_average:
					val += dval;
					break;
			}
 		}
	}
	//return the answer based on heightType
	if (count && (heightType == height_average))
		return val / count;
	else
		return val;
}

void ObjectCollection::setHeightExaggeration(GLfloat heightEx)
{
	if(nodeList)
	{
		cursor = nodeList;

		while (cursor != NULL)
		{
			cursor->object->useNewHeightExaggeration(heightEx);
			cursor = cursor->next;
		}
	}

	globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
}
/** No descriptions */
void ObjectCollection::UpdateObjectsUsing(std::shared_ptr<TextureMap> &texture)
{
    cursor = nodeList;
    while (cursor != NULL)
    {
        if (cursor->object->GetCurrentTexture() == texture)
            cursor->object->UpdateTexture();
        cursor = cursor->next;
    }
}
/** No descriptions */
void ObjectCollection::UpdateCustomTextures()
{
  cursor = nodeList;
  while (cursor != NULL)
    {
      cursor->object->UpdateCustomTexture();
      cursor = cursor->next;
    }
}

void ObjectCollection::SaveObject(int id, char * fn)
{
  Node * cursor = nodeList;
	
  Object::SaveObject(id,fn);

  while(cursor)
  {
		cursor->object->SaveObject(id,fn);
		cursor = cursor->next;
  }
}

void ObjectCollection::SaveEditedObjects()
{
	Node * cursor = nodeList;

	while (cursor)
	{
		if (cursor->object->wasEdited())
			cursor->object->save();
		cursor = cursor->next;
    }
}

void ObjectCollection::SetVisible(int id)
{
	Node * cursor = nodeList;

	Object::SetVisible(id);

	if (id != this->id && id!=-1)
	{
		while(cursor != NULL)
		{
			cursor->object->SetVisible(id);
			cursor = cursor->next;
		}
	}
}

void ObjectCollection::SetInvisible(int id)
{
	Node * cursor = nodeList;

	Object::SetInvisible(id);

	if (id != this->id && id!=-1)
	{
		while(cursor != NULL)
		{
			cursor->object->SetInvisible(id);
			cursor = cursor->next;
		}
	}
}

double ObjectCollection::GetHeightAt(double x, double y, Point3d coord, GLboolean secondary)
{
	Node * cursor = nodeList;
	double d,ret;

	ret = Object::GetHeightAt(x,y,coord,secondary);

	while(cursor != NULL)
	{
		d = cursor->object->GetHeightAt(x,y,coord,secondary);
		if(d!=invalidHeight)
			ret = d;
		cursor = cursor->next;
	}
	return ret;
}

void ObjectCollection::PostMenu(int id)
{
	Node * cursor = nodeList;

	Object::PostMenu(id);

	while(cursor != NULL)
	{
		cursor->object->PostMenu(id);
		cursor = cursor->next;
	}
}

void ObjectCollection::SetDefaultTexture(int id)
{
    std::shared_ptr<TextureMap> empty;
    SetCustomTexture(id,empty);
}

void ObjectCollection::SetCustomTexture(int oid, std::shared_ptr<TextureMap> &t)
{
	Node * cursor = nodeList;

	if(oid == id)
	{
		SetCustomTexture(t);
	}

	while(cursor)
	{
		cursor->object->SetCustomTexture(oid,t);
		cursor = cursor->next;
	}
}

void ObjectCollection::SetCustomTexture(std::shared_ptr<TextureMap> &t)
{
	Node * cursor = nodeList;

	Object::SetCustomTexture(t);

	while(cursor)
	{
		cursor->object->SetCustomTexture(t);
		cursor = cursor->next;
	}
}


void ObjectCollection::WriteToScene(string scenepath)
{
	Node * ocursor;
	Object * obj;
	std::shared_ptr<TextureMap> customTexture;

	for (ocursor = nodeList; ocursor; ocursor = ocursor->next)
	{
		obj = ocursor->object;
		customTexture = obj->GetCustomTexture();
		if (obj->GetFilename().empty() || obj->wasEdited())
			obj->save(scenepath);
		string customTextureLabel;
		if (customTexture)
			customTextureLabel = customTexture->GetLabel();
		else
			customTextureLabel = "default";
			
		
		
		stringstream transparency;
		if (obj->getTransparencyLevel())
			transparency << " transparency=\\\"" << obj->getTransparencyLevel()<< "\\\"";

		stringstream tclstr;
		tclstr << "puts $outfile \"\t<Object filename=\\\"" << bfs::path(scenepath)/obj->GetFilename() << "\\\" label=\\\"" <<  obj->GetLabel() << "\\\" texture=\\\"" << customTextureLabel << "\\\"" << transparency.str() << "/>\"";

		mainWindow->Tcl_Eval(tclstr.str().c_str());
	}
}

void ObjectCollection::ResetAllHighRes(Node * start)
{
	if(nodeList)
  {
		if (start)
			cursor = start;
		else
			cursor = nodeList;
		while (cursor != NULL)
		{
			cursor->object->ResetHighRes();
			cursor = cursor->next;
		}
  }
}

int ObjectCollection::IsEmpty()
{
	if(nodeList)
		return 0;
	else
		return 1;
}

string ObjectCollection::GetExtension(int id)
{
	Node * cursor = nodeList;

	while(cursor->object->GetId() != id && cursor->next != NULL)
		cursor = cursor->next;

	if (cursor != NULL)
		return cursor->object->GetSuffix();

	return "";
}

/**
	Opens a .gzx file using an xml parser and if successfull, passes the parsed
	xml tree to LoadXML(xmlDocPtr,xmlNodePtr,char*).
	
	@param  fname The name of the .gzx file to load
	\return The last top level Object loaded from the file, or NULL if no Object was loaded.
	
*/
Object *ObjectCollection::LoadXML(std::string const &fname)
{
    Object * object = NULL;
    bfs::path newpath = bfs::path(fname).parent_path();

    bpt::ptree doc;
    read_xml(fname,doc);

    if(doc.empty())
    {
        std::cerr << "Error parsing " << fname << std::endl;
        return NULL;
    }

    bpt::ptree node = doc.get_child("GZ");
    object = ObjectCollection::LoadXML(node,newpath.string(),gz4d::netcdf::File());

    return object;
}

/** Loads (by looping) textures, objects, collections, scripts and animators from an XML document.
 * @param[in] xml gz4d::Xml instance from which to load.
 * @param[in] cur the current item that is being evaluated.
 * @param[in] path of the directory from which the XML file is loaded.
 * \return Object* - The entire collection of objects
 * \par Purpose: 
 * To process all valid portions of an XML formatted file (i.e. .gzx).
 * \note LoadXMLObject() is used for objects and AddObject() is used for collections of objects.  
 * \sa LoadXMLObject(), AddObject()
 * \author Roland Arsenault (rja)
 * \date 2002
 */
Object * ObjectCollection::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{

    Object * object = NULL;
    string name;
    
    for(bpt::ptree::const_iterator node = xml.begin(); node != xml.end(); ++node)
    {
        if(node->first == "Texture")
            mainWindow->getScene3D()->LoadXMLTexture(node->second,path, ncfile);

        // Object
        if(node->first == "Object")
        {
            object = LoadXMLObject(node->second,path, ncfile);
        }

        // Collection
        if(node->first == "Collection")
        {
            object = new ObjectCollection(GL_TRUE, 0); //instantiate it so the ObjectCollection isn't pickable
            if(object)
            {
                boost::optional<std::string> attribute;
                if(attribute = node->second.get_optional<std::string>("<xmlattr>.label"))
                    object->SetLabel(*attribute);
                boost::optional<float> float_attrib;
                if(float_attrib = node->second.get_optional<float>("<xmlattr>.priority"))
                    object->setPriority(*float_attrib);
                object->LoadXML(node->second,path, ncfile);
                AddObject(object);
            }
        }

                // Scripts
        if(node->first == "Script")
            mainWindow->getScene3D()->LoadXMLScript(node->second,path, ncfile);
        
                // Animators
        if (node->first == "Animator")
            mainWindow->getScene3D()->LoadXMLAnimator(node->second, path, ncfile);
    }

    return object;
}

/** This will load a specific object from an xml file.
 * @param[in] doc a pointer to an XML formatted document.
 * @param[in] cur the current item that is being evaluated.
 * @param[in] path the full path of the XML file to be evaluated.
 * \return Object* - One single object is returned.  If there may be recursion that would otherwise cause an object to be loaded twice, this is returned.
 * \par Purpose: 
 * To process all valid portions of an XML formatted file (i.e. .gzx).
 * \note If it is from a gxz file LoadXML() is used, otherwise Scene3D()::LoadObject() from mainWindow->getScene3D is used.\n
 * AddObject() adds just one object and LoadXMLObject() is <b>NOT</b> used.
 * \sa LoadXML(char*), LoadXML(xmlDocPtr, xmlNodePtr, char*)
 * \author Roland Arsenault (rja)
 * \date 2002
 * \par Important Comments:
 */
Object * ObjectCollection::LoadXMLObject(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object * object = NULL;
    bfs::path fn;
    string ext;
    bool recursionFlag = false;
    
    boost::optional<std::string> attribute;
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.type"))
    {
        object = mainWindow->getScene3D()->NewObject(attribute->c_str());
        if(object)
            object->LoadXML(xml,path, ncfile);
        else
            std::cerr << "Error creating object of type " << *attribute << endl;
    }
    else
    {
        if(attribute = xml.get_optional<std::string>("<xmlattr>.filename"))
        {
            if((*attribute)[0] == '/')
                fn = *attribute;
            else
                fn = bfs::path(path)/(*attribute);
            ext = extension(fn);
            if (!ext.empty()) //Load the appropriate object based on extension
            {
                if(ext == ".gzx")
                {
                    object = LoadXML(fn.string());
                    recursionFlag = true;
                }
                else
                    object = mainWindow->getScene3D()->LoadObject(fn.string(),NULL,NULL,false,this);
                if (object)
                    object->LoadXML(xml,path,ncfile); //We lost the Object:: at some point... this is important so objects don't try to load themselves twice, but only get overridden... why was it removed (comment by mdp 7/28/2009) It was removed to allow overiding attributes in a gzx file, such as: <Object filename="foo.gzx" bar="override something">. If this causes a problem, a derived object's LoadXML should be fixed to support being called multiple times. (rja 2013-10-01)
            }
        }
    }

    if(object && !recursionFlag)
    {
        if (object->getParent())
            object->getParent()->AddObject(object);
        else
            AddObject(object);
    }
    return object;
}

BBox ObjectCollection::GetBoundingBox()
{
	BBox tmp;
	Point3d p = {0.0, 0.0, 0.0};
	
	if(bboxStale)
	{
		cursor = nodeList;
		if(localBBox)
		{
			bbox = *localBBox;
		}
		else if(cursor)
		{
			bbox.set(cursor->object->GetBoundingBox());
			GetPosition(p);
		}
		while (cursor != NULL)
		{
			tmp = cursor->object->GetBoundingBox();
			bbox.merge(tmp);
			cursor = cursor->next;
		}
		bboxStale = 0;
	}
	
	return bbox;
}

void ObjectCollection::setBBoxStale()
{
	bboxStale = 1;
	if(parent)
		parent->setBBoxStale();
}

void ObjectCollection::SaveXML(FILE *out, std::string const &path)
{
	if(out)
	{
		if(parent)
		{
			fprintf(out,"<Collection ");
			SaveXMLattributes(out);
			fprintf(out,">\n");
		}
		SaveXMLChildren(out,path);
		if(parent)
		{
			fprintf(out,"</Collection>\n");
		}
	}
}

void ObjectCollection::SaveXMLChildren(FILE *out, std::string const &path)
{
	Node * cursor = nodeList;
	if(out)
	{
		while(cursor)
		{
			cursor->object->SaveXML(out,path);
			cursor=cursor->next;
		}
	}
}

void ObjectCollection::expandBBoxTime(double t)
{
	if(!localBBox)
	{
		localBBox = new BBox;
	}
	localBBox->expandTime(t);
	setBBoxStale();
}

int ObjectCollection::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    Object * object = NULL;

    if (argc >= 1)
    {
        std::string command = argv[0];
        if (command == "loadobject")
        {
            if (argc == 2)
            {
                bfs::path fn(argv[1]);
                string ext = extension(fn);
                if (!ext.empty()) //Load the appropriate object based on extension
                {
                    if(ext == ".gzx")
                    {
                        LoadXML(fn.string().c_str());
                    }
                    else
                    {
                        object = mainWindow->getScene3D()->LoadObject(fn.string(),NULL);
                    }
                }
                if(object)
                {
                    AddObject(object);
                    if(!object->getAddress())
                    {
                        std::string id = boost::lexical_cast<std::string>(object->GetId());
                        Tcl_SetResult(interp,(char *)(id.c_str()), TCL_VOLATILE);
                    }
                    else
                        Tcl_SetResult(interp,(char *)(object->getAddress()->getAddress().c_str()), TCL_VOLATILE);
                    return TCL_OK;
                }
                else
                {
                    Tcl_AppendResult(interp, "Problem loading ", NULL);
                    Tcl_AppendResult(interp, argv[1], NULL);
                    return TCL_ERROR;
                }
            }
            else
            {
                Tcl_AppendResult(interp, "Usage: loadobject filename", NULL);
                return TCL_ERROR;
            }
        }
        if(command == "newobject")
        {
            if(argc == 3)
            {
                Object * object = Object::createFromClassname(argv[1]);
                if(object)
                {
                    object->SetLabel(argv[2]);
                    AddObject(object);
                    globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
                    Tcl_SetResult(interp, const_cast<char *>(boost::lexical_cast<std::string>(object->GetId()).c_str()), TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            Tcl_AppendResult(interp, "Usage: newobject type label", NULL);
            return TCL_ERROR;
        }
    }
    return Object::perform(argc,argv,interp);
}

void ObjectCollection::getHtmlInfo(stringstream &html, short level)
{
	Object::getHtmlInfo(html, level);
	
	html << "<h" << level << ">ObjectCollection:</h" << level << ">";
	
	for (Node * n = nodeList; n; n = n->next)
	{
		html << "<a href=\"/object/" << n->object->GetId() << "\">" << n->object->GetId() << ": " << n->object->GetLabel() << "</a>";
                if (n->object->isVisible())
                    html << " visible <a href=\"/command/gz/tellObject/" << n->object->GetId() << "/hide\">hide</a><br>";
                else
                    html << " hidden <a href=\"/command/gz/tellObject/" << n->object->GetId() << "/show\">show</a><br>";
	}

}
