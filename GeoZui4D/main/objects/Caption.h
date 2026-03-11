// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Caption.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides for drawing captions that change over time.
 *	This class implements the following responsibilities:
 *		* Read caption description from a file
 *		* Support viewing of the caption.
 *
 *	Created by mdp 1/4/2007
 */
/*---------------------------------------------------------------------------*/
class Caption;

#ifndef CAPTION_H
#define CAPTION_H

#include "engine/Object.h"

class Caption : public Object
{
	public:
		int readFrom(FILE *file);
		virtual int processField(int option, char **Field);
		int writeTo(FILE *file);
		void DrawLowRes(float degradation, Window3D *win);
		int DrawHighRes(int eye, Window3D *win);

		Caption();
		~Caption();
		DERIVE_OBJECT(Caption);

		enum UsageFlags {Blank						= 0x0000,
						 UseText					= 0x0001,
						 UseFont					= 0x0002,
						 UseColor					= 0x0004,
						 UseBackgroundColor			= 0x0008,
						 UseLocation				= 0x0010,
						 UseComponentMask			= 0x001F,

						 UseShadow					= 0x0100,
						 UseBackgroundBox			= 0x0200,
						 UseBoxWithOutline			= 0x0300,
						 UseBoxOrShadowMask			= 0x0300,
						 UseBackgroundTransient		= 0x0400,
						 UseBackgroundFragile		= 0x0800,
						 UseBackgroundRobust		= 0x0C00,
						 UseBackgroundRobustnessMask= 0x0C00,

						 UseLineAlways				= 0x1000,
						 UseLineAuto				= 0x2000,
						 UseLineNever				= 0x3000,
						 UseLineMask				= 0x3000,
						 UseLineTransient			= 0x4000,
						 UseLineFragile				= 0x8000,
						 UseLineRobust				= 0xC000,
						 UseLineRobustnessMask		= 0xC000,

						 UseBehaviorMask			= 0x3300,
						 UseTransientVal			= 0x4400,
						 UseFragileVal				= 0x8800,
						 UseRobustVal				= 0xCC00,
						 UseDefaultMask				= 0xEE00,

						 UseRobustness				= 0x10000
						};

		enum LabelParams {TEXT = FirstObjectKeyword, FONT, COLOR,
						  CAPTION, LINE, BACKGROUND, LOCATION,
						  ALWAYS, AUTO, DEFAULT, SHADOW,
						  BOX, OUTLINED_BOX,
						  TRANSIENT, FRAGILE, ROBUST, OFF, N_RECORDS,
			  			  NEW, STANDARD_9_BY_15, STANDARD_8_BY_13,
						  TIMES_ROMAN_10, TIMES_ROMAN_24, HELVETICA_10,
						  HELVETICA_12, HELVETICA_18
						 };
	protected:	
		struct CaptionSpec
		{
			double startTime, endTime;
			char *text;
			void *font;
			int width, height, lineHeight, numLines;
			float lineSpacing;
			Point4ub color, backgroundColor;
			int behaviorFlags;
			Point2f location;
			int usageFlags;
			int robustness;

			CaptionSpec();
		};

		CaptionSpec defaultSpec, *timeSpecs, *currentSpec;
		double timeMin, timeMax;
		int numSpecs;

		static const char *CaptionNames[];
		static const char *CaptionFormats[];

		void DrawSpec(Window3D *win, int robustnessType, GLboolean asEditGhost = GL_FALSE);
		void DrawEditGhosts(Window3D *win, int robustnessType);
		void DetermineSpec(Window3D *win);
		void *ConvertFont(char *fontName);
		const char *ConvertFont(void *font);
		int ConvertRobustness(char *field);
		const char *ConvertRobustness(int robustness, int mask);
		int writeSpecTo(FILE *file);
		void determinePrintedSize();
};

#endif 
