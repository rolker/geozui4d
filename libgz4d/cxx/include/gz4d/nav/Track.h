// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_NAV_TRACK_H
#define GZ4D_NAV_TRACK_H

#include <gz4d/gz4d.h>
#include <gz4d/geo/Position.h>
#include <gz4d/data/NMEA.h>
#include <gz4d/TimeSequence.h>
#include <gz4d/nav/Fix.h>
#include <gz4d/thread/Producer.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/Box.h>
#include <gz4d/data/binary/Wrapper.h>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <gz4d/time/ios.h>

namespace gz4d
{
    namespace nav
    {
        class Track: public Producer<Fix::Ptr>
        {
            TimeSequence<geo::Position<double> > positions;
            TimeSequence<double> zs;
            TimeSequence< Angle<double,Degree> > headings;
            TimeSequence< Angle<double,Degree, ZeroCenteredPeriod> > pitches;
            TimeSequence< Angle<double,Degree, ZeroCenteredPeriod> > rolls;

            TimeSequence< Angle<double,Degree> > cmgs;
            TimeSequence<double> sogs;

            TimeSequence<Time> gpsTimes;

            bool useInertial;

            std::mutex mutex;
            std::condition_variable new_nav;
            Time latestTime;
            nmea::ZDA lastZDA;
            nmea::PSXN last_PSXN_20;

            std::string _label;
            
            struct File
            {
                std::string _filename;
                int _timestamp_column;
                int _nmea_column;
                std::string _separator;
                
                File(std::string const &f):_filename(f),_timestamp_column(-1),_nmea_column(-1)
                {
                }
                
                File(const boost::property_tree::ptree &f):_timestamp_column(-1),_nmea_column(-1)
                {
                    _filename = f.data();
                    boost::property_tree::ptree::const_assoc_iterator a = f.find("<xmlattr>");
                    if(a != f.not_found())
                    {
                        _timestamp_column = a->second.get("timestamp_column",_timestamp_column);
                        _nmea_column = a->second.get("nmea_column",_nmea_column);
                        _separator = a->second.get("separator",_separator);
                    }
                }
                
                void save(std::ofstream& s) const
                {
                    s << "    <File ";
                    if(_timestamp_column >= 0)
                        s << "timestamp_column=\""<<_timestamp_column<<"\" ";
                    if(_nmea_column >= 0)
                        s << "nmea_column=\""<<_nmea_column<<"\" ";
                    if(!_separator.empty())
                        s << "separator=\""<<_separator<<"\" ";
                    s << ">" << _filename << "</File>\n";
                }
                
            };
            
            std::vector<File> _files;
            
            void loadFile(File const &f)
            {
                std::ifstream in(f._filename.c_str());
                std::string line;
                while(std::getline(in,line))
                {
                    std::string n;
                    if(f._timestamp_column >=0||f._nmea_column>0)
                    {
                        if(f._separator != "")
                        {
                            std::vector<std::size_t> seps;
                            std::size_t s = 0;
                            while(s != std::string::npos)
                            {
                                seps.push_back(s);
                                s = line.find_first_of(f._separator,s);
                                if(s != std::string::npos)
                                    s += 1;
                            }
                            seps.push_back(line.size()+1);
                            
                            n = line.substr(seps[f._nmea_column],seps[f._nmea_column+1]-seps[f._nmea_column]-1);
                            
                            if(f._timestamp_column >=0)
                            {
                                Time t = boost::lexical_cast<Time>(line.substr(seps[f._timestamp_column],seps[f._timestamp_column+1]-seps[f._timestamp_column]-1));
                                
                                try
                                {
                                    Consume(t,nmea::Sentence(n));
                                }
                                catch(...)
                                {
                                }
                            }
                            else
                            {
                                try
                                {
                                    Consume(nmea::Sentence(n));
                                }
                                catch(...)
                                {
                                }
                            }
                        }
                    }
                    else
                    {
                        std::size_t nstart = line.find_first_of("$!");
                        if(nstart != std::string::npos)
                            try
                            {
                                Consume(nmea::Sentence(line.substr(nstart)));
                            }
                            catch(...)
                            {
                            }
                    }
                }
                
            }
            
            
            public:
                typedef std::shared_ptr<Track> Ptr;
                typedef Consumer<Track,std::pair<Time,nmea::Sentence::Ptr> > consumer_type;

                Track()
                :useInertial(false)
                {
                }
                
                Track(std::string const &fn)
                :useInertial(false)
                {
                    _files.push_back(File(fn));
                    loadFile(_files.back());
                    std::ifstream in(fn.c_str());
                    std::string line;
                    while(std::getline(in,line))
                    {
                        std::size_t nstart = line.find_first_of("$!");
                        if(nstart != std::string::npos)
                            try
                            {
                                Consume(nmea::Sentence(line.substr(nstart)));
                            }
                            catch(...)
                            {
                            }
                    }
                }

                Track(boost::property_tree::ptree const &n)
                :useInertial(false)
                {
                    boost::optional<std::string> a = n.get_optional<std::string>("<xmlattr>.label");
                    if(a)
                        _label = a.get();

                    for(boost::property_tree::ptree::const_iterator f = n.begin(); f != n.end(); ++f)
                    {
                        if(f->first == "File")
                            _files.push_back(File(f->second));
                    }
                    
                    for(std::vector< File >::iterator f=_files.begin(); f!=_files.end(); ++f)
                    {
                        loadFile(*f);
                    }
                    
                }
                
                
                void save(std::ofstream& s) const
                {
                    s << "  <Navigation label=\"" << _label << "\">\n";
                    for(std::vector< File >::const_iterator f=_files.begin(); f!=_files.end();++f)
                        f->save(s);
                    s << "  </Navigation>\n";
                }
                
                void loadFile(const std::string &fname)
                {
                    File f(fname);
                    _files.push_back(f);
                    loadFile(f);
                }
                    
                

                void Consume(binary::Wrapper<binary::Navigation::Position6DOF::Geographic> n)
                {
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        Time t = n->timestamp;
                        if(!latestTime.valid() || t > latestTime)
                            latestTime = t;
                        if(!IsNan(n->latitude))
                            positions[t].latitude = n->latitude;
                        if(!IsNan(n->longitude))
                            positions[t].longitude = n->longitude;
                        if(!IsNan(n->heading))
                            headings[t] = n->heading;
                        if(!IsNan(n->pitch))
                            pitches[t] = n->pitch;
                        if(!IsNan(n->roll))
                            rolls[t] = n->roll;
                        if(!IsNan(n->z))
                            zs[t] = n->z;
                        
                    }
                }
                
                void Consume(binary::Wrapper<binary::Navigation::Position::Geographic> n)
                {
                    {
                    	std::unique_lock<std::mutex> lock(mutex);
                        Time t = n->timestamp;
                        if(!latestTime.valid() || t > latestTime)
                            latestTime = t;
                        if(!IsNan(n->latitude))
                            positions[t].latitude = n->latitude;
                        if(!IsNan(n->longitude))
                            positions[t].longitude = n->longitude;
                    }
                }
                
                void Consume(std::pair<Time,nmea::Sentence::Ptr> n)
                {
                    nmea::Sentence::Ptr s = n.second;
                    Time t = n.first;
                    Consume(n.first,*(n.second));
                }
                
                void Consume(nmea::Sentence const &s)
                {
                    if (s.IsType("??ZDA"))
                    {
                        lastZDA = nmea::ZDA(s);
                    }
                    
                    if (s.IsType("??RMC"))
                    {
                        nmea::RMC rmc(s);
                        if(rmc.year > 0 && rmc.month > 0 && rmc.day > 0 && rmc.hour > 0 && rmc.minute > 0 && !IsNan(rmc.second))
                            latestTime = Time(rmc.year,rmc.month,rmc.day,rmc.hour,rmc.minute,rmc.second);
                    }
                    
                    if (s.IsType("??GGA") && (!useInertial || s.IsType("INGGA")))
                    {
                        nmea::GGA gga(s);
                        if(lastZDA.year > 0)
                            latestTime = Time(lastZDA.year,lastZDA.month,lastZDA.day,gga.hour,gga.minute,gga.second);
                    }
                    
                    if (s.IsType("PASHR"))
                    {
                        nmea::PASHR pashr(s);
                        if(lastZDA.year > 0)
                            latestTime = Time(lastZDA.year,lastZDA.month,lastZDA.day,pashr.hour,pashr.minute,pashr.second);
                    }
                    
                    if(latestTime.valid())
                        Consume(latestTime,s);
                }
                
                void Consume(Time t, nmea::Sentence const &s)
                {
                    if(!useInertial && s.IsType("IN???"))
                        useInertial = true;
                    
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        if(!latestTime.valid() || t > latestTime)
                            latestTime = t;
                        
                        if (s.IsType("??GGA") && (!useInertial || s.IsType("INGGA")))
                        {
                            nmea::GGA gga(s);
                            
                            positions[t].latitude = gga.latitude;
                            positions[t].longitude = gga.longitude;
                            if(lastZDA.year > 0)
                                gpsTimes[t] = Time(lastZDA.year,lastZDA.month,lastZDA.day,gga.hour,gga.minute,gga.second);
                            new_nav.notify_all();
                        }
                        
                        if (s.IsType("??HDT") && (!useInertial || s.IsType("INHDT")))
                        {
                            nmea::HDT hdt(s);
                            headings[t] = hdt.heading;
                            new_nav.notify_all();
                        }
                        
                        if (s.IsType("??HDG"))
                        {
                            nmea::HDG hdg(s);
                            headings[t] = hdg.magnetic_heading+hdg.magnetic_declination;
                            if(!IsNan(hdg.magnetic_deviation))
                                headings[t] += hdg.magnetic_deviation;
                            new_nav.notify_all();
                        }
                        
                        if (s.IsType("??RMC"))
                        {
                            nmea::RMC rmc(s);
                            if(!IsNan(rmc.latitude) && !IsNan(rmc.longitude))
                            {
                                positions[t].latitude = rmc.latitude;
                                positions[t].longitude = rmc.longitude;
                            }
                            if(rmc.year > 0 && rmc.month > 0 && rmc.day > 0 && rmc.hour > 0 && rmc.minute > 0 && !IsNan(rmc.second))
                                gpsTimes[t] = Time(rmc.year,rmc.month,rmc.day,rmc.hour,rmc.minute,rmc.second);
                            if(!IsNan(rmc.track))
                                cmgs[t] = rmc.track;
                            if(!IsNan(rmc.sog))
                                sogs[t] = rmc.sog;
                            new_nav.notify_all();
                        }
                        
                        if (s.IsType("PASHR"))
                        {
                            nmea::PASHR pashr(s);
                            pitches[t] = pashr.pitch;
                            rolls[t] = pashr.roll;
                            zs[t] = pashr.heave;
                            //gpsTimes[t] = Time(t.GetYear(),t.GetMonth(),t.GetDay(),pashr.hour,pashr.minute,pashr.second);
                            new_nav.notify_all();
                        }
                        
                        if(s.IsType("PSXN"))
                        {
                            nmea::PSXN psxn(s);
                            if(psxn.type == 20)
                                last_PSXN_20 = psxn;
                            if(psxn.type == 23)
                            {
                                if(last_PSXN_20.heading_quality == 0 || last_PSXN_20.heading_quality == 1)
                                    headings[t] = psxn.heading;
                                if(last_PSXN_20.roll_pitch_quality == 0 || last_PSXN_20.roll_pitch_quality == 1)
                                {
                                    pitches[t] = psxn.pitch;
                                    rolls[t] = psxn.roll;
                                }
                                if(last_PSXN_20.vertical_quality == 0 || last_PSXN_20.vertical_quality == 1)
                                {
                                    zs[t] = psxn.heave;
                                }
                                new_nav.notify_all();
                            }
                        }
                        
                        if (s.IsType("??ZDA"))
                        {
                            lastZDA = nmea::ZDA(s);
                        }
                        
                    }
                    if(!NoOutputQueue())
                        Send(operator()(t));
                }
                

                Fix::Ptr operator()(Time t)
                {
                	std::unique_lock<std::mutex> lock(mutex);
                    Fix::Ptr ret(new Fix);
                    ret->timestamp = t;
                    if(positions.InInterval(t))
                    {
                        geo::Position<double> p = positions.GetInterpolated(t);
                        ret->latitude = p.latitude;
                        ret->longitude = p.longitude;
                    }
                    
                    if(zs.InInterval(t))
                    {
                        ret->altitude = zs.GetInterpolated(t);
                    }
                    
                    if(headings.InInterval(t))
                    {
                        ret->heading = headings.GetInterpolated(t);
                    }
                    
                    if(pitches.InInterval(t))
                    {
                        ret->pitch = pitches.GetInterpolated(t);
                    }
                    
                    if(rolls.InInterval(t))
                    {
                        ret->roll = rolls.GetInterpolated(t);
                    }
                    return ret;
                }
                
                Fix::Ptr operator()(Time t, gz4d::time::Duration timeout)
                {
                    Fix::Ptr ret = operator()(t);
                    if(t.valid())
                    {
                        while(!ret->IsComplete())
                        {
                            {
                            	std::unique_lock<std::mutex> lock(mutex);
                                Time ln = latestTime;
                                if (ln >= t+timeout)
                                    return ret;
                                while(ln == latestTime)
                                    new_nav.wait(lock);
                            }
                            boost::thread::yield();
                            ret = operator()(t);
                        }
                    }
                    return ret;
                }
                

                float CMG(Time t)
                {
                    if(cmgs.InInterval(t))
                        return cmgs.GetInterpolated(t).value();
                    return Nan<float>();
                }
                
                float SOG(Time t)
                {
                    if(sogs.InInterval(t))
                        return sogs.GetInterpolated(t);
                    return Nan<float>();
                }
                

                Time GPSTime(Time t)
                {
                    if(gpsTimes.InInterval(t))
                        return gpsTimes.GetInterpolated(t);
                    return Time();
                }
                
                time::Duration GPSOffset()
                {
                    if(gpsTimes.empty())
                        return time::Duration();
                    
                    time::Duration ret(0.0);
                    for(TimeSequence<Time>::const_iterator t = gpsTimes.begin(); t != gpsTimes.end(); ++t)
                        ret += t->second - t->first;
                    return ret/double(gpsTimes.size());
                }
                

                Time FirstGPSTime()
                {
                    return gpsTimes.period().start();
                }
                
                Time LastGPSTime()
                {
                    return gpsTimes.period().finish();
                }
                

                Box2d GetBox(time::Period ts)
                {
                    Box2d ret;
                    TimeSequence<geo::Position<double> >::iterator_pair ip = positions.GetSpan(ts);
                    for(TimeSequence<geo::Position<double> >::const_iterator i = ip.first; i != ip.second; ++i)
                        ret.expand(Vector<double,2>(i->second.latitude.value(),i->second.longitude.value()));
                    return ret;
                }
                
                std::string const &getLabel() const
                {
                    return _label;
                }
                void setLabel(std::string const &l)
                {
                    _label = l;
                }
                
                TimeSequence<geo::Position<double> > const & getPositions() const
                {
                    return positions;
                }
                
                
                std::string getInfo() const
                {
                    std::stringstream ret;
                    if(!_label.empty())
                        ret << _label << std::endl;
                    Box<geo::Position<double> > bbox;
                    for(TimeSequence< gz4d::geo::Position< double > >::const_iterator p = positions.begin(); p != positions.end(); ++p)
                    {
                        bbox.expand(p->second);
                    }
                    ret << "time min: " << positions.begin()->first << " max: " << positions.rbegin()->first << std::endl;
                    ret << "position min: " << bbox.getMin().latitude.value() << "," << bbox.getMin().longitude.value() << " max: " << bbox.getMax().latitude.value() << "," << bbox.getMax().longitude.value() << std::endl;
                    
                    return ret.str();
                }
                
        };
    }
}

#endif
