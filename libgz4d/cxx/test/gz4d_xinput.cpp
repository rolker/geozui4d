// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/gz4d.h>
#ifdef GZ4D_XINPUT
#include <gz4d/XInput.h>

int main(int argc, char * argv[])
{
    gz4d::XInput x;
    x.Connect();
    x.Print();
    x.OpenDevice(0);
}
#else
int main(){}
#endif
