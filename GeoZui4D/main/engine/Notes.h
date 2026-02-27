/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#ifndef NOTES_H
#define NOTES_H

#include<gz4d/glo.h>
#include "SDV_types.h"
#include "engine/PosableState.h"
#include "engine/Persistence.h"
#include "interactors/Pickable.h"

class Window3D;

class Notes: public Persistence, public Pickable
{
public:
	Notes(Window3D * w = NULL);
	~Notes();

	//void TogleGUI();
	
	int Add();
	void Delete();
	void Adjust();
	void PublishCurrent();
	void ChangeCurrent(std::string const &s);
	void ChangeCurrent(int i);
	void Activate(std::string const &s);
	void ChangeLabel(const std::string& s);
	void ChangeDescription(std::string const &s);
        void ChangeCode(int n, std::string const &s);
        void Save(std::string const &fname);
        void Load(std::string const &fn);
	void Draw(Window3D * w);
	
        int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);
	void renderPickableObject(Window3D *inWindow);
	int isMoving(){return moving;};
	void move();
	int Preview();

	void ResetCursor();
	double GetNextTime();
	double GetFirstNoteTime();
	double GetLastNoteTime();
	
private:

	struct NoteType
	{
		double time;
		double timeScale;
		std::string description;
		Point3d cowPosition;
		OrientationInfo cowOrientation;
		double scale;
		int index;
		std::string label;
		std::string code[3];
		NoteType * next;
	};

	void CreateNote(NoteType*&n);
	void Snapshot(NoteType * n);
	void Backup();

	Window3D * w3d;
	
	NoteType * notes;  
	NoteType * currentNote;
	NoteType * fromNote;
	NoteType * noteCursor;
	int moving;
	double movingStart;
	int preview;
	int nextid;
	
	
	int picking;
	
};


#endif
