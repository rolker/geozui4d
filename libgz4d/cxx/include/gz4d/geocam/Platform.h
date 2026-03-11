// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEOCAM_PLATFORM_H
#define GZ4D_GEOCAM_PLATFORM_H

#include "Camera.h"
#include <gz4d/nav/Track.h>
#include <boost/property_tree/xml_parser.hpp>

namespace gz4d
{
    namespace geocam
    {
        class Platform
        {
        public:
            typedef std::map<std::string,Camera::Ptr> CameraMap;
        private:
            CameraMap _cameras;
            gz4d::nav::Track::Ptr _nav;
        public:
            typedef boost::shared_ptr<Platform> Ptr;
            
            static Ptr create(std::string const &fname)
            {
                Ptr ret(new Platform);
                ret->parseConfig(fname);
                return ret;
            }
            
            void parseConfig(std::string const &fname)
            {
                boost::property_tree::ptree doc;
                read_xml(fname,doc);
                boost::property_tree::ptree::const_iterator root = doc.begin();
                for(boost::property_tree::ptree::const_iterator c = root->second.begin(); c != root->second.end(); ++c)
                {
                    if(c->first == "Camera")
                    {
                        gz4d::geocam::Camera::Ptr cam(new gz4d::geocam::Camera(c->second));
                        _cameras[cam->getLabel()]=cam;
                    }
                }
            }
            
            void setNavigation(std::string const &n)
            {
                _nav = gz4d::nav::Track::Ptr(new gz4d::nav::Track(n));
            }
            
            
            Camera::Ptr getCamera(std::string const &l) const
            {
                std::map< std::string, Camera::Ptr >::const_iterator c = _cameras.find(l);
                if(c == _cameras.end())
                    return Camera::Ptr();
                return c->second;
            }
            
            const CameraMap &getCameras() const
            {
                return _cameras;
            }
            
            nav::Fix::Ptr getFix(Time t) const
            {
                nav::Fix::Ptr ret = (*_nav)(t);
                return ret;
            }
            
            nav::Track::Ptr getNavigation() const
            {
                return _nav;
            }
        };
    }
}

#endif
