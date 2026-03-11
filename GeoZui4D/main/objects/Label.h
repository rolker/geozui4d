// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Label.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides for drawing labels in a scale-dependent manner.
 *	This class implements the following responsibilities:
 *		* Read label description from a file
 *		* Support viewing of the label.
 *
 *	Created by mdp 8/29/2002
 */
/*---------------------------------------------------------------------------*/
class Label;

#ifndef LABEL_H
#define LABEL_H

#include "engine/Object.h"

class Gutm;

class Label : public Object
{
	public:
		int readFrom(FILE *file);
		virtual int processField(int option, char **Field);
		int writeTo(FILE *file);
		void DrawLowRes(float degradation, Window3D *win);
		int DrawHighRes(int eye, Window3D *win);

		Label();
		~Label();
		DERIVE_OBJECT(Label);

		enum UsageFlags {Blank						= 0x0000,
						 UseText					= 0x0001,
						 UseFont					= 0x0002,
						 UseColor					= 0x0004,
						 UseBackgroundColor			= 0x0008,
						 UsePriority				= 0x0010,
						 UseSurface					= 0x0020,
						 UseInterestPoint			= 0x0040,
						 UseInterestRegion			= 0x0080,
						 UseInterestMask			= 0x00C0,
						 UseComponentMask			= 0x00FF,

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
						  LABEL, PRIORITY, LINE, BACKGROUND, INTEREST,
						  SURFACE, ALWAYS, AUTO, DEFAULT, SHADOW,
						  BOX, OUTLINED_BOX, POINT, REGION, SCALE,
						  TRANSIENT, FRAGILE, ROBUST, OFF, N_RECORDS,
			  			  NEW_SCALE, STANDARD_9_BY_15, STANDARD_8_BY_13,
						  TIMES_ROMAN_10, TIMES_ROMAN_24, HELVETICA_10,
						  HELVETICA_12, HELVETICA_18
						 };
	protected:	
		struct LabelSpec
		{
			float scaleMin;
			char *text;
			void *font;
			int width, height;
			Point4ub color, backgroundColor;
			int priority;
			int behaviorFlags;
			Point3f interestRegionMin, interestRegionMax;
			Gutm *surface;
			int usageFlags;
			int robustness;

			LabelSpec();
		};

		LabelSpec defaultSpec, *scaleSpecs, *currentSpec;
		float scaleMax;
		int numSpecs;
		GLdouble oldScreenPos[3];
		GLboolean oldScreenPosValid;

		static const char *LabelNames[];
		static const char *LabelFormats[];

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
