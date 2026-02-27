/* Copyright (c) 2000 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "SDV_types.h"
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class Persistence
{
public:
	Persistence();
	virtual ~Persistence();
	
	//Filename access
	int SetFilename(std::string const &fn);
	std::string const &GetFilename() const;
        std::string GetExtension() const
        {
            return fs::path(filename).extension().string();
        }

        /// Returns filename as std::filesystem::path
        fs::path GetFilenameAsPath() const
        {
            return fs::path(filename);
        }

	//ID
	int GetId();

	//Label
	virtual bool SetLabel(std::string const &l);
	std::string const &GetLabel() const;
	void SanitizeLabel();

	//Version control
	void SetTypeAndVersion(const char *type, int version = 1, int revision = 0);
	const char * GetType() const;
	int GetVersion() const;
	int GetRevision() const;

	//Keyword table
	void SetKeywordTable(const char **ktab);

	//File opening
    int Open(std::string const &fname = std::string());
	int OpenRead(std::string const &fname = "");	// same as Open(), but does not SkipToBegin().
	int ShareWith(Persistence *p);
	int OpenWrite(const std::string& fn = "");

	//File processing
	int SkipToBegin();	// skip to and verify line in file beginning with "BEGIN"
	char **NextLine(GLboolean lookupFirstToken = GL_TRUE,
		GLboolean uppercase = GL_FALSE);
	int Lookup(const char *name, GLboolean reportError = GL_TRUE, GLboolean disregardCase = GL_FALSE) const;
	const char *Keyword(int index) {return keytable[index];}

	void DeleteFields();
	char *ReleaseField(int i);
	int getNumFields();

	int ConvertInt(int &i, int charOffset = 0);
	int ConvertIntAt(int i, int charOffset = 0);
	float ConvertFloat(int &i, int charOffset = 0);
	float ConvertFloatAt(int i, int charOffset = 0);
	double ConvertDouble(int &i, int charOffset = 0);
	void ConvertFloatPoint(Point3f point, int &i, GLboolean reverseXY = GL_FALSE, int charOffset = 0);
	void ConvertDoublePoint(Point3d point, int &i, GLboolean reverseXY = GL_FALSE, int charOffset = 0);
	void ConvertFloatColor(Point4ub color, int &i, int charOffset = 0);
	void ConvertFloatColor3(Color3ub color, int &i, int charOffset = 0);
	void Convert255Color(Point4ub color, int &i, int charOffset = 0);

	//misc.
	static char *strdupp(const char *s); //strdup that uses c++ new operator instead of malloc
	void WriteFloatColor(FILE *file, Point4ub color, GLboolean forceFourthComponent = GL_FALSE);// function to write out color the way ConvertFloatColor read it in
	virtual void SaveXML(FILE * out,const char * path){};

	//file closing
	void Close();
	void CloseRead();		// for symmetry, same as Close()
	void BreakShare();
	void CloseWrite();

	//------Progress bar functions----------
	void createProgress(std::string const &title = std::string());
	void updateProgress(double prog, std::string const &newTitle = std::string());
	void updateTitle(std::string const &title = std::string());
	void killProgress();
	void disableProgress(){progDisabled=1;}

protected:
	FILE *in;		// file pointer for reading
	FILE *out;		// file pointer for writing
	//------Progress bar support variables----------
	static double objProg;
	int progDisabled;

	const char **keytable;	// table of valid keywords to be found in file
	int id;
	bool binaryFile;

private:
	std::string filename;	// filename info for loading
	std::string outfilename;	// filename for saving
	std::string label;
	char ** fields;
	int numFields;

	const char * magicname;     // derived class must store these. Used to check
	int versionno;	//   the file for compatible type, version and revision
	int revisionno;

	int nentries;		// number of entries in keyword table

	int VerifyTypeAndVersion(char *line);
	int VersionOK(char *versionfield) const;

	char *EatWhite(char *str) const;	// skip any leading white space in str
	char **Parse(char *line, int &error, GLboolean lookupFirstToken = GL_TRUE,
		GLboolean reporterror = GL_TRUE,
		GLboolean uppercase = GL_TRUE);
	static int nextid;
};

#endif
