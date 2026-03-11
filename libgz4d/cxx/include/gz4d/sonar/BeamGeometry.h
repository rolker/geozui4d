// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SONAR_BEAMGEOMETRY_H
#define GZ4D_SONAR_BEAMGEOMETRY_H

#include <gz4d/gz4d.h>
#include <gz4d/Vector.h>
#include <gz4d/vector/operations.h>
#include <gz4d/util/Utilities.h>

namespace gz4d
{
    namespace sonar
    {

        struct BeamGeometry
        {
            Vector<float,2> width; ///< Degrees, width[0] is x, or across track and width[1] or y is along track
            Vector<float,2> direction; ///< Degrees, positive x is towards starboard and positive y is twards the bow
            BeamGeometry():width(Nan<float>(),Nan<float>()),direction(Nan<float>(),Nan<float>()){}

            float Area(float r = 1.0) const
            {
                float tanx = tan(Radians(direction[0]));
                float tany = tan(Radians(direction[1]));

                float z = r/sqrt(1+tanx*tanx+tany*tany);
                float x = z*tanx;
                float y = z*tany;

                float x1 = z*tan(Radians(direction[0]-width[0]/2.0));
                float x2 = z*tan(Radians(direction[0]+width[0]/2.0));
                float y1 = z*tan(Radians(direction[1]-width[1]/2.0));
                float y2 = z*tan(Radians(direction[1]+width[1]/2.0));

                // Assuming area is sum of 4 "quarter-ellipses"
                return (fabs(x-x1)*fabs(y-y1)+fabs(x-x1)*fabs(y-y2)+fabs(x-x2)*fabs(y-y1)+fabs(x-x2)*fabs(y-y2))*M_PI/4.0;
            };

            float AreaRatio() const
            {
                float dx = Radians(direction[0]);
                float bwx = Radians(width[0]);
                float basex = 2.0*tan(bwx/2.0);
                float rx = fabs(tan(dx+bwx/2.0)-tan(dx-bwx/2.0))/basex;

                float dy = Radians(direction[1]);
                float bwy = Radians(width[1]);
                float basey = 2.0*tan(bwy/2.0);
                float ry = fabs(tan(dy+bwy/2.0)-tan(dy-bwy/2.0))/basey;
                return rx*ry;
            }

            float PulseAreaRatio(float r, float pl) const
            {
                float dx = Radians(direction[0]);
                float bwx = Radians(width[0]);
                float sinx = sin(dx);
                float cosx = cos(dx);

                float dy = Radians(direction[1]);
                float bwy = Radians(width[1]);
                float siny = sin(dy);
                float cosy = cos(dy);

                //float tanx = sinx/cosx;
                //float tany = siny/cosy;

                //float xRatio =  fabs(tan(dx+bwx/2.0)-tan(dx-bwx/2.0))/(2.0*tan(bwx/2.0));
                //float yRatio =  fabs(tan(dy+bwy/2.0)-tan(dy-bwy/2.0))/(2.0*tan(bwy/2.0));

                Vector<float,3> beamVector(sinx/(cosx*cosy),siny/(cosx*cosy),1.0);

                float z = r-(pl/2.0)*normalize(beamVector)[2];
                float z2 = z*z;

                if (z<r-pl)
                {
                    float pd = sqrt(r*r-z2)-sqrt((r-pl)*(r-pl)-z2);
                    float rx = pd/fabs(tan(dx+bwx/2.0)-tan(dx-bwx/2.0));
                    float ry = pd/fabs(tan(dy+bwy/2.0)-tan(dy-bwy/2.0));

                    float n = sqrt(beamVector[0]*beamVector[0]+beamVector[1]*beamVector[1]);
                    return std::min(float(1.0),rx*beamVector[0]/n+ry*beamVector[1]/n);
                }
                return 1.0;
            }

        };
    }
}

#endif
