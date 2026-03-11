// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/gPhoto/Camera.h>

int main(int argc, char * argv[])
{
    gz4d::gphoto::Camera camera;
    camera.Capture(argv[1]);
}
