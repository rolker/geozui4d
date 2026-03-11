// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/rtsp/Rtsp.h>
#include <gz4d/time.h>

int main(int argc, char *argv[]){
    double timeout = atof(argv[argc-1]);
    gz4d::rtsp::Rtsp rtsp(argv[1],"geocam","geocampwd");
    for (int i = 2; i < argc-1; ++i)
    {
        rtsp.save(argv[i]);
        gz4d::Sleep(gz4d::seconds(timeout));
    }
}
