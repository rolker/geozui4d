/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "MainWindow.h"
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <ctype.h>
#include "engine/Persistence.h"
#include "widgets/Sv_Standards.h"
#include <filesystem>
#include "utilities/Utilities.h"

//using namespace std;

extern MainWindow *mainWindow;

int Persistence::nextid = 0; // Assuming that Scene3D's objects gets id 0, since it is the root of the object tree - rja

/*--------------------------------------------------------------------------*/
#define MAX_ENTRIES 20
#define MAX_LINE 100000
#define INVALID_VERSION -99999
#ifdef WIN32
#define strcasecmp(a, b) _stricmp(a, b)
#endif

double Persistence::objProg = -1.0;

/*--------------------------------------------------------------------------*/
Persistence::Persistence()
{
  fields = NULL;

  in = NULL;
  out = NULL;
  
	id = nextid++;

  magicname = NULL;
  versionno = 1;
  revisionno = 0;

  keytable = NULL;
  nentries = 0;

  progDisabled = 0;
  binaryFile = false;
  
}

/*--------------------------------------------------------------------------*/
Persistence::~Persistence()
{

  if(in != NULL)
    fclose(in);
  if(out != NULL)
    fclose(out);

  DeleteFields();
  //  delete magicname;

  //delete []keytable;
  
}

/*--------------------------------------------------------------------------*/
int Persistence::ShareWith(Persistence *p)
{
	filename = p->filename;
	outfilename = p->outfilename;
	label = p->label;

	in = p->in;
	out = p->out;

	return 0;
}

/*--------------------------------------------------------------------------*/
void Persistence::BreakShare()
{
	filename = "";
	outfilename = "";
	label.erase();

	in = NULL;
	out = NULL;
}

/*--------------------------------------------------------------------------*/
void Persistence::DeleteFields()
{
  int i;

  if (!fields)
    return;

  for (i = 0; i < numFields; i++)
    {
      if (fields[i])
	delete[] fields[i];
    }

  delete []fields;
  fields = NULL;
}

/*--------------------------------------------------------------------------*/
char *Persistence::ReleaseField(int i)
{
  char *field = fields[i];

  fields[i] = NULL;
  return field;
}

/*--------------------------------------------------------------------------*/
/**     This function skips through a file to the first line beginning
 *      with the token BEGIN.  It then verifys that this line has the
 *      correct type string identifier and a valid version number.
 *      Returns 0 if found and validated, otherwise returns 1
 */
int Persistence::SkipToBegin()
{
  char instring[MAX_LINE];
  char begin[] = "BEGIN";
  int i = 0;

  // if no valid type names are set for this reader, then skip the search for
  // BEGIN magicname version.revision
  if(magicname == NULL || strlen(magicname) == 0)
    return 0;

  while(fgets(instring, MAX_LINE, in) != NULL)
    {
      if(strlen(instring) >= 5)
	for(i = 0; i < 5 && toupper(instring[i]) == begin[i]; i++);
      if(i == 5) break;
    }

  if(i != 5)
    {
      std::cerr << "Unexpected end of file looking for BEGIN in " << 
	GetFilenameAsPath().filename() << "!" << std::endl;
      return 1;
    }

  return VerifyTypeAndVersion(EatWhite(&(instring[i])));
}

//--------------------------------------------------------------------------
int Persistence::VerifyTypeAndVersion(char *line)
{
  int error;

  Parse(line, error, GL_FALSE);

  error = 0;
  if(fields == NULL || fields[0] == NULL || fields[1] == NULL)
    {
      std::cerr << "Missing file type and/or version after BEGIN";
      error = 1;
    }
  else if(strcmp(fields[0], magicname) != 0)
    {
      std::cerr << "Incorrect file type! Expecting " << magicname;
      error = 1;
    }
  else if(!VersionOK(fields[1]))
    {
      std::cerr << "Incorrect version or revision! Expecting " << versionno << 
	"." << revisionno;
      error = 1;    
    }

  if(error)
    std::cerr << " in " << GetFilenameAsPath().filename() << "!" << std::endl;

  DeleteFields();

  return error;
}

/*--------------------------------------------------------------------------*/
int Persistence::VersionOK(char *versionfield) const
{
  int i;
  int len = strlen(versionfield);
  int version, revision;

  for(i = 0, version = 0; i < len && versionfield[i] != '.'; i++)
    {
      if(isdigit(versionfield[i]))
	version = version * 10 + versionfield[i] - '0';
      else
	version = INVALID_VERSION;
    }

  if(i + 1 >= len)
    revision = 0;
  else
    revision = atoi(&(versionfield[i + 1]));

  return (version < versionno || 
	  (version == versionno && revision <= revisionno));
}

/*--------------------------------------------------------------------------*/
// Scan the input string str, skipping any whitespace characters
// Return pointer to the first non-white character in str, or NULL if none
char *Persistence::EatWhite(char *str) const
{
  int i;

  if(str == NULL)
    return NULL;

  for(i = 0; isspace(str[i]); i++);

  if(str[i] == '\0')
    return NULL;
  else
    return &(str[i]);
}

/*--------------------------------------------------------------------------*/
int Persistence::getNumFields()
{
  if (fields)
    return numFields;
  else
    return 0;
}

/*--------------------------------------------------------------------------*/
// Break the input line into white-space separated fields
// Return a NULL terminated array of the fields on the line using the
// same style as argv in main().  This class will take care of destroying
// the returned array.  Any string that you want to keep will have to be
// "forgotten" using ReleaseField().
char **Persistence::Parse(char *line, int &error, GLboolean lookupFirstToken,
			  GLboolean reporterror, GLboolean uppercase)
{
  char *str, c, *lastQuote;
  char instring[MAX_LINE];
  int i, j, len, tempLen;
	
  error = 0;
  if (fields)
    DeleteFields();
	
  for(numFields = 0, str = EatWhite(line); str != NULL; numFields++, str = EatWhite(str))
    {
      c = str[0];

      if(c == '%')
	break;	// comment follows a % to end of line
		
      if(numFields == MAX_ENTRIES)
	{
	  error = 2;
	  if(reporterror)
	    std::cerr << "Too many fields in a single input file line!" << std::endl;
	  break;
	}
		
      if (c == '"') // capture the quoted string (may cross lines)
	{
	  str++;
	  strcpy(instring, str);
	  str = instring;
	  len = 0;
	  for (lastQuote = strchr(str, '"'); !lastQuote; lastQuote = strchr(str, '"'))
	    {
	      if (feof(in))
		{
		  error = 3;
		  if (reporterror)
		    std::cerr << "Unmatched quote at \"" << instring << std::endl;
		  break;
		}
	      tempLen = strlen(str);
	      len += tempLen;
	      /*
		if (len < MAX_LINE)
		instring[len++] = '\n';
	      */
	      if (len >= MAX_LINE)
		{
		  error = 4;
		  if (reporterror)
		    std::cerr << "Quoted literal too long (greater than " << MAX_LINE
			 << " at \"" << instring << std::endl;
		  break;
		}

	      if (!fgets(&(instring[len]), MAX_LINE - len, in))
		{
		  error = 5;
		  if (reporterror)
		    std::cerr << "Unmatched quote at \"" << instring << std::endl;
		}
	      str += tempLen;
	    }
	  if (error)
	    break;
	  i = (lastQuote - str); // setup i to increment str by right amount
	  len += i;              // to capture more input after string
	  i++;

	  if(fields == NULL)
	    fields = new char*[MAX_ENTRIES + 1];

	  fields[numFields] = new char[len + 1];
	  strncpy(fields[numFields], instring, len);
	  fields[numFields][len] = '\0';
	}
      else
	{
	  for(i = 0; !isspace(str[i]) && str[i] != '%' && str[i] != '\0'; i++);
		
	  if(fields == NULL)
	    fields = new char*[MAX_ENTRIES + 1];
		
	  fields[numFields] = new char[i + 1];
	  for(j = 0; j < i; j++)
	  {
		if (uppercase)
			fields[numFields][j] = toupper(str[j]);
		else
			fields[numFields][j] = str[j];
	  }
	  fields[numFields][j] = '\0';
	}
		
      if(lookupFirstToken && (numFields == 0) && (Lookup(fields[numFields], reporterror) < 0))
	{
	  error = 1;
	  break;
	}
		
      str += i;
    }
	
  if(numFields > 0)
    fields[numFields] = NULL;
	
  return fields;
}

/*--------------------------------------------------------------------------*/
int Persistence::SetFilename(std::string const &fn)
{
    filename = fn;

    if(fn.empty())
        return 1;

    if (label.empty())
        label = fs::path(filename).filename().string();

  return 0;
}

/*--------------------------------------------------------------------------*/
std::string const &Persistence::GetFilename() const
{
  return filename;
}



/*---------------------------------------------------------------------------*/
bool Persistence::SetLabel(std::string const &l)
{
  label = l;
  return true;
}

/*--------------------------------------------------------------------------*/
std::string const &Persistence::GetLabel() const
{
	return label;
}

/*---------------------------------------------------------------------------*/
void Persistence::SetTypeAndVersion(const char *type, int version, int revision)
{
  magicname = type;
  /*
    delete magicname;

    if(type == NULL)
    magicname = NULL;
    else
    {
    magicname = new char[strlen(type) + 1];
    strcpy(magicname, type);
    }
  */
  versionno = version;
  revisionno = revision;
}

/*--------------------------------------------------------------------------*/
const char *Persistence::GetType() const
{
  return magicname;
}

/*--------------------------------------------------------------------------*/
int Persistence::GetVersion() const
{
  return versionno;
}

/*--------------------------------------------------------------------------*/
int Persistence::GetRevision() const
{
  return revisionno;
}

/*---------------------------------------------------------------------------*/
// The keyword table ktab should be an array of strings, terminated by
// either a NULL entry in the table or a zero length string. For example:
// char keywords[] = {"APPLE", "ORANGE", "LEMON", ""};
// In order to leave the option for case-sensitivity in the future,
// all keywords should be upper-case at this point.  This will make
// it easier to fix the code at that time, since we won't have to
// go through upper-casing everything. (mdp 5/23/2001)
//
void Persistence::SetKeywordTable(const char **ktab)
{
  int i;
  //  int j, keylen;

  keytable = ktab;

  for(i = 0; ktab != NULL && ktab[i] != NULL && strlen(ktab[i]) > 0; i++);
  nentries = i;

  /* This was the original code, which made a local copy and
     upper-cased everything. *//*
				 delete []keytable;

				 for(i = 0; ktab != NULL && ktab[i] != NULL && strlen(ktab[i]) > 0; i++);
				 nentries = i;
				 if(nentries > 0)
				 keytable = new char*[nentries];
				 else
				 keytable = NULL;

				 for(i = 0; i < nentries; i++)
				 {
				 keylen = strlen(ktab[i]);
				 keytable[i] = new char[keylen + 1];
				 strcpy(keytable[i], ktab[i]);
				 for(j = 0; j < keylen; j++)
				 keytable[i][j] = toupper(keytable[i][j]);
				 }
			       */
}

/*---------------------------------------------------------------------------*/
int Persistence::Open(std::string const &fname)
{
	int retval = OpenRead(fname);
	if (!retval & !binaryFile)
		return SkipToBegin();
	return retval;
}

/*--------------------------------------------------------------------------*/
int Persistence::OpenRead(std::string const &fname)
{
	if (fields)
		DeleteFields();

	if(!fname.empty())
		SetFilename(fname);

	if(!filename.empty())
	{
		if(binaryFile)
		{
			in = fopen(GetFilename().c_str(), "rb");
		}
		else
		{
			in = fopen(GetFilename().c_str(), "r");
		}
	}

	if(in == NULL)
	{
		if(filename.empty())
			std::cerr << "Load error, no filename specified!" << std::endl;
		else
			std::cerr << "Load error, can't open file: " << GetFilename() << std::endl;
		return 1;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
int Persistence::OpenWrite(std::string const &fn)
{
    if(!fn.empty())
  {
    outfilename = fn;
  }
  else if(outfilename.empty() && filename.empty())
    {
      std::cerr << "Can't save, no filename!" << std::endl;
      return 1;      
    }
  else if(outfilename.empty())
    outfilename = GetFilename();

  if((out = fopen(outfilename.c_str(), "w")) == NULL)
    {
      std::cerr << "Can't save, error opening file: " << outfilename << std::endl;
      return 1;
    }

  return 0;
}

/*---------------------------------------------------------------------------*/
char **Persistence::NextLine(GLboolean lookupFirstToken, GLboolean uppercase)
{
  char instring[MAX_LINE];
  char *inptr;
  int error = 0;
	
  do
    {
      inptr = fgets(instring, MAX_LINE, in); 
      if(inptr != NULL)
	Parse(instring, error, lookupFirstToken, GL_TRUE, uppercase);
    }
  while(inptr != NULL && error == 0 && fields == NULL);
	
  if(inptr == NULL || error)
    DeleteFields();
	
  return fields;
}

/*---------------------------------------------------------------------------*/
int Persistence::Lookup(const char *name, GLboolean reportError, GLboolean disregardCase) const
{
	int i;
	
	if(name == NULL)
		return -1;
	
	if(name[0] == '-' || name[0] == '+' || name[0] == '.' || isdigit(name[0]))
		return 0;
	
	if (disregardCase)
	{
		for(i = 0; i < nentries && (strcasecmp(name, keytable[i]) != 0); i++);
	}
	else
	{
		for(i = 0; i < nentries && (strcmp(name, keytable[i]) != 0); i++);
	}
	
	if(i == nentries)
    {
		i = -1;
		if (reportError)
		{
			std::cerr << "Invalid keyword " << name << " in " << GetFilenameAsPath().parent_path().string() <<
				"!" << std::endl;
		}
    }
	
	return i;
}

/*---------------------------------------------------------------------------*/
int Persistence::ConvertInt(int &i, int charOffset)
{
  if(fields[i] != NULL && i < numFields)
    return(atoi(&fields[i++][charOffset]));
  else
    return 0;
}

/*---------------------------------------------------------------------------*/
int Persistence::ConvertIntAt(int i, int charOffset)
{
  if(fields[i] != NULL && i < numFields)
    return(atoi(&fields[i][charOffset]));
  else
    return 0;
}

/*--------------------------------------------------------------------------*/
float Persistence::ConvertFloat(int &i, int charOffset)
{
  if(fields[i] != NULL && i < numFields)
    return (float)(atof(&fields[i++][charOffset]));
  else
    return 0;
}

/*--------------------------------------------------------------------------*/
float Persistence::ConvertFloatAt(int i, int charOffset)
{
  if(fields[i] != NULL && i < numFields)
    return (float)(atof(&fields[i][charOffset]));
  else
    return 0;
}

/*--------------------------------------------------------------------------*/
double Persistence::ConvertDouble(int &i, int charOffset)
{
  double value;

  if(fields[i] != NULL && i < numFields)
    sscanf(&fields[i++][charOffset], "%lf", &value);
  else
    value = 0;

  return value;
}

/*--------------------------------------------------------------------------*/
void Persistence::ConvertFloatPoint(Point3f point, int &i, GLboolean reverseXY, int charOffset)
{
  int j = 0;

  if (fields[i] && i < numFields)
  {
	  if (reverseXY)
		point[Y] = (float)atof(&fields[i++][charOffset]);
	  else
		point[X] = (float)atof(&fields[i++][charOffset]);
	  j++;
	  if (fields[i])
	  {
		  if (reverseXY)
			point[X] = (float)atof(&fields[i++][charOffset]);
		  else
			point[Y] = (float)atof(&fields[i++][charOffset]);
		  j++;
		  if (fields[i])
		  {
			  point[Z] = (float)atof(&fields[i++][charOffset]);
			  j++;
		  }
	  }
  }

  if(j != 3)
  {
	for(; j < 3; j++)
		point[j] = 0;
  }
}

/*--------------------------------------------------------------------------*/
void Persistence::ConvertDoublePoint(Point3d point, int &i, GLboolean reverseXY, int charOffset)
{
  int j = 0;

  if (fields[i] && i < numFields)
  {
	  if (reverseXY)
		point[Y] = (double)atof(&fields[i++][charOffset]);
	  else
		point[X] = (double)atof(&fields[i++][charOffset]);
	  j++;
	  if (fields[i])
	  {
		  if (reverseXY)
			point[X] = (double)atof(&fields[i++][charOffset]);
		  else
			point[Y] = (double)atof(&fields[i++][charOffset]);
		  j++;
		  if (fields[i])
		  {
			  point[Z] = (double)atof(&fields[i++][charOffset]);
			  j++;
		  }
	  }
  }

  if(j != 3)
  {
	for(; j < 3; j++)
		point[j] = 0;
  }
}

/*--------------------------------------------------------------------------*/
void Persistence::ConvertFloatColor3(Color3ub color, int &i, int charOffset)
{
  int j;

  for(j = 0; j < 3 && fields[i] != NULL && i < numFields; j++)
    color[j] = atoi(&fields[i++][charOffset]);

  if(j != 3)
    {
      for(; j < 3; j++)
	color[j] = 0;
    }
}

/*--------------------------------------------------------------------------*/
void Persistence::ConvertFloatColor(Point4ub color, int &i, int charOffset)
{
  int j;

  for(j = 0; j < 4 && fields[i] != NULL && i < numFields; j++)
    color[j] = int(255 * atof(&fields[i++][charOffset]));

  if(j != 4)
    {
      for(; j < 3; j++)
	color[j] = 0;
      color[3] = 255;
    }
}

/*--------------------------------------------------------------------------*/
void Persistence::WriteFloatColor(FILE *file, Point4ub color, GLboolean forceFourthComponent)
{
	int i;

	for (i = X; i <= Z; i++)
		fprintf(file, " %g", color[i]/255.0);
	if (color[3] != 255 || forceFourthComponent)
		fprintf(file, " %g", color[3]/255.0);
}

/*--------------------------------------------------------------------------*/
void Persistence::Convert255Color(Point4ub color, int &i, int charOffset)
{
  int j;

  for(j = 0; j < 4 && fields[i] != NULL && i < numFields; j++)
    color[j] = atoi(&fields[i++][charOffset]);

  if(j != 4)
    {
      for(; j < 3; j++)
	color[j] = 0;
      color[3] = 255;
    }
}

/*---------------------------------------------------------------------------*/
void Persistence::Close()
{
  if(in != NULL)
  {
    fclose(in);
  }
  in = NULL;
  if (fields)
    DeleteFields();
}

/*--------------------------------------------------------------------------*/
void Persistence::CloseRead()
{
  Close();
}

/*--------------------------------------------------------------------------*/
void Persistence::CloseWrite()
{
  if(out != NULL)
    fclose(out);

  out = NULL;
}

/*---------------------------------------------------------------------------*/
void Persistence::createProgress(std::string const &title)
{
	objProg = 0.0;

	if(!progDisabled)
	{
		mainWindow->Tcl_LinkVar("objProg",&objProg,TCL_LINK_DOUBLE, GL_FALSE);

		std::stringstream tclstr;
		if(!title.empty())
			tclstr << "ProgressDlg .objProgress -maximum 100 -variable objProg -title \"" << title << "\" -parent .";
		else
			if(!GetFilename().empty())
				tclstr << "ProgressDlg .objProgress -maximum 100 -variable objProg -title \"Loading " << GetFilenameAsPath().parent_path() << "...\" -parent .";
			else
				tclstr << "ProgressDlg .objProgress -maximum 100 -variable objProg -title \"Loading...\" -parent .";
		mainWindow->Tcl_Eval(tclstr.str().c_str());
	}
}

/*---------------------------------------------------------------------------*/
void Persistence::updateTitle(std::string const &title)
{
	if (objProg <= 0.0)
		return;

	std::stringstream tclstr;

	if(!title.empty())
		tclstr << "Dialog::configure .objProgress -title \"" << title << "\"";
	else
		if(!GetFilename().empty())
			tclstr << "Dialog::configure .objProgress -title \"Loading " << GetFilenameAsPath().parent_path() << "...\"";
		else
			tclstr << "Dialog::configure .objProgress -title \"Loading...\"";
	mainWindow->Tcl_Eval(tclstr.str().c_str());
}

/*---------------------------------------------------------------------------*/
void Persistence::updateProgress(double prog, std::string const &newTitle)
{
	if(!progDisabled && objProg >= 0.0)
	{
		objProg = prog;
		mainWindow->Tcl_UpdateLinkedVar("objProg");
		if (!newTitle.empty())
			updateTitle(newTitle);
	}
}

/*---------------------------------------------------------------------------*/
void Persistence::killProgress()
{
	if(!progDisabled)
		mainWindow->Tcl_Eval("destroy .objProgress\n");
	objProg = -1.0;
}


/*---------------------------------------------------------------------------*/
int Persistence::GetId()
{
  return (id);
}

/// Version of strdup that uses c++ new operator instead of malloc
char *Persistence::strdupp(const char *s)
{
	char *retstring;

	if (!s)
		return NULL;

	retstring = new char[strlen(s) + 1];
	if (retstring)
		strcpy(retstring, s);
	return retstring;
}

/// Turns dots and spaces in the label to underscores.
void Persistence::SanitizeLabel()
{
	std::string::size_type badChar;
	while ( (badChar = label.find_first_of(". ")) != std::string::npos)
		label[badChar] = '_';
}
