// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/render/Grid.h>
#include <gz4d/glut/Application.h>
#include <gz4d/SphericalPoint.h>
#include <gz4d/util/Utilities.h>

class GlutGrid: public gz4d::Grid<gz4d::SphericalPoint<float> >,  public gz4d::glut::Application<GlutGrid>
{
    public:
        void Draw()
        {
            gz4d::Grid<gz4d::SphericalPoint<float> >::Draw();
            SwapBuffers();
        }
};


int main()
{

    GlutGrid sg;
    //std::shared_ptr<gz4d::Grid<gz4d::SphericalPoint<float> > > sg(new gz4d::Grid<gz4d::SphericalPoint<float> >);

    sg.SetStartPoint(gz4d::SphericalPoint<float>(0.0,gz4d::Radians(-90.0),0.0));
    sg.AddDimension(gz4d::SphericalPoint<float>(0.0,gz4d::Radians(5.0),0.0),36);
    sg.AddDimension(gz4d::SphericalPoint<float>(0.2,0.0,0.0),5);


    sg("Grid");
}
