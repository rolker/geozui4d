// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// GeoData.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides some base functionality for the Data*
 *	Objects (DataPoints, DataLines, DataTriangles).  Each has
 *	attributes associated with points or records (temperature,
 *	for instance) mapped to attributes of the points or
 *	records (color, for instance).
 *	This class implements the following responsibilities:
 *		* Read & parse some source data from a file
 *		* Create some structures
 *
 *	Created by mdp 5/24/2001
 */
/*---------------------------------------------------------------------------*/

#include "GZincludes.h"
//#include "MainWindow.h"
//#include "engine/Scene3D.h"
#include "GeoData.h"
#include "widgets/Sv_Standards.h"
#include "engine/TimeControl.h"

extern MainWindow *mainWindow;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------*/
/**	The GeoData class tries to take care of everything sufficiently
 *	common among the Data* classes.  Each derived class should be
 *	sure that keywords begin with GEODATA_NAMES, a macro that lists
 *	all the keywords understood by this class.
 */
GeoData::GeoData(int numAttributes, const char *type,
				const char **keywords, int major, int minor)
{
	int i;

	SetTypeAndVersion(type, major, minor);
	SetKeywordTable(keywords);

	currentRecord = 0;

	numRecords = numColumns = 0;
	sizeFlags = textFlags = FORMAT_NONE;
	
	colorFlags = FORMAT_CONSTANT | COLOR_CODE;
	constantColor.index = convertColorCode('W');
	size = NULL;
	dimensionalSize[X] = NULL;
	dimensionalSize[Y] = NULL;
	dimensionalSize[Z] = NULL;
	text = NULL;
	selectedRecord = -1;
	selectedPoint = -1;
	sizeFactor = 1.0;
	allocatedRecords = 0;
	lastDataSpec = dataSpec = NULL;
	numDataFields = 0;

	dataNames = NULL;
	dataRepository = NULL;

	numAttribs = numAttributes;
	attribToColumn = new int[numAttribs];
	for (i = 0; i < numAttribs; i++)
		attribToColumn[i] = -1;
 
//	isEditableFlag = GL_TRUE;
	canUseInternalHandleFlag = GL_TRUE;
	numDimensions = 0;

	trailingSeconds = 0;
	trailingStartIndex = 0;
	trailingEndIndex = 0;
	timeAttributeIndex = -1;
	timeAttributeOffset = 0;
	timeAttributeScale = 1;
}

/*--------------------------------------------------------------------------*/
GeoData::~GeoData()
{
    unsigned int i;

	if (attribToColumn)
		delete attribToColumn;
	if (size)
		delete size;
	if (dimensionalSize[X])
		delete dimensionalSize[X];
	if (dimensionalSize[Y])
		delete dimensionalSize[Y];
	if (dimensionalSize[Z])
		delete dimensionalSize[Z];
	if (text)
	{
		if (!(textFlags & FORMAT_CONSTANT))
		{
			for (i = 0; i < numRecords; i++)
			{
				if (text[i])
					delete text[i];
			}
		}
		delete text;
	}
	if (dataRepository)
	{
		for (i = 0; i < numDataFields; i++)
		{
			delete []dataRepository[i].data;
			delete dataNames[i];
		}
		delete []dataNames;
		delete []dataRepository;
	}
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
void GeoData::ResetHighRes()
{
	doneFlag = currentRecord = 0;
}

/*---------------------------------------------------------------------------*/
/**	Display info about the selected point in the info window on the GUI.
 */
void GeoData::ShowInfo(GLuint *names, int numNames)
{
	Scene3D *scene;
	char value[10];

	scene = mainWindow->getScene3D();

	if (isEditableFlag && selectedRecord >= 0)
	{
		if (numDimensions >= 2)
		{
			sprintf(value, "%d", selectedRecord);
			scene->appendInfo("Selected line: ");
			scene->appendInfo(value, GL_TRUE);
		}
		sprintf(value, "%d", selectedPoint);
		scene->appendInfo("Selected point: ");
		scene->appendInfo(value, GL_TRUE);
		if (!text)
			scene->appendInfo("--- Generic Info:---\n");
	}

	if (text)
	{
		if (numNames)
		{
			scene->appendInfo(text[names[0]], GL_TRUE);
			scene->appendInfo("--- Generic Info:---\n");
		}
		else if (numRecords == 1)
		{
			scene->appendInfo(text[0], GL_TRUE);
			scene->appendInfo("--- Generic Info:---\n");
		}
	}

	Object::ShowInfo(names, numNames);
}

/*--------------------------------------------------------------------------*/
int GeoData::assignColumn(int &i, int attrib, int field, int modifier)
{
	int val;

	val = ConvertInt(i) - 1;
	if (val < 0 || (numColumns && val >= numColumns)) //allow it if N_COLUMNS appears later (probably automatically generated anyway)
	{
            std::cerr << "Invalid column " << (val + 1) << " in " << Keyword(field);
		if (modifier >= 0)
                    std::cerr << " " << Keyword(modifier);
                std::cerr << ".  Must be between 1 and N_COLUMNS (" << numColumns << ")." << std::endl;
		return 1;
	}
	attribToColumn[attrib] = val;
	return 0;
}

/*--------------------------------------------------------------------------*/
int GeoData::processField(int option, char **Field)
{
	char *currentField;
        unsigned int j, nFields;
	DataSpec *currentSpec;

	int i = 1;
	switch(option)
	{
		case N_RECORDS:
			allocatedRecords = numRecords = ConvertInt(i);
			//cerr << "Discovered that I need to read " << numRecords << " records\n";
			break;
		case N_COLUMNS:
			numColumns = ConvertInt(i);
			break;
		case XYZ:
			attribToColumn[ATTRIB_X] = 0;
			attribToColumn[ATTRIB_Y] = 1;
			attribToColumn[ATTRIB_Z] = 2;
			break;
		case XYZ_COLUMNS:
			for (j = ATTRIB_X; j <= ATTRIB_Z; j++)
			{
				if (assignColumn(i, j, XYZ_COLUMNS) != 0)
					return 1;
			}
			break;
		case COLOR:
			switch (Lookup(Field[i++]))
			{
				case ATTRIBUTE:
					colorFlags = COLOR_ATTRIBUTE | FORMAT_BY_COLUMN | COLOR_AS_BYTES;
					if (assignColumn(i, ATTRIB_COLOR, COLOR, ATTRIBUTE) != 0)
						return 1;
					if (Field[i] && strcmp(Field[i], "AS_HEIGHT") == 0)
						colorFlags |= COLOR_AS_HEIGHT;
					break;
				case RGB_COLUMNS:
					colorFlags = COLOR_RGB | FORMAT_BY_COLUMN;
					for (j = ATTRIB_R; j <= ATTRIB_B; j++)
					{
						if (assignColumn(i, j, COLOR, RGB_COLUMNS) != 0)
							return 1;
					}
					break;
				case CODE:
					colorFlags = COLOR_CODE | FORMAT_BY_COLUMN;
					if (assignColumn(i, ATTRIB_COLOR, COLOR, CODE) != 0)
						return 1;
					break;
				case TEXTURE:
					colorFlags = COLOR_TEXTURE;
					break;
				case TEXTURE_COLUMNS:
					colorFlags = COLOR_TEXTURE | FORMAT_BY_COLUMN;
					if (assignColumn(i, ATTRIB_TEXTURE_S, COLOR, TEXTURE_COLUMNS) != 0)
						return 1;
					if (Field[i])
					{
						if (assignColumn(i, ATTRIB_TEXTURE_T, COLOR, TEXTURE_COLUMNS) != 0)
							return 1;
						colorFlags |= COLOR_TEXTURE_T;
					}
					break;
				case PER_RECORD:
					colorFlags = FORMAT_BY_RECORD;
					switch (Lookup(Field[i++]))
					{
						case ATTRIBUTE:
							colorFlags |= COLOR_ATTRIBUTE | COLOR_AS_BYTES;
							break;
						case RGB_COLUMNS:
							colorFlags |= COLOR_RGB;
							break;
						case CODE:
							colorFlags |= COLOR_CODE;
							break;
						default:
                                                    std::cerr << "Invalid COLOR PER_RECORD argument: "
                                                            << Field[i - 1] << ".  Must be ATTRIBUTE, RGB_COLUMNS, or CODE." << std::endl;
							//fall through
						case -1:
							return 1;
							break;
					}
					break;
				case CONSTANT:
					colorFlags = FORMAT_CONSTANT;
					switch (Lookup(Field[i++]))
					{
						case RGB:
							colorFlags |= COLOR_RGB;
							ConvertFloatPoint(constantColor.rgb, i);
							break;
						case CODE:
							colorFlags |= COLOR_CODE;
							constantColor.index = convertColorCode(Field[i][0]);
							break;
						default:
                                                    std::cerr << "Invalid COLOR CONSTANT argument: "
                                                            << Field[i - 1] << ".  Must be RGB or CODE." << std::endl;
							//fall through
						case -1:
							return 1;
							break;
					}
					break;
				default:
                                    std::cerr << "Invalid COLOR argument: "
						 << Field[i - 1]
                                            << ".  Must be ATTRIBUTE, RGB_COLUMNS, CODE, TEXTURE, TEXTURE_COLUMNS or PER_RECORD." << std::endl;
					//fall through
				case -1:
					return 1;
					break;
			}
			break;
		case SIZE:
			switch (Lookup(Field[i++]))
			{
				case CONSTANT:
					sizeFlags = FORMAT_CONSTANT;
					sizeFactor = ConvertFloat(i);
					break;
				case COLUMNS:
					currentField = Field[i++];
					for (j = 0; j < 5 && currentField[j]; j++)
					{
						sizeFlags = FORMAT_DIMENSION;
						switch (currentField[j])
						{
							case 'C':
								sizeFlags |= FORMAT_BY_COLUMN;
								size = new float[numRecords];
								if (assignColumn(i, ATTRIB_SIZE, SIZE, COLUMNS) != 0)
									return 1;
								break;
							case 'K':
								sizeFactor = ConvertFloat(i);
								break;
							case 'X':
								dimensionalSize[X] = new float[numRecords];
								if (assignColumn(i, ATTRIB_X_SIZE, SIZE, COLUMNS) != 0)
									return 1;
								break;
							case 'Y':
								dimensionalSize[Y] = new float[numRecords];
								if (assignColumn(i, ATTRIB_Y_SIZE, SIZE, COLUMNS) != 0)
									return 1;
								break;
							case 'Z':
								dimensionalSize[Z] = new float[numRecords];
								if (assignColumn(i, ATTRIB_Z_SIZE, SIZE, COLUMNS) != 0)
									return 1;
								break;
							default:
                                                            std::cerr << "Invalid SIZE dimension: "
									 << currentField[j]
                                                                    << ".  Must be X, Y, or Z." << std::endl;
								return 1;
								break;
						}
					}
					break;
				case COLUMN:
					sizeFlags = FORMAT_BY_COLUMN;
					size = new float[numRecords];
					if (assignColumn(i, ATTRIB_SIZE, SIZE, COLUMN) != 0)
						return 1;
					if (i < getNumFields())
						sizeFactor = ConvertFloat(i);
					break;
				case PER_RECORD:
					sizeFlags = FORMAT_BY_RECORD;
					size = new float[numRecords];
					break;
				default:
                                    std::cerr << "Invalid SIZE argument: "
						 << Field[i - 1]
                                            << ".  Must be CONSTANT, COLUMN, or PER_RECORD." << std::endl;
					//fall through
				case -1:
					return 1;
					break;
			}
			break;
		case TEXT:
			switch (Lookup(Field[i++]))
			{
				case NONE:
					textFlags = FORMAT_NONE;
					break;
				case COLUMN:
					textFlags = FORMAT_BY_COLUMN;
					text = new char*[numRecords];
					if (assignColumn(i, ATTRIB_TEXT, TEXT, COLUMN) != 0)
						return 1;
					break;
				case PER_RECORD:
					textFlags = FORMAT_BY_RECORD;
					text = new char*[numRecords];
					break;
				default:
                                    std::cerr << "Invalid TEXT argument: "
						 << Field[i - 1]
                                            << ".  Must be NONE, COLUMN, or PER_RECORD." << std::endl;
					//fall through
				case -1:
					return 1;
					break;
			}
			if (text)
			{
				for (j = 0; j < numRecords; j++)
					text[j] = NULL;
			}
			break;
		case DATA:
			nFields = getNumFields();
			currentSpec = new DataSpec;
			switch (Lookup(Field[i++]))
			{
				case COLUMN:
					currentSpec->format = FORMAT_BY_COLUMN;
					if (assignColumn(i, ATTRIB_DATA, DATA, COLUMN) != 0)
						return 1;
					currentSpec->column = attribToColumn[ATTRIB_DATA];
					break;
				case PER_RECORD:
					currentSpec->format = FORMAT_BY_RECORD;
					break;
				default:
                                    std::cerr << "Invalid DATA argument: " << Field[i - 1]
                                            << ".  Must be COLUMN or PER_RECORD." << std::endl;
					//fall through
				case -1:
					delete currentSpec;
					return 1;
					break;
			}
			if (i < (int)nFields)
				currentSpec->name = ReleaseField(i++);
			else
			{
                            std::cerr << "Missing name for DATA line." << std::endl;
				delete currentSpec;
				return -1;
			}
			if (i < (int)nFields)
				currentSpec->type = Lookup(Field[i++]);
			else
				currentSpec->type = AS_NUMBER;
			currentSpec->next = NULL;
			if (!lastDataSpec)
				dataSpec = lastDataSpec = currentSpec;
			else
			{
				lastDataSpec->next = currentSpec;
				lastDataSpec = currentSpec;
			}
			numDataFields++;
			break;
		case REVERSED_XY:
                    std::cerr << Field[0] << " keyword is not supported because of the alternate syntax available." << std::endl;
			return 1;
			break;
		default:
			return Object::processField(option, Field);
			break;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
/**	The GeoData writeTo function takes care of writing out the non-specific
 *	header information appropriately.  It assumes numColumns is set to the
 *	number of child-specific columns that are to be written out as of the
 *	call to this function, and calculates how many more columns are necessary
 *	to build.  The child is to write itself out as compactly as possible,
 *	always using XYZ layout (X, Y, and Z in the first three columns of data).
 *	The order of columns after the XYZ columns is always child-specific first,
 *	then color, size, and text, and finally user-defined data columns.  Each
 *	child must take care of writing out the N_RECORDS line since it is so
 *	vital to being at or near the top.
 */
int GeoData::writeTo(FILE *file)
{
    unsigned int i;

	//easy lines
	fprintf(file, "%s\n", Keyword(XYZ));
	numColumns += 3; //for XYZ

	//color line
	fprintf(file, "%s ", Keyword(COLOR));
	if (colorFlags & FORMAT_BY_RECORD)
	{
		fprintf(file, "%s ", Keyword(PER_RECORD));
		if (colorFlags & COLOR_ATTRIBUTE)
			fprintf(file, "%s\n", Keyword(ATTRIBUTE));
		else if (colorFlags & COLOR_RGB)
			fprintf(file, "%s\n", Keyword(RGB_COLUMNS));
		else if (colorFlags & COLOR_CODE)
			fprintf(file, "%s\n", Keyword(CODE));
	}
	if (colorFlags & FORMAT_CONSTANT)
	{
		fprintf(file, "%s ", Keyword(CONSTANT));
		if (colorFlags & COLOR_RGB)
			fprintf(file, "%s %g %g %g\n", Keyword(RGB_COLUMNS), constantColor.rgb[0], constantColor.rgb[1], constantColor.rgb[2]);
		else if (colorFlags & COLOR_CODE)
			fprintf(file, "%s %c\n", Keyword(CODE), convertColorCode(constantColor.index));
	}
	else if (colorFlags & COLOR_ATTRIBUTE)
	{
		fprintf(file, "%s ", Keyword(ATTRIBUTE));
		if (attribToColumn[ATTRIB_COLOR] == attribToColumn[ATTRIB_Z])
			fprintf(file, "%d", 3);
		else if (attribToColumn[ATTRIB_COLOR] == attribToColumn[ATTRIB_Y])
			fprintf(file, "%d", 2);
		else if (attribToColumn[ATTRIB_COLOR] == attribToColumn[ATTRIB_X])
			fprintf(file, "%d", 1);
		else
			fprintf(file, "%d", ++numColumns);
		if (colorFlags & COLOR_AS_HEIGHT)
			fprintf(file, " AS_HEIGHT\n");
		else
			fprintf(file, "\n");
	}
	else if (colorFlags & COLOR_CODE) //must come before RGB_COLUMNS
		fprintf(file, "%s %d\n", Keyword(CODE), ++numColumns);
	else if (colorFlags & RGB_COLUMNS)
	{
		fprintf(file, "%s %d %d %d\n", Keyword(RGB_COLUMNS), numColumns + 1, numColumns + 2, numColumns + 3); //don't use ++ here
		numColumns += 3;
	}
	else if (colorFlags & COLOR_TEXTURE)
	{
		if (colorFlags & FORMAT_BY_COLUMN)
		{
			fprintf(file, "%s %d", Keyword(TEXTURE_COLUMNS), ++numColumns);
			if (colorFlags & COLOR_TEXTURE_T)
				fprintf(file, " %d", ++numColumns);
			fprintf(file, "\n");
		}
		else
			fprintf(file, "%s\n", Keyword(TEXTURE));
	}
	else
	{
            std::cerr << "Don't know how to save color information properly!" << std::endl;
		fprintf(file, "%s %s %c\n", Keyword(CONSTANT), Keyword(CODE), convertColorCode(DEFAULT_COLOR));
	}

	//size line
	fprintf(file, "%s ", Keyword(SIZE));
	if (sizeFlags & FORMAT_DIMENSION)
	{
		fprintf(file, "%s ", Keyword(COLUMNS));
		if (attribToColumn[ATTRIB_X_SIZE] >= 0)
			fprintf(file, "X");
		if (attribToColumn[ATTRIB_Y_SIZE] >= 0)
			fprintf(file, "Y");
		if (attribToColumn[ATTRIB_Z_SIZE] >= 0)
			fprintf(file, "Z");
		if (attribToColumn[ATTRIB_SIZE] >= 0)
			fprintf(file, "C");
		fprintf(file, "K");
		if (attribToColumn[ATTRIB_X_SIZE] >= 0)
			fprintf(file, " %d", ++numColumns);
		if (attribToColumn[ATTRIB_Y_SIZE] >= 0)
			fprintf(file, " %d", ++numColumns);
		if (attribToColumn[ATTRIB_Z_SIZE] >= 0)
			fprintf(file, " %d", ++numColumns);
		if (attribToColumn[ATTRIB_SIZE] >= 0)
			fprintf(file, " %d", ++numColumns);
		fprintf(file, " %g\n", sizeFactor);
	}
	else if (sizeFlags & FORMAT_BY_COLUMN)
		fprintf(file, "%s %d %g\n", Keyword(COLUMN), ++numColumns, sizeFactor);
	else if (sizeFlags & FORMAT_BY_RECORD)
		fprintf(file, "%s\n", Keyword(PER_RECORD));
	else
	{
		if (!(sizeFlags & FORMAT_CONSTANT))
                    std::cerr << "Don't know how to save size information properly!" << std::endl;
		fprintf(file, "%s %g\n", Keyword(CONSTANT), sizeFactor);
	}

	//text line
	fprintf(file, "%s ", Keyword(TEXT));
	if (textFlags & FORMAT_BY_COLUMN)
		fprintf(file, "%s %d\n", Keyword(COLUMN), ++numColumns);
	else if (textFlags & FORMAT_BY_RECORD)
		fprintf(file, "%s\n", Keyword(PER_RECORD));
	else
		fprintf(file, "%s\n", Keyword(NONE));

	//data lines
	for (i = 0; i < numDataFields; i++)
	{
		fprintf(file, "%s ", Keyword(DATA));
		if (dataRepository[i].format & FORMAT_BY_COLUMN)
			fprintf(file, "%s %d ", Keyword(COLUMN), ++numColumns);
		else
			fprintf(file, "%s ", Keyword(PER_RECORD));
		fprintf(file, "\"%s\"", dataNames[i]);
		if (dataRepository[i].type == SmartMemory::String)
			fprintf(file, " %s", Keyword(AS_STRING));
		else
			fprintf(file, " %s", Keyword(AS_NUMBER));
		fprintf(file, "\n");
	}

	//numColumns
	fprintf(file, "%s %d\n", Keyword(N_COLUMNS), numColumns);

	return Object::writeTo(file);
}

/*--------------------------------------------------------------------------*/
int GeoData::convertColorCode(char code)
{
	int color;

	switch(code)
	{
		case 'K': color = BLACK; break;//follows Skip's Matlab convention
		case 'D': color = DARK_GREEN; break;
		case 'G': color = GREEN; break;
		case 'C': color = CYAN; break;
		case 'B': color = BLUE; break;
		case 'E': color = ELEPHANT_GREY; break;
		case 'M': color = MAGENTA; break;
		case 'A': color = AUBURN; break;
		case 'P': color = PERIWINKLE; break;
		case 'Y': color = YELLOW; break;
		case 'R': color = RED; break;
		case 'W': color = WHITE; break;//follows Skip's Matlab convention
		default:
			color = DEFAULT_COLOR;
			break;
	}

	return color;
}

/*---------------------------------------------------------------------------*/
/**	We could convert to a color distance and find the nearest one, but this
 *	implementation looks for exact matches, assinging 'P' to anything it
 *	doesn't know.
 */
int GeoData::convertRGBToColorCode(float *color)
{
	int i, start, end, middle;

	middle = NUM_COLOR_CODES / 2;
	if (color[0] >= palette[middle][0]) //cut our loop in half
	{
		start = middle;
		end = NUM_COLOR_CODES;
	}
	else
	{
		start = 0;
		end = middle;
	}
	for (i = start; i < end; i++)
	{
		if (color[0] == palette[i][0] &&
			color[1] == palette[i][1] &&
			color[2] == palette[i][2])
			return i;
	}
	return -1;
}

/*--------------------------------------------------------------------------*/
char GeoData::convertColorCode(int code)
{
	char color;

	switch(code)
	{
		case BLACK:			color = 'K'; break;
		case DARK_GREEN:	color = 'D'; break;
		case GREEN:			color = 'G'; break;
		case CYAN:			color = 'C'; break;
		case BLUE:			color = 'B'; break;
		case ELEPHANT_GREY:	color = 'E'; break;
		case MAGENTA:		color = 'M'; break;
		case AUBURN:		color = 'A'; break;
		case PERIWINKLE:	color = 'P'; break;
		case YELLOW:		color = 'Y'; break;
		case RED:			color = 'R'; break;
		case WHITE:			color = 'W'; break;
		default:
			color = 'P'; //Periwinkle
			break;
	}

	return color;
}

/*---------------------------------------------------------------------------*/
/**	Converts dataSpec list to basic dataRepository array, deleting the dataSpec
 *	list along the way.
 */
void GeoData::convertDataFields()
{
    unsigned int i;
	SmartMemory::IndexType type;

	dataRepository = new DataRepository[numDataFields];
	dataNames = new char *[numDataFields];

	lastDataSpec = dataSpec;
	for (i = 0; (i < numDataFields) && lastDataSpec; i++)
	{
		dataNames[i] = dataSpec->name;
		if (dataSpec->type == AS_STRING)
			type = SmartMemory::String;
		else if (dataSpec->type == AS_EXPONENTIAL)
			type = (SmartMemory::IndexType)(SmartMemory::Float | TAKE_LG);
		else if (dataSpec->type == DIFFERENTIATE)
			type = (SmartMemory::IndexType)(SmartMemory::Float | TAKE_DIFFERENTIAL);
		else if (dataSpec->type == DOUBLE_DIFFERENTIATE)
			type = (SmartMemory::IndexType)(SmartMemory::Float | TAKE_DOUBLE_DIFFERENTIAL);
		else
		{
			type = SmartMemory::Float;
			if (dataSpec->type == AS_OFFSET)
				type = (SmartMemory::IndexType)(type | TREAT_AS_OFFSET);
		}
		dataRepository[i].type = type;
		dataRepository[i].column = dataSpec->column;
		dataRepository[i].format = dataSpec->format;
		if (numDimensions == 1 || dataSpec->format == FORMAT_BY_RECORD)
		{
			dataRepository[i].data = new SmartArray[1];
			dataRepository[i].data[0].init(numRecords, (SmartMemory::IndexType)(type & ~(TAKE_LG | TREAT_AS_OFFSET)));
		}
		else
			dataRepository[i].data = new SmartArray[numRecords];
		dataSpec = dataSpec->next;
		delete lastDataSpec;
		lastDataSpec = dataSpec;
	}
}

/*---------------------------------------------------------------------------*/
/**	Converts textual input from the current line of a file to info in the
 *	passed DataRepository.
 */
void GeoData::convertData(char **fields, struct DataRepository *drep, int i1, int i2, int column)
{
	int type;
	float val;

	//we could just do this with the string set, but then things would
	//crash if someone put a non-string field in twice.  As it is, it
	//would still crash if a string field is put in twice, but I don't
	//see that happening as often.
	type = drep->type;
	if (type == SmartMemory::String)
		drep->data[i1][i2].set(ReleaseField(column));
	else
	{
		if (type & TAKE_LG) //convert to normal float
			drep->data[i1][i2].set((float)(log(ConvertFloatAt(column)) / log(2.0)));
		else if (type & TREAT_AS_OFFSET) //add to previous, or look for '='
		{
			if (fields[column][0] == '=') //base
				val = ConvertFloatAt(column, 1);
			else //true offset
			{
				val = ConvertFloatAt(column);
				if (i2 > 0)
					val += drep->data[i1][i2 - 1].asFloat();
			}
			drep->data[i1][i2].set(val);
		}
		else
			drep->data[i1][i2].set(ConvertFloatAt(column));
	}
}

/*---------------------------------------------------------------------------*/
void GeoData::setIndexedAttributeOrdering(unsigned int &numAttributes, const char **&attributeNames,
										  int *names)
{
    unsigned int i, nameIndex, j;

	for (i = 0; names[i] >= 0; i++); //find -1 terminator

	numAttributes = i + numDataFields - 1;
	attributeNames = new const char*[numAttributes];
	for (i = nameIndex = 0; names[i] >= 0; i++, nameIndex++)
	{
		if (names[i] != DATA)
			attributeNames[nameIndex] = Keyword(names[i]);
		else
		{
			for (j = 0; j < numDataFields; j++, nameIndex++)
				attributeNames[nameIndex] = dataNames[j];
		}
	}
}

/*---------------------------------------------------------------------------*/
void GeoData::showTrailing(float seconds)
{
	trailingSeconds = seconds;
}

/*---------------------------------------------------------------------------*/
void GeoData::resetTimeAttribute(int timeAttribute)
{
	timeAttributeIndex = timeAttribute - Object::getNumAttributes();
	if (timeAttributeIndex < 0 || timeAttributeIndex >= (int)numDataFields)
		timeAttributeIndex = -1;
}

/*---------------------------------------------------------------------------*/
void GeoData::setTimeAttributeOffset(double seconds)
{
	timeAttributeOffset = seconds;
}

/*---------------------------------------------------------------------------*/
void GeoData::setTimeAttributeScale(double scale)
{
	timeAttributeScale = scale;
}

/*---------------------------------------------------------------------------*/
void GeoData::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
	if ((colorFlags & COLOR_AS_HEIGHT) && (whatChanged & ChangeTranslateZMask))
		UpdateTexture();
	Object::PoseChanged(whichPose, whatChanged);
}

/*---------------------------------------------------------------------------*/
/**	Trigger the appropriate event when an object is selected.
 */
int GeoData::pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
	int numNames = names[0];

	if (pickType == (PICK_CLICK1) && !selectedFlag)
	{
		setSelectedObject(this);
		if (isEditableFlag)
		{
			if (numDimensions >= 2)
				selectedRecord = names[1];
			else
				selectedRecord = -1;
			if (numNames >= 2)
				selectedPoint = names[2];
			else
			{
				selectedPoint = -1;
				useInternalHandle(GL_FALSE);
			}
		}
		ShowInfo(&names[1], numNames);
		highLevelEventMediator->Trigger(objectSelectedInputType->value, pose, (void*)selectedRecord);
		return GL_TRUE;
	}
	else
	{
		if (pickType == (PICK_PRESS1) && isEditableFlag)
		{
			if (numNames >= 2)
				selectedRecord = names[2];
			else
				selectedRecord = -1;
			if (numNames >= 3)
				selectedPoint = names[3];
			else
			{
				selectedPoint = -1;
				useInternalHandle(GL_FALSE);
			}
			ShowInfo(&names[1], numNames);
			highLevelEventMediator->Trigger(objectSelectedInputType->value, pose, (void*)selectedRecord);
		}

		return Object::pickAction(pickType, inputInfo, names, hits);
	}
}

/*---------------------------------------------------------------------------*/
#define EMPTY()
#define COPY_RECORD(source, sourceIndex, target, targetIndex) \
{	int dim;\
\
	if (sizeFlags & FORMAT_BY_RECORD_OR_COLUMN)\
		target##size[targetIndex] = source##size[sourceIndex];\
	if (sizeFlags & FORMAT_DIMENSION)\
	{\
		for (dim = X; dim <= Z; dim++)\
		{\
			target##dimensionalSize[dim][targetIndex] = source##dimensionalSize[dim][sourceIndex];\
		}\
	}\
	if (textFlags & FORMAT_BY_RECORD_OR_COLUMN)\
	{\
		target##text[targetIndex] = source##text[sourceIndex];\
	}\
}

/*---------------------------------------------------------------------------*/
/**	Insert a record at the passed index.
 */
void GeoData::insertRecord(unsigned int index)
{
	float *new_size = NULL, *new_dimensionalSize[3];
	char **new_text = NULL;
	SmartArray *data, *new_data;
	unsigned int newLength, insertPoint, i, j;

	if (index < numRecords)
	{
		newLength = numRecords + 1;
		insertPoint = index;
	}
	else
	{
		newLength = index + 1;
		insertPoint = numRecords;
	}

	//create new record arrays
	if (newLength > allocatedRecords)
	{
		//size
		if (sizeFlags & FORMAT_BY_RECORD_OR_COLUMN)
			new_size = new float[newLength];
		if (sizeFlags & FORMAT_DIMENSION)
		{
			for (int dim = X; dim <= Z; dim++)
			{
				if (dimensionalSize[dim])
					new_dimensionalSize[dim] = new float[newLength];
			}
		}
		//text
		if (textFlags & FORMAT_BY_RECORD_OR_COLUMN)
			new_text = new char*[newLength];
		for (i = 0; i < insertPoint; i++)
//#ifdef WIN32
#ifdef MSVC
			COPY_RECORD(EMPTY(), i, new_, i);
#else
			COPY_RECORD(, i, new_, i);
#endif
	}
	else
	{
		new_size = size;
		new_dimensionalSize[X] = dimensionalSize[X];
		new_dimensionalSize[Y] = dimensionalSize[Y];
		new_dimensionalSize[Z] = dimensionalSize[Z];
		new_text = text;
	}

	//copy everything past insertion point
	for (i = numRecords; i > insertPoint; i--)
//#ifdef WIN32
#ifdef MSVC
		COPY_RECORD(EMPTY(), i - 1, new_, i);
#else
		COPY_RECORD(, i - 1, new_, i);
#endif

	//insertion in middle or at end
	if (sizeFlags & FORMAT_BY_RECORD_OR_COLUMN)
		new_size[insertPoint] = 1.0;
	if (sizeFlags & FORMAT_DIMENSION)
	{
		for (int dim = X; dim <= Z; dim++)
		{
			if (new_dimensionalSize[dim])
				new_dimensionalSize[dim][insertPoint] = 1.0;
		}
	}
	if (textFlags & FORMAT_BY_RECORD_OR_COLUMN)
		new_text[insertPoint] = NULL;

	//insertion past end
	for (i = numRecords + 1; i < newLength; i++)
		COPY_RECORD(new_, insertPoint, new_, i);
	
	//reassign & clean up
	if (newLength > allocatedRecords)
	{
		if (sizeFlags & FORMAT_BY_RECORD_OR_COLUMN)
		{
			delete [] size;
			size = new_size;
		}
		if (sizeFlags & FORMAT_DIMENSION)
		{
                    for (int dim = X; dim <= Z; dim++)
			{
				if (dimensionalSize[dim])
				{
					delete [] dimensionalSize[dim];
					dimensionalSize[dim] = new_dimensionalSize[dim];
				}
			}
		}
		if (textFlags & FORMAT_BY_RECORD_OR_COLUMN)
		{
			delete [] text;
			text = new_text;
		}
	}

	//data
	for (i = 0; i < numDataFields; i++)
	{
		if (dataRepository[i].format & FORMAT_BY_COLUMN)
		{
			if (newLength > allocatedRecords)
			{
				data = dataRepository[i].data;
				new_data = new SmartArray[newLength];
				//copy to insertion point
				for (j = 0; j < insertPoint; j++)
					new_data[j] = data[j];
			}
			else
				data = new_data = dataRepository[i].data;

			//copy everything past insertion point
			for (j = numRecords; j > insertPoint; j--)
				new_data[j] = data[j - 1];

			//insertion in middle or at end
			new_data[insertPoint].init(2, dataRepository[i].type);

			//insertion past end
			for (j = numRecords + 1; j < newLength; j++)
				new_data[insertPoint].init(2, dataRepository[i].type);

			//reassign & clean up
			if (newLength > allocatedRecords)
			{
				delete [] data;
				dataRepository[i].data = new_data;
			}
		}
		else
			dataRepository[i].data->insertPosition(index);
	}

	if (selectedRecord >= (int)index)
		selectedRecord++;

	numRecords = newLength;
	if (allocatedRecords < numRecords)
		allocatedRecords = numRecords;
}

/*---------------------------------------------------------------------------*/
/**	Delete the record at the passed index.
 */
void GeoData::deleteRecord(unsigned int index)
{
    unsigned int i, j;
	SmartArray *data;

	//delete text at current position
	if (text)
		delete text[index];

	//bump everything down
	for (i = index + 1; i < numRecords; i++)
//#ifdef WIN32
#ifdef MSVC
		COPY_RECORD(EMPTY(), i, EMPTY(), i - 1);
#else
		COPY_RECORD(, i,, i - 1);
#endif

	//data
	for (i = 0; i < numDataFields; i++)
	{
		if (dataRepository[i].format & FORMAT_BY_COLUMN)
		{
			data = dataRepository[i].data;
			data[index].clear();

			//bump everything down
			for (j = index + 1; j < numRecords; j++)
				data[j - 1] = data[j];
		}
		else
			dataRepository[i].data->deletePosition(index);
	}

	if (selectedRecord > (int)index)
		selectedRecord--;
	else if (selectedRecord == (int)index)
		selectedRecord = -1;

	numRecords--;
}

Object * GeoData::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
	return Object::LoadXML(xml,path, ncfile);
}

/*---------------------------------------------------------------------------*/
int GeoData::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	const char *message = argv[0];
	char retval[256];
	GLboolean badSyntax = GL_FALSE;
	
	if (argc >= 1)
	{
		if (!strcmp(message, "trailingSection"))
		{
			if (argc >= 2)
				trailingSeconds = (float)atof(argv[1]);
			sprintf(retval, "%f", trailingSeconds);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if (!strcmp(message, "timeAttribute"))
		{
			if (argc >= 2)
			{
				if (!strcmp(argv[1], "name"))
				{
					if (argc >= 3)
					{
						if (argv[2][0] == '-')
							resetTimeAttribute();
						else
						{
							resetTimeAttribute(getAttributeIndex(argv[2]));
							if (timeAttributeIndex < 0)
							{
								Tcl_AppendResult(interp, "Invalid timeAttribute ", argv[2],
												 ".  Try using listAttributes to find a valid attribute.", NULL);
								return TCL_ERROR;
							}
						}
					}
					Tcl_SetResult(interp, (char *) getAttributeName(timeAttributeIndex + Object::getNumAttributes()).c_str(), TCL_VOLATILE);
					return TCL_OK;
				}
				else if (!strcmp(argv[1], "offset"))
				{
					if (argc >= 3)
					{
						if (!strcmp(argv[2], "now"))
							setTimeAttributeOffset(Scene3D::timeControl->getGZTime());
						else
							setTimeAttributeOffset(atof(argv[2]));
					}
					sprintf(retval, "%f", timeAttributeOffset);
					Tcl_SetResult(interp, retval, TCL_VOLATILE);
					return TCL_OK;
				}
				else if (!strcmp(argv[1], "scale"))
				{
					if (argc >= 3)
						setTimeAttributeScale(atof(argv[2]));
					sprintf(retval, "%f", timeAttributeScale);
					Tcl_SetResult(interp, retval, TCL_VOLATILE);
					return TCL_OK;
				}
				else
					badSyntax = GL_TRUE;
			}
			else
				badSyntax = GL_TRUE;
			if (badSyntax)
			{
				Tcl_AppendResult(interp, "Invalid syntax.  Usage:\n",
								 "timeAttribute name [<attribute> | -]\n",
								 "timeAttribute offset [<startingTime> | now]\n",
								 "timeAttribute scale [<timeScale>]\n",
								 NULL);
				return TCL_ERROR;
			}
			return TCL_OK;
		}
	}
	return Object::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int GeoData::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "trailingSection timeAttribute ", NULL);
	return Object::list(interp);
}

/*----------------------------Attribute Support------------------------------*/
/*---------------------------------------------------------------------------*/
// documented in PosableStateUser
int GeoData::getAttributeIndex(std::string attributeName)
{
	int retval;

	retval = Object::getAttributeIndex(attributeName);
	if (retval < 0)
            for (unsigned int i = 0; i < numDataFields; i++)
			if (dataNames[i] == attributeName)
				return Object::getNumAttributes() + i;
	return retval;
}

/*---------------------------------------------------------------------------*/
/**	Returns the attribute name for the passed attribute index, or NULL if it
 *	is not a valid attribute index.
 */
std::string GeoData::getAttributeName(int attributeIndex)
{
    std::string retval;
	int i;

	retval = Object::getAttributeName(attributeIndex);
	if (retval.empty())
	{
		i = attributeIndex - Object::getNumAttributes();
		if (i >= 0 && i < (int)numDataFields)
			return dataNames[i];
	}

	return retval;
}
