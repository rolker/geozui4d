// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEOCAM_CAMERA_H
#define GZ4D_GEOCAM_CAMERA_H

#include <gz4d/gz4d.h>
#include <gz4d/nav/Fix.h>
#include <gz4d/Scale.h>
#include <gz4d/Rotation.h>
#include <gz4d/Translation.h>
#include <gz4d/Box.h>
#include <gz4d/data/Raster.h>
#include <boost/property_tree/ptree.hpp>

namespace gz4d
{
    namespace geocam
    {
        class Camera
        {
        public:
            typedef boost::shared_ptr<Camera> Ptr;
            
            struct Intrinsic
            {
                /// X direction focal length in pixels.
                double fx;
                
                /// Y direction focal length in pixels.
                double fy; 
                
                /// X center of projection in pixels relative to left edge of image sensor.
                double cx;
                
                /// Y center of projection in pixels relative to bottom edge of image sensor.
                double cy;
                
                
                double k1,k2,k3,k4,k5,k6;
                
                double p1,p2;
                
                Intrinsic():fx(0.0),fy(0.0),cx(0.0),cy(0.0),k1(0.0),k2(0.0),k3(0.0),k4(0.0),k5(0.0),k6(0.0),p1(0.0),p2(0.0){}
            };
            
            Camera(std::string const &l):_label(l),_image_width(0),_image_height(0),_attitude(0.0,0.0,0.0),_mask(Vector<double, 2>(0.0,0.0),Vector<double, 2>(1.0,1.0)){}
            Camera(boost::property_tree::ptree const &n):_image_width(0),_image_height(0),_attitude(0.0,0.0,0.0),_mask(Vector<double, 2>(0.0,0.0),Vector<double, 2>(1.0,1.0))
            {
                boost::property_tree::ptree::const_assoc_iterator a = n.find("<xmlattr>");
                if (a != n.not_found())
                {
                    _label = a->second.get("label",_label);
                    _offset[0] = a->second.get("x",_offset[0]);
                    _offset[1] = a->second.get("y",_offset[1]);
                    _offset[2] = a->second.get("z",_offset[2]);
                    
                    _attitude.pitch = a->second.get("pitch",_attitude.pitch.value());
                    _attitude.roll = a->second.get("roll",_attitude.roll.value());
                    _attitude.heading = a->second.get("yaw",_attitude.heading.value());
                    
                    _image_width = a->second.get("width",_image_width);
                    _image_height = a->second.get("height",_image_height);
                    
                    gz4d::Vector< double, 2 > min = _mask.getMin();
                    gz4d::Vector< double, 2 > max = _mask.getMax();
                    min[0] = a->second.get("minx",min[0]);
                    min[1] = a->second.get("miny",min[1]);
                    max[0] = a->second.get("maxx",max[0]);
                    max[1] = a->second.get("maxy",max[1]);
                    
                    _mask.setMin(min);
                    _mask.setMax(max);
                }
                    
                parseIntrinsic(n);
            }
            
            void parseIntrinsic(boost::property_tree::ptree const &i)
            {
                boost::property_tree::ptree::const_assoc_iterator a = i.find("<xmlattr>");
                if (a != i.not_found())
                {
                    _intrinsic.fx = a->second.get("fx",_intrinsic.fx);
                    _intrinsic.fy = a->second.get("fy",_intrinsic.fy);
                    
                    _intrinsic.cx = a->second.get("cx",_intrinsic.cx);
                    _intrinsic.cy = a->second.get("cy",_intrinsic.cy);

                    _intrinsic.k1 = a->second.get("k1",_intrinsic.k1);
                    _intrinsic.k2 = a->second.get("k2",_intrinsic.k2);
                    _intrinsic.k3 = a->second.get("k3",_intrinsic.k3);
                    _intrinsic.k4 = a->second.get("k4",_intrinsic.k4);
                    _intrinsic.k5 = a->second.get("k5",_intrinsic.k5);
                    _intrinsic.k6 = a->second.get("k6",_intrinsic.k6);

                    _intrinsic.p1 = a->second.get("p1",_intrinsic.p1);
                    _intrinsic.p2 = a->second.get("p2",_intrinsic.p2);
                }
                
            }
            

            void save(std::ostream& s) const
            {
                s << "  <Camera label=\"" << _label << "\"";
                s << " x=\"" << _offset[0] << "\"";
                s << " y=\"" << _offset[1] << "\"";
                s << " z=\"" << _offset[2] << "\"";
                s << " roll=\"" << _attitude.roll.value() << "\"";
                s << " pitch=\"" << _attitude.pitch.value() << "\"";
                s << " yaw=\"" << _attitude.heading.value() << "\"";
                s << " width=\"" << _image_width << "\"";
                s << " height=\"" << _image_height << "\"";
                s << " minx=\"" << _mask.getMin()[0] << "\"";
                s << " miny=\"" << _mask.getMin()[1] << "\"";
                s << " maxx=\"" << _mask.getMax()[0] << "\"";
                s << " maxy=\"" << _mask.getMax()[1] << "\"";
                //s << ">\n";
                
                //s << "    <Intrinsic";
                s << " fx=\"" << _intrinsic.fx << "\"";
                s << " fy=\"" << _intrinsic.fy << "\"";
                s << " cx=\"" << _intrinsic.cx << "\"";
                s << " cy=\"" << _intrinsic.cy << "\"";
                s << " k1=\"" << _intrinsic.k1 << "\"";
                s << " k2=\"" << _intrinsic.k2 << "\"";
                s << " k3=\"" << _intrinsic.k3 << "\"";
                s << " k4=\"" << _intrinsic.k4 << "\"";
                s << " k5=\"" << _intrinsic.k5 << "\"";
                s << " k6=\"" << _intrinsic.k6 << "\"";
                s << " p1=\"" << _intrinsic.p1 << "\"";
                s << " p2=\"" << _intrinsic.p2 << "\"";
                s << "/>\n";
                
                //s << "  </Camera>\n";
            }
            
            Vector<double,2> getFOVs() const
            {
                return Degrees(getRadianFOVs());
            }
            
            Vector<double,2> getRadianFOVs() const
            {
                return Vector<double,2>(2.0*atan((_image_width/2.0)/_intrinsic.fx),2.0*atan((_image_height/2.0)/_intrinsic.fy));
            }
            
            
            std::string getInfo() const
            {
                std::stringstream ss;
                
                ss << "label: " << _label << "\n";
                
                if(_image_width > 0 && _intrinsic.fx > 0.0)
                {
                    double fov = 2.0*atan((_image_width/2.0)/_intrinsic.fx);
                    ss << "fov (horizontal): " << Degrees(fov) << "\n";
                }
                if(_image_height > 0 && _intrinsic.fy > 0.0)
                {
                    double fov = 2.0*atan((_image_height/2.0)/_intrinsic.fy);
                    ss << "fov (vertical): " << Degrees(fov) << "\n";
                }
                if(_intrinsic.fy > 0.0 && _offset[2] > 0.0)
                {
                    for(double r = 250.0; r <= 5000.0; r*=2.0)
                    {
                        double angle = atan(r/_offset[2]);
                        double dangle = atan(1.0/(2.0*_intrinsic.fy));
                        double r1 = _offset[2]*tan(angle-dangle);
                        double r2 = _offset[2]*tan(angle+dangle);
                        ss << "range: " << r << " angle: " << Degrees(angle) << " footprint: " << r2-r1 << "\n";
                    }
                }
                
                return ss.str();
            }
            
            
            Matrix<double,4,4> getProjection() const
            {
                double l = -_intrinsic.cx/_intrinsic.fx;
                double r = l + _image_width/_intrinsic.fx;
                double b = -_intrinsic.cy/_intrinsic.fy;
                double t = b + _image_height/_intrinsic.fy;
                double n = 1.0;
                return Frustum<double>(l,r,b,t,n)*getCameraToGround();
            }
            
            
            Vector<float,2> getFocalLength() const
            {
                return Vector<float,2>(_intrinsic.fx/float(_image_width),_intrinsic.fy/float(_image_height));
            }
            
            Vector<double,2> getImagerOffset() const
            {
                return Vector<float,2>(_intrinsic.cx/double(_image_width),_intrinsic.cy/double(_image_height));
            }
            
            void setImagerOffset(Vector<double,2> const &io)
            {
                _intrinsic.cx = io[0]*_image_width;
                _intrinsic.cy = io[1]*_image_height;
                _distortion_grid.reset();
                _undistort_grid.reset();
                
            }
            
            std::vector<double> getRadialDistortionParameters() const
            {
                std::vector<double> ret;
                ret.push_back(_intrinsic.k1);
                ret.push_back(_intrinsic.k2);
                ret.push_back(_intrinsic.k3);
                ret.push_back(_intrinsic.k4);
                ret.push_back(_intrinsic.k5);
                ret.push_back(_intrinsic.k6);
                return ret;
            }
            
            void setRadialDistortionParameters(std::vector<double> const &ks)
            {
                _intrinsic.k1 = ks[0];
                _intrinsic.k2 = ks[1];
                _intrinsic.k3 = ks[2];
                _intrinsic.k4 = ks[3];
                _intrinsic.k5 = ks[4];
                _intrinsic.k6 = ks[5];
                _distortion_grid.reset();
                _undistort_grid.reset();
                
            }
            
            void setRadialDistortionParameter(int i, double k)
            {
                switch(i)
                {
                    case 1:
                        _intrinsic.k1 = k;
                        break;
                    case 2:
                        _intrinsic.k2 = k;
                        break;
                    case 3:
                        _intrinsic.k3 = k;
                        break;
                    case 4:
                        _intrinsic.k4 = k;
                        break;
                    case 5:
                        _intrinsic.k5 = k;
                        break;
                    case 6:
                        _intrinsic.k6 = k;
                        break;
                }
                _distortion_grid.reset();
                _undistort_grid.reset();
            }
            
            std::vector<double> getTangentialDistortionParameters() const
            {
                std::vector<double> ret;
                ret.push_back(_intrinsic.p1);
                ret.push_back(_intrinsic.p2);
                return ret;
            }
            
            void setTangentialDistortionParameters(std::vector<double> const &tdp)
            {
                if(_intrinsic.p1 != tdp[0] || _intrinsic.p2 != tdp[1])
                {
                    _intrinsic.p1 = tdp[0];
                    _intrinsic.p2 = tdp[1];
                    _distortion_grid.reset();
                    _undistort_grid.reset();
                }
            }
            
            /// Camera position relative to platform.
            Point<double> const &getOffset() const
            {
                return _offset;
            }
            
            void setOffset(Point<double> const &p)
            {
                _offset = p;
            }
            
            Matrix<double,4,4> getTransform() const
            {
                return Rotation<double>(_attitude.roll,Point<double>(0.0,1.0,0.0)).GetMatrix()
                *Rotation<double>(-_attitude.pitch,Point<double>(1.0,0.0,0.0)).GetMatrix()
                *Rotation<double>(_attitude.heading,Point<double>(0.0,0.0,1.0)).GetMatrix()
                *Translation<double>(-_offset).GetMatrix();
            }
            
            /// Get matrix that transforms from camera's reference frame to world reference frame and vice-versa.
            /// Camera's reference frame is x and y positive point right and up along image plane and z positive points forward along lens.
            /// World reference frame has x pointing east, y pointing north and z pointing up.
            /// Camera is located at the origin and is pointing north.
            static Matrix<double,4,4> getCameraToGround()
            {
                //return Scale<double>(1.0,1.0,-1.0).GetMatrix()*Rotation<double>(-Angle<double,Degree>(90.0),Point<double>(1.0,0.0,0.0)).GetMatrix();
                
                const double vals[] = {1.0,0.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,0.0,1.0};
                return Matrix<double,4,4>(std::pair<const double*,const double*>(vals,&vals[16]));
            }
            
            nav::Attitude<double> const & getAttitude() const
            {
                return _attitude;
            }
            
            Vector<double,2> distort(double x1, double y1)
            {
                if(!_distortion_grid)
                {
                    _distortion_grid = Raster<Vector<double, 2> >::Ptr(new Raster<Vector<double, 2> >);
                    _distortion_grid->Resize(_image_width,_image_height);
                    for(int y = 0; y < _image_height; ++y)
                    {
                        gz4d::Vector< double, 2 >* r = _distortion_grid->GetRow(y);
                        double yd = (y-_image_height/2)/_intrinsic.fy;
                        for(int x = 0; x < _image_width; ++x)
                        {
                            double xd = (x-_image_width/2)/_intrinsic.fx;
                            
                            double r2 = xd*xd+yd*yd;
                            double kr = (1.0+_intrinsic.k1*r2+_intrinsic.k2*r2*r2+_intrinsic.k3*r2*r2*r2)/
                            (1.0+_intrinsic.k4*r2+_intrinsic.k5*r2*r2+_intrinsic.k6*r2*r2*r2);
                            double x2 = xd*kr+2.0*_intrinsic.p1*xd*yd+_intrinsic.p2*(r2+2.0*xd*xd);
                            double y2 = yd*kr+_intrinsic.p1*(r2+2.0*yd*yd)+2.0*_intrinsic.p2*xd*yd;
                            r[x]=Vector<double,2>(x2,y2);
                        }
                    }
                }
                
                double r2 = x1*x1+y1*y1;
                double kr = (1.0+_intrinsic.k1*r2+_intrinsic.k2*r2*r2+_intrinsic.k3*r2*r2*r2)/
                (1.0+_intrinsic.k4*r2+_intrinsic.k5*r2*r2+_intrinsic.k6*r2*r2*r2);
                double x2 = x1*kr+2.0*_intrinsic.p1*x1*y1+_intrinsic.p2*(r2+2.0*x1*x1);
                double y2 = y1*kr+_intrinsic.p1*(r2+2.0*y1*y1)+2.0*_intrinsic.p2*x1*y1;
                return Vector<double,2>(x2,y2);
                
                
                int xi = (_image_width/2)+(x1*_intrinsic.fx);
                int yi = (_image_height/2)+(y1*_intrinsic.fy);
                if(xi < 0 || xi >= _image_width || yi < 0 || yi >= _image_height)
                    //return Vector<double,2>(xi,yi);
                    return Vector<double,2>(Nan<double>(),Nan<double>());
                return _distortion_grid->GetRow(yi)[xi];
            }
            
            
            Vector<double,2> undistort(double x2, double y2)
            {
                if(!_undistort_grid)
                {
                    if(!_distortion_grid)
                        distort(0.0,0.0);
                    _undistort_grid = Raster<Vector<double, 3> >::Ptr(new Raster<Vector<double, 3> >(_image_width,_image_height,Vector<double,3>(0.0,0.0,0.0)));
                    for(int y = 0; y < _image_height; ++y)
                    {
                        gz4d::Vector< double, 2 >* r = _distortion_grid->GetRow(y);
                        double y1 = (y-(_image_height/2))/_intrinsic.fy;
                        for(int x = 0; x < _image_width; ++x)
                        {
                            int x2i = (_image_width/2)+(r[x][0]*_intrinsic.fx);
                            double xp = (_image_width/2)+(r[x][0]*_intrinsic.fx)-x2i;
                            
                            int y2i = (_image_height/2)+(r[x][1]*_intrinsic.fy);
                            double yp = (_image_height/2)+(r[x][1]*_intrinsic.fy)-y2i;
                            
                            double x1 = (x-(_image_width/2))/_intrinsic.fx;
                            
                            if(x2i >= 0 && x2i < _image_width)
                            {
                                if(y2i >= 0 && y2i < _image_height)
                                    _undistort_grid->GetRow(y2i)[x2i]+=Vector<double,3>(x1,y1,(1-xp)*(1-yp));
                                if(y2i+1 >= 0 && y2i+1 < _image_height)
                                    _undistort_grid->GetRow(y2i+1)[x2i]+=Vector<double,3>(x1,y1,(1-xp)*yp);
                            }
                            if(x2i+1 >= 0 && x2i+1 < _image_width)
                            {
                                if(y2i >= 0 && y2i < _image_height)
                                    _undistort_grid->GetRow(y2i)[x2i+1]+=Vector<double,3>(x1,y1,xp*(1-yp));
                                if(y2i+1 >= 0 && y2i+1 < _image_height)
                                    _undistort_grid->GetRow(y2i+1)[x2i+1]+=Vector<double,3>(x1,y1,xp*yp);
                            }
                        }
                    }
                }
                
                int xi = (_image_width/2)+(x2*_intrinsic.fx);
                int yi = (_image_height/2)+(y2*_intrinsic.fy);
                if(xi < 0 || xi >= _image_width || yi < 0 || yi >= _image_height)
                    return Vector<double,2>(Nan<double>(),Nan<double>());
                gz4d::Vector< double, 3 > ret = _undistort_grid->GetRow(yi)[xi];
                return Vector<double,2>(ret,0)/ret[2];
            }
            
            std::string const &getLabel() const
            {
                return _label;
            }
            
            void updateTransforms(nav::Fix::Ptr nav)
            {
                if(_lastNav != nav)
                {
                    gz4d::Matrix< double, 4, 4 > roll = gz4d::Rotation<double>(-nav->roll,gz4d::Point<double>(0.0,1.0,0.0)).GetMatrix();
                    gz4d::Matrix< double, 4, 4 > heading = gz4d::Rotation<double>(nav->heading,gz4d::Point<double>(0.0,0.0,1.0)).GetMatrix();
                    gz4d::Matrix< double, 4, 4 > pitch = gz4d::Rotation<double>(-nav->pitch,gz4d::Point<double>(1.0,0.0,0.0)).GetMatrix();
                    
                    gz4d::Matrix< double, 4, 4 > ship_attitude = roll*pitch*heading
                    *gz4d::Translation<double>(0.0,0.0,-nav->altitude).GetMatrix();
                    gz4d::Matrix< double, 4, 4 > offset = getCameraToGround()*getTransform();
                    _transform = offset*ship_attitude;
                    _inverse_transform = inverse(_transform);
                    
                    _lastNav = nav;
                }
                
            }

            /// Calculates point relative to platform of a point in image at distance d.
            /// @param u Image x point, (pixels)
            /// @param v Image y point, (pixels)
            /// @param d normal distance from image plane, (m)
            Point<double> imageToPlatformPoint(double u, double v, double d = 1.0)
            {
                if(d==0.0)
                    return inverse(getCameraToGround()*getTransform())*Point<double>(0.0,0.0,0.0);
                
                double x2 = (u-_intrinsic.cx)/_intrinsic.fx;
                double y2 = (v-_intrinsic.cy)/_intrinsic.fy;
                
                gz4d::Vector< double, 2 > ud = undistort(x2,y2);
                
                return inverse(getCameraToGround()*getTransform())*Point<double>(ud[0]*d,ud[1]*d,d);
            }

            /// Calculates vector in platform's reference frame from camera through point in image.
            /// @param u Image x point, (pixels)
            /// @param v Image y point, (pixels)
            /// @param d normal distance from image plane, (m)
            Point<double> imageToVector(double u, double v, double d = 1.0)
            {
                Point<double> p = imageToPlatformPoint(u,v,1.0);
                Point<double> p2 = imageToPlatformPoint(u,v,0.0);

                return p-p2;
            }
            
            
            /// Calculates point on the ground in meters relative to platform's position of a point in image.
            /// @param u Image x point, (pixels)
            /// @param v Image y point, (pixels)
            /// @param nav Navigation fix, used for platform's attitude
            Point<double> imageToRelativePoint(double u, double v, nav::Fix::Ptr nav, double altitude = 0.0)
            {
                updateTransforms(nav);
                
                double x2 = (u-_intrinsic.cx)/_intrinsic.fx;
                double y2 = (v-_intrinsic.cy)/_intrinsic.fy;
                
                gz4d::Vector< double, 2 > ud = undistort(x2,y2);
                
                
                gz4d::Point<double> p(ud[0],ud[1],1.0);
                gz4d::Point<double> p2(0.0,0.0,0.0);
                
                p = _inverse_transform*p;
                p2 = _inverse_transform*p2;
                
                gz4d::Vector< double, 3 > pd = p-p2;
                // std::cerr << "p: " << p << " p2: " << p2 << " diff: " << pd << std::endl;
                
                if((altitude<p[2] && pd[2] < 0.0)||(altitude>p[2] && pd[2] > 0.0))
                    return p2+pd*(altitude-p[2]/pd[2]);
                return gz4d::Point<double>(Nan<double>(),Nan<double>(),Nan<double>());
                
            }
            
            
            /// Transform point in world relative to platform position to uv image point.
            /// Returns u,v image indecies and p[2] (distance?)
            Vector<int,3> relativePointToImage(gz4d::Point<double> const &relativePoint, nav::Fix::Ptr nav)
            {
                updateTransforms(nav);
                gz4d::Point<double> p = _transform*relativePoint;
//                 if(p[2]<=0.0)
//                     return Vector<int,2>(-1,-1);
                double x1,y1;
                if (p[2] == 0)
                {
                    x1 = 0;
                    y1 = 0;
                }
                else
                {
                    x1 = p[0]/p[2];
                    y1 = p[1]/p[2];
                }
                
                if(fabs(x1) > 1.25*(_image_width/2.0)/_intrinsic.fx || fabs(y1) > 1.25*(_image_height/2.0)/_intrinsic.fy)
                    return Vector<int,3>(-1,-1,p[2]);
                
                gz4d::Vector< double, 2 > xy2 = distort(x1,y1);
                if(IsNan(xy2[0]) || IsNan(xy2[1]))
                    return Vector<int,3>(-1,-1,p[2]);
                
                return Vector<int,3>(_intrinsic.fx*xy2[0]+_intrinsic.cx,_intrinsic.fy*xy2[1]+_intrinsic.cy,p[2]);
            }

            /// Transform point relative to platform to uv image point.
            Vector<double,2> platformPointToImage(gz4d::Point<double> const &relativePoint)
            {
                gz4d::Matrix< double, 4, 4 > offset = getCameraToGround()*getTransform();
                gz4d::Point<double> p = offset*relativePoint;
                double x1 = p[0]/p[2];
                double y1 = p[1]/p[2];
                gz4d::Vector< double, 2 > xy2 = distort(x1,y1);
                
                gz4d::Vector< float, 2 > fl = getFocalLength();
                gz4d::Vector< float, 2 > io = getImagerOffset();
                
                double u = fl[0]*xy2[0]+io[0];
                double v = fl[1]*xy2[1]+io[1];
                
                return Vector<double,2> (u,v);
            }
            
            bool isMasked(Vector<double,2> const &uv)
            {
                return !_mask.contains(uv);
            }
            
            double getPitch() const
            {
                return _attitude.pitch.value();
            }
            
            void setPitch(double p)
            {
                _attitude.pitch = p;
            }
            
            double getYaw() const
            {
                return _attitude.heading.value();
            }
            
            void setYaw(double y)
            {
                _attitude.heading = y;
            }
            
            double getRoll() const
            {
                return _attitude.roll.value();
            }
            
            void setRoll(double r)
            {
                _attitude.roll = r;
            }
            
            double getFX() const
            {
                return _intrinsic.fx;
            }
            
            void setFX(double fx)
            {
                _intrinsic.fx = fx;
            }
                
            double getFY() const
            {
                return _intrinsic.fy;
            }
            
            void setFY(double fy)
            {
                _intrinsic.fy = fy;
            }

            double getCX() const
            {
                return _intrinsic.cx;
            }

            void setCX(double cx)
            {
                _intrinsic.cx = cx;
            }
           
            double getCY() const
            {
                return _intrinsic.cy;
            }

            void setCY(double cy)
            {
                _intrinsic.cy = cy;
            }
            
            long int getImageWidth() const
            {
                return _image_width;
            }
            
            long int getImageHeight() const
            {
                return _image_height;
            }
            
            void setImageSize(long int w, long int h)
            {
                _image_width = w;
                _image_height = h;
            }
                
            
        private:
            std::string _label;
            long int _image_width,_image_height;
            Intrinsic _intrinsic;

            Point<double> _offset;
            nav::Attitude<double> _attitude;
            Box<Vector<double,2> > _mask;
            
            nav::Fix::Ptr _lastNav;
            gz4d::Matrix< double, 4, 4 > _transform;
            gz4d::Matrix< double, 4, 4 > _inverse_transform;
            
            Raster<Vector<double,2> >::Ptr _distortion_grid;
            Raster<Vector<double,3> >::Ptr _undistort_grid;
        };
    }
}

#endif
