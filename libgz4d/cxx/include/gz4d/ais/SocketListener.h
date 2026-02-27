#ifndef GZ4D_AIS_SOCKETLISTENER_H
#define GZ4D_AIS_SOCKETLISTENER_H


#include "Message.h"
#include <gz4d/socket/TCPSocket.h>
#include <gz4d/socket/SocketReceiver.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/data/NMEA.h>
#include <gz4d/URI.h>

namespace gz4d
{
    namespace ais
    {
        /// Listens for AIS messages encoded in VDM/VDO NMEA strings and returns decoded messages in thread-aware queue.
        class SocketListener:  public Producer<Message>, protected Consumer<SocketListener,SocketReceiver::output_type>
        {
            Socket _socket;
            SocketReceiver::Ptr _receiver;
            
            std::string _data;
            
            bool _uscg_nais_format;

            int _message_id_filter;
            int _mmsi_filter;
            int _dac_filter;
            int _fi_filter;
            
            std::map<std::string,std::vector<std::string> > _fragments;
            
        public:
            /// Constructor from a socket receiving NMEA strings.
            /// Threads will be launched for packet reception and message decoding.
            /// @param s: open socket.
            /// @param uscg_nais: flag indicating nais stream where extra fields are appended to distinguish multiple receiver stations
            SocketListener(Socket const &s, bool uscg_nais = false):_socket(s),_uscg_nais_format(uscg_nais),
                _message_id_filter(0),_mmsi_filter(0),_dac_filter(0),_fi_filter(0)
            {
                _receiver = SocketReceiver::Ptr(new SocketReceiver(_socket));
                _receiver->AddOutQueue(GetInQueue());
            }
            
            SocketListener(URI const &uri):_uscg_nais_format(false),
                _message_id_filter(0),_mmsi_filter(0),_dac_filter(0),_fi_filter(0)
            {
                URI::QueryMap opts = uri.getQueryMap();
                if(opts.count("uscg_nais"))
                    _uscg_nais_format = opts["uscg_nais"].empty() || IsTrue(opts["uscg_nais"]);
                _message_id_filter = atoi(opts["message_id"].c_str());
                _mmsi_filter = atoi(opts["mmsi"].c_str());
                _dac_filter = atoi(opts["dac"].c_str());
                _fi_filter = atoi(opts["fi"].c_str());
                
                _receiver = SocketReceiver::Ptr(new SocketReceiver(uri));
                _receiver->AddOutQueue(GetInQueue());
            }

        protected:
            void Consume(SocketReceiver::output_type p)
            {
                _data += std::string(p->begin(),p->end());
                while(true)
                {
                    std::size_t cr = _data.find('\n');
                    if(cr == std::string::npos)
                        break;
                    std::string line = _data.substr(0,cr);
                    _data = _data.substr(cr+1);
                    
                    nmea::Sentence nmea_str;
                    try
                    {
                        nmea_str = nmea::Sentence(line,true);
                    }
                    catch(Exception)
                    {
                    }
                    
                    if(nmea_str.IsType("A?VD?"))
                    {
                        nmea::VDM vdm(nmea_str);
//                         if(vdm.fragment_count > 1)
//                         {
//                             std::cerr << "\t" <<  nmea_str.operator()();
//                             //std::cerr << vdm.payload << std::endl;
//                         }
                        
                        std::string station = "";
                        std::string r_timestamp = "";
                        
                        if(_uscg_nais_format)
                        {
                            std::string extras = nmea_str.getExtras();
                            //std::cerr << extras << std::endl;
                            std::vector<std::string> fields;
                            boost::algorithm::split(fields,extras,boost::algorithm::is_any_of(","));
                            for(std::vector<std::string>::iterator f = fields.begin(); f != fields.end(); ++f)
                            {
                                if(f->size() && ((*f)[0] == 'r' || (*f)[0] == 'b' ))
                                {
                                    station = *f;
                                    break;
                                }
                            }
                            r_timestamp = fields.back();
                        }
                        
                        station += ",";
                        station += vdm.channel;
                        
                        
                        std::string payload;
                        if (vdm.fragment_count == 1)
                        {
                            payload = vdm.payload;
                        }
                        else
                        {
                            //std::cerr << "\t" << station << std::endl;

                            if(vdm.fragment_number == 1)
                                _fragments[station].clear();
                            
                            if(_fragments.count(station))
                                _fragments[station].push_back(vdm.payload);
                            
                            if(vdm.fragment_number == vdm.fragment_count)
                            {
                                if(_fragments.count(station) && _fragments[station].size() == vdm.fragment_count)
                                {
                                    for (std::vector<std::string>::iterator i = _fragments[station].begin(); i != _fragments[station].end(); ++i)
                                        payload += *i;
                                }
                                _fragments[station].clear();
                                //std::cerr << "payload: (" << payload.size() << ") " << payload << std::endl;
                            }
                            
                        }

                        if(payload.size())
                        {
                            AisMsg* m = decode(payload);
                            if(m)
                            {
                                if(m->had_error())
                                    std::cerr << "Error: " << m->get_error() << std::endl;
                                
                                bool skip = !((_message_id_filter == 0 || m->message_id == _message_id_filter) && (_mmsi_filter == 0 || m->mmsi == _mmsi_filter));
                                
                                if(!skip && (_dac_filter || _fi_filter))
                                {
                                    if(m->message_id == 8)
                                    {
                                            Ais8* ais8 = dynamic_cast<Ais8*>(m);
                                            skip = skip || (_dac_filter && ais8->dac != _dac_filter);
                                            skip = skip || (_fi_filter && ais8->fi != _fi_filter);
                                    }
                                }
                                
                                if(skip)
                                    delete m;
                                else
                                {
                                    gz4d::ais::Message msg;
                                    msg._message = std::shared_ptr<AisMsg>(m);
                                    msg._channel = vdm.channel;
                                    msg._vdm_msg_id = vdm.message_id;
                                    msg._timestamp = time::Now();
                                    if(station.size() > 2)
                                        msg._receiver = station.substr(0,station.size()-2);
                                    if(!r_timestamp.empty())
                                        msg._receiver_timestamp = gz4d::Time(boost::lexical_cast<double>(r_timestamp));
                                    msg._payload = payload;
                                    Send(msg);
                                }
                            }
                            //else
                              //  std::cerr << "skipping null decoded payload: " << payload << std::endl;
                        }
                    }
                }
            }
        };
    }
}

#endif
