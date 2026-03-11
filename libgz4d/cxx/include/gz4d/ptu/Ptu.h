// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_PTU_PTU_H
#define GZ4D_PTU_PTU_H

#include <gz4d/gz4d.h>
#include <gz4d/serial/SerialPort.h>
#include <boost/algorithm/string.hpp>
#include <gz4d/time.h>


namespace gz4d
{
    class Ptu
    {
        SerialPort port;
        float degs_to_pan_units;
        float degs_to_tilt_units;

        int pan_min, pan_max;
        int tilt_min, tilt_max;

        int pan_min_speed, pan_max_speed;
        int tilt_min_speed, tilt_max_speed;

        std::string Command(std::string const &cmd)
        {
            port.Write(cmd);
            std::vector<char> ret(256);
            gz4d::Sleep(seconds(0.1));
            port.Read(ret);
            boost::iterator_range<std::vector<char>::iterator> ir = boost::find_last(ret,"*");
            return std::string(ir.begin(),ret.end());
        }
        
        int GetRawPan()
        {
            std::string ret_str = Command("PP\n");
            std::istringstream ss(ret_str.substr(ret_str.find("is")+3));
            int ret;
            ss >> ret;
            return ret;
        }
        
        int GetRawTilt()
        {
            std::string ret_str = Command("TP\n");
            std::istringstream ss(ret_str.substr(ret_str.find("is")+3));
            int ret;
            ss >> ret;
            return ret;
        }
        
        public:
            Ptu(std::string const &p)
            {
                port.Open(p);
                port.SetBaudRate(9600);
                
                std::istringstream ss(Command("PR\n").substr(2));
                float r;
                ss >> r;
                degs_to_pan_units = 3600.0/r;
                
                ss.str(Command("TR\n").substr(2));
                ss >> r;
                degs_to_tilt_units = 3600.0/r;
                
                std::string result = Command("PN\n");
                ss.str(result.substr(result.find("is")+3));
                ss >> pan_min;
                
                result = Command("PX\n");
                ss.str(result.substr(result.find("is")+3));
                ss >> pan_max;
                
                result = Command("TN\n");
                ss.str(result.substr(result.find("is")+3));
                ss >> tilt_min;
                
                result = Command("TX\n");
                ss.str(result.substr(result.find("is")+3));
                ss >> tilt_max;
                
                result = Command("PL\n");
                ss.str(result.substr(result.find("is")+3));
                ss >> pan_min_speed;
                
                result = Command("PU\n");
                ss.str(result.substr(result.find("is")+3));
                ss >> pan_max_speed;
                
                result = Command("TL\n");
                ss.str(result.substr(result.find("is")+3));
                ss >> tilt_min_speed;
                
                result = Command("TU\n");
                ss.str(result.substr(result.find("is")+3));
                ss >> tilt_max_speed;
            }
            

            void VelocityMode(bool b)
            {
                if(b)
                    Command("CV\n");
                else
                    Command("CI\n");
            }
            
            void Reset()
            {
                port.Write(std::string("R\n"));
                std::vector<char> ret(512);
                gz4d::Sleep(seconds(30.0));
                port.Read(ret);
            }
            

            void Pan(float pan, bool wait=true)
            {
                std::stringstream cmd;
                int pan_i = pan*degs_to_pan_units;
                cmd << "PP" << pan_i << "\n";
                Command(cmd.str());
                if(wait)
                    while(GetRawPan() != pan_i)
                        Sleep(seconds(0.1));
            }
            
            void Tilt(float tilt, bool wait=true)
            {
                std::stringstream cmd;
                int tilt_i = tilt * degs_to_tilt_units;
                cmd << "TP" << tilt_i << "\n";
                Command(cmd.str());
                if(wait)
                    while(GetRawTilt() != tilt_i)
                        Sleep(seconds(0.1));
            }
            
            void PanTilt(float pan, float tilt, bool wait=true)
            {
                std::stringstream cmd;
                int pan_i = pan*degs_to_pan_units;
                int tilt_i = tilt * degs_to_tilt_units;
                cmd << "PP" << pan_i << " " << "TP" << tilt_i << "\n";
                Command(cmd.str());
                if(wait)
                {
                    bool pan_good = false;
                    bool tilt_good = false;
                    while(!pan_good || !tilt_good)
                    {
                        Sleep(seconds(0.1));
                        if(!pan_good)
                            pan_good = (GetRawPan() == pan_i);
                        if(!tilt_good)
                            tilt_good = (GetRawTilt() == tilt_i);
                    }
                }
            }
            

            float GetPan()
            {
                return GetRawPan()/degs_to_pan_units;
            }
            
            float GetTilt()
            {
                return GetRawTilt()/degs_to_tilt_units;
            }
            

            void PanSpeed(float ps)
            {
                std::stringstream cmd;
                cmd << "PS" << int(ps*degs_to_pan_units) << "\n";
                Command(cmd.str());
            }
            
            void TiltSpeed(float ts)
            {
                std::stringstream cmd;
                cmd << "TS" << int(ts*degs_to_tilt_units) << "\n";
                Command(cmd.str());
            }
            
            void PanTiltSpeed(float ps, float ts)
            {
                std::stringstream cmd;
                cmd << "PS" << int(ps*degs_to_pan_units) << " " << "TS" << int(ts*degs_to_tilt_units) << "\n";
                Command(cmd.str());
            }
            

            void PanAcceleration(float pa)
            {
                std::stringstream cmd;
                cmd << "PA" << int(pa*degs_to_pan_units) << "\n";
                Command(cmd.str());
            }
            
            void TiltAcceleration(float ta)
            {
                std::stringstream cmd;
                cmd << "TA" << int(ta*degs_to_tilt_units) << "\n";
                Command(cmd.str());
            }
            
            void PanBaseSpeed(float pb)
            {
                std::stringstream cmd;
                cmd << "PB" << int(pb*degs_to_pan_units) << "\n";
                Command(cmd.str());
            }
            
            void TiltBaseSpeed(float tb)
            {
                std::stringstream cmd;
                cmd << "TB" << int(tb*degs_to_tilt_units) << "\n";
                Command(cmd.str());
            }
            

            float GetPanMin() const
            {
                return pan_min/degs_to_pan_units;
            }
            
            float GetPanMax() const
            {
                return pan_max/degs_to_pan_units;
            }
            
            float GetTiltMin() const
            {
                return tilt_min/degs_to_tilt_units;
            }
            
            float GetTiltMax() const
            {
                return tilt_max/degs_to_tilt_units;
            }
            
            float GetPanMinSpeed() const
            {
                return pan_min_speed/degs_to_pan_units;
            }
            
            float GetPanMaxSpeed() const
            {
                return pan_max_speed/degs_to_pan_units;
            }
            
            float GetTiltMinSpeed() const
            {
                return tilt_min_speed/degs_to_tilt_units;
            }
            
            float GetTiltMaxSpeed() const
            {
                return tilt_max_speed/degs_to_tilt_units;
            }
            
    };
}

#endif
