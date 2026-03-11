// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_RENDER_BOX_H
#define GZ4D_RENDER_BOX_H

#include <gz4d/glo/Vertex.h>

namespace gz4d
{
    template<typename T, std::size_t N> void Draw(Box<T,N> const &b)
    {
        Vector<T,N> corners[2];
        corners[0] = b.getMin();
        corners[1] = b.getMax();


        glBegin(GL_LINES);
        for(int i = 0; i < N-1; ++i)
        {
            int c1 = i;
            int c2 = (i+1)%2;
            for(int j = 0; j < N; ++j)
            {
                int d1 = j;
                int d2 = (j+1)%N;

                Point<T> sideCorners[4];
                sideCorners[0] = corners[c1];
                sideCorners[1] = sideCorners[0];
                sideCorners[1][d1] = corners[c2][d1];
                sideCorners[2] = sideCorners[1];
                sideCorners[2][d2] = corners[c2][d2];
                sideCorners[3] = sideCorners[2];
                sideCorners[3][d1] = sideCorners[0][d1];

                for(int k = 0; k < 3; ++k)
                    for(int l = k+1; l < 4; ++l)
                    {
                        glo::Vertex(sideCorners[k]);
                        glo::Vertex(sideCorners[l]);
                    }
            }
        }
        glEnd();
    }
}

#endif
