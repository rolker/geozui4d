/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// BilinearInterpolation.cpp
//
//	This class implements bilinear interpolation for converting
//	ConverstionGrids to GUTMs.
//
// Created by mdp 12/17/2001
///////////////////////////////////////////////////////////////////////////////

#include "BilinearInterpolation.h"
#include "engine/ConversionGrid.h"
#include <iostream>

using namespace std;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
BilinearInterpolation::BilinearInterpolation()
: ConversionMethod("BILINEAR")
{
}

/*---------------------------------------------------------------------------*/
BilinearInterpolation::~BilinearInterpolation()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int BilinearInterpolation::conversionMethod()
{
	int col, row, i, left, right, top, bottom;
	float sourceRow, sourceCol, lastSourceRow = 1, lastSourceCol = 1;
	double x, y, sourceX, sourceY;
	float minXVal, maxXVal, minYVal, maxYVal;
	float xRatio, yRatio, xRatioInv, yRatioInv, val;

	minXVal = sourceCoordsX[0];
	maxXVal = sourceCoordsX[sourceCols - 1];
	minYVal = sourceCoordsY[0];
	maxYVal = sourceCoordsY[sourceRows - 1];
	for (row = 0; row < numRows; row++)
	{
		y = row * spacing + mins[1];
		for (col = 0; col < numCols; col++)
		{
			x = col * spacing + mins[0];
			gz4d::geo::Position<double> s = projection.inverse(x, y);
                        sourceX = s.longitude;
                        sourceY = s.latitude;
			if (sourceX < minXVal || sourceY < minYVal ||
				sourceX > maxXVal || sourceY > maxYVal)
			{
				targetGrid[row][col] = 0.0;
				lastSourceRow = lastSourceCol = 1;
			}
			else //in range
			{
				if (sourceSpacingY > 0.0)
					sourceRow = (float) ((sourceY - minYVal) / sourceSpacingY);
				else //search for proper row
				{
					//ensure sourceCoordsY[i] is just barely greater than sourceY
					if (lastSourceRow > 1 && sourceY < sourceCoordsY[(int)lastSourceRow])
					{
						for (i = (int)lastSourceRow - 2; i >= 0; i--)
						{
							if (sourceCoordsY[i] <= sourceY)
								break;
						}
						i++;
					}
					else
					{
						for (i = (int)lastSourceRow; i < sourceRows; i++)
						{
							if (sourceCoordsY[i] > sourceY)
								break;
						}
					}
					//linearly interpolate to find sourceRow.
					sourceRow = (float) (i - 1 + ((sourceY - sourceCoordsY[i - 1]) / (sourceCoordsY[i] - sourceCoordsY[i - 1])));
				}

				if (sourceSpacingX > 0.0)
					sourceCol = (float)((sourceX - minXVal) / sourceSpacingX);
				else //search for proper col
				{
					//ensure sourceCoordsX[i] is just barely greater than sourceX
					if (lastSourceCol > 1 && sourceX < sourceCoordsX[(int)lastSourceCol])
					{
						for (i = (int)lastSourceCol - 2; i >= 0; i--)
						{
							if (sourceCoordsX[i] <= sourceX)
								break;
						}
						i++;
					}
					else
					{
						for (i = (int)lastSourceCol; i < sourceCols; i++)
						{
							if (sourceCoordsX[i] > sourceX)
								break;
						}
					}
					//linearly interpolate to find sourceCol.
					sourceCol = (float)(i - 1 + ((sourceX - sourceCoordsX[i - 1]) / (sourceCoordsX[i] - sourceCoordsX[i - 1])));
				}
				lastSourceRow = sourceRow;
				lastSourceCol = sourceCol;

				//interpolate
				left = (int) sourceCol;
				right = left + 1;
				bottom = (int) sourceRow;
				top = bottom + 1;
				if (top >= sourceRows || right >= sourceCols
					|| bottom < 0 || left < 0)
				{
					targetGrid[row][col] = 0.0;
					lastSourceRow = lastSourceCol = 1;
				}
				else
				{
					xRatio = sourceCol - left;
					yRatio = sourceRow - bottom;
					xRatioInv = 1.0f - xRatio;
					yRatioInv = 1.0f - yRatio;
					val =
						(yRatioInv * (sourceGrid[bottom][left] * xRatioInv +
									  sourceGrid[bottom][right] * xRatio)) +
						(yRatio * (sourceGrid[top][left] * xRatioInv +
								   sourceGrid[top][right] * xRatio)) + zOffset;
					targetGrid[row][col] = val;
				}
			}
		}
		gridObject->updateProgress((30.0 * (row + 1.0) / numRows) + 50.0);
	}

	return 0;
}
