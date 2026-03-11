// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GPHOTO_CAMERA
#define GZ4D_GPHOTO_CAMERA

#include <gz4d/gz4d.h>
#include <memory>
#include <gz4d/gPhoto/Config.h>
#include <gphoto2.h>

namespace gz4d
{
    namespace gphoto
    {
        class Camera
        {
            std::shared_ptr<_GPContext> context;
            std::shared_ptr<_Camera> camera;
            public:
                Camera()
                :context(gp_context_new(),gp_context_unref)
                {
                    _Camera * c;
                    gp_camera_new(&c);
                    camera = std::shared_ptr<_Camera>(c,gp_camera_unref);
                    gp_camera_init(camera.get(),context.get());
                }
                
                bool Capture(std::string const &fname)
                {
                    CameraFilePath path;
                    if(gp_camera_capture(camera.get(),GP_CAPTURE_IMAGE,&path,context.get()) >= 0)
                    {
                        CameraFile *camera_file;
                        gp_file_new(&camera_file);
                        gp_camera_file_get(camera.get(), path.folder, path.name, GP_FILE_TYPE_NORMAL, camera_file, context.get());
                        gp_file_save( camera_file, fname.c_str());
                        gp_file_unref( camera_file );
                        return true;
                    }
                    return false;
                }
                
                Config GetConfig()
                {
                    CameraWidget * cw;
                    if(gp_camera_get_config(camera.get(), &cw, context.get()) >= 0)
                    {
                        std::shared_ptr<CameraWidget> cwsp(cw,gp_widget_unref);
                        return Config(cwsp);
                    }
                    return Config();
                }
                
                bool SetConfig(Config const & c)
                {
                    if(c)
                        return gp_camera_set_config(camera.get(), c.cameraWidget.get(), context.get()) >= 0;
                    return false;
                }
                
        };
    }
}

#endif
