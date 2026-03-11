// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_WIIMOTEMANAGER_H
#define GZ4D_WIIMOTEMANAGER_H

#include <gz4d/gz4d.h>

#include <gz4d/Wiimote.h>
#include <vector>
#include <map>

#include <iostream>
#include <gz4d/time.h>
#include <gz4d/socket/BTSocketManager.h>

#define WIIMOTE_NAME "Nintendo RVL-CNT-01"


namespace gz4d
{
    /**
    \ingroup devices
    */
    class WiimoteManager: public Producer<Wiimote::Ptr>
    {
        public:
            WiimoteManager(bool debug = false)
            :showDebug(debug)
            {
            }
            
            ~WiimoteManager()
            {
                if(scanThread)
                {
                    scanThread->interrupt();
                    scanThread->join();
                }
            }
            
            void Scan()
            {
                if(scanThread)
                {
                    scanThread->interrupt();
                    scanThread->join();
                }
                scanThread = std::shared_ptr<boost::thread>(new boost::thread(boost::bind(&WiimoteManager::ScanThread,this)));
            }
            
        private:
            void ScanThread()
            {
                BTSocketManager scansock;
                while(!boost::this_thread::interruption_requested())
                {
                    std::vector<BTDevice> devList = scansock.Scan(4,37,0,true);
                    if(showDebug)
                        std::cerr << "Found " << devList.size() << " devices.\n";
                    for (std::vector<BTDevice>::iterator i = devList.begin(); i != devList.end(); ++i)
                    {
                        if(showDebug)
                            std::cerr << i->name << " , " << i->address << std::endl;
                        Wiimote::Ptr w(new Wiimote);
                        w->open(i->address);
                        Send(w);
                    }
                    boost::this_thread::yield();
                }
            }
            
            std::shared_ptr<boost::thread> scanThread;
            bool showDebug;
    };
};
#endif
