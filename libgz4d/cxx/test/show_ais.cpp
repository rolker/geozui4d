// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/ais/SocketListener.h>
#include <gz4d/socket/TCPSocket.h>
#include <gz4d/time/ios.h>
#include <gz4d/ais/History.h>
#include<gz4d/glut/Application.h>

class MyScene: public gz4d::glut::Application<MyScene,gz4d::glut::Ortho<> >, public gz4d::ais::History
{
    gz4d::ais::SocketListener::output_queue_type queue;
    
public:
    MyScene(gz4d::ais::SocketListener::output_queue_type q):queue(q)
    {
    }
    
    void Draw()
    {
        glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1.0,1.0,1.0);
        gz4d::Time now = gz4d::time::Now();
        glPointSize(2.5);
        glBegin(GL_POINTS);
        
        for(std::map<int,gz4d::ais::MMSIHistory>::iterator m = _targets.begin(); m != _targets.end(); ++m)
        {
            if(m->second.hasPosition())
            {
                gz4d::ais::Position p = m->second.latestPosition();
                gz4d::time::Duration age = now- m->second.latestPositionTime();
                if(age < gz4d::seconds(1))
                {
                    float i = 1.0- age.asSeconds()/1.0;
                    glColor3f(i,i,i);
                    glVertex2d(p.longitude/180.0,p.latitude/180.0);
                }
            }
        }
        
        for(BaseStationHistoryMap::iterator m = _baseStations.begin(); m != _baseStations.end(); ++m)
        {
            if(m->second.hasReports())
            {
                gz4d::ais::DateTime r = m->second.latestReport();
                gz4d::time::Duration age = now- m->second.latestReportTime();
                if(age < gz4d::seconds(1))
                {
                    float i = .5+(1.0- age.asSeconds()/1.0)*.5;
                    glColor3f(i,0,0);
                    
                }
                else
                    glColor3f(.5,0.0,0.0);
                glVertex2d(r.longitude/180.0,r.latitude/180.0);
            }
        }
        
        glEnd();
        SwapBuffers();
    }
    
    void Idle()
    {

        while(!queue->empty())
        {
            gz4d::ais::Message m = queue->pop();
            if(m._message->mmsi <= 0)
            {
                std::cerr << "non-positive mmsi: " << m._message->mmsi << " msg id: " << m._message->message_id << std::endl;
            }
            else
            {
//                 if(!history.count(m._message->mmsi))
//                 {
//                     std::cerr << history.size() + 1<< " different mmsis" << std::endl;
//                 }
                insert(m);
            }
        }
        boost::this_thread::yield();
        PostRedisplay();
    }
};

int main(int argc, char *argv[])
{
    // Host: data.aishub.net TCP port: 4349

    std::vector<std::string> sources;
    
    if (argc == 1)
    {
        sources.push_back("tcp://datalogger1.ccom.nh:31414?uscg_nais");
        sources.push_back("tcp://datalogger1.ccom.nh:31415");
    }
    
    for(int i = 1; i < argc; ++i)
        sources.push_back(argv[i]);
    
    
    std::vector<std::shared_ptr<gz4d::ais::SocketListener> > listeners;

    gz4d::ais::SocketListener::output_queue_type q;
    
    for(std::vector<std::string>::const_iterator s = sources.begin(); s != sources.end(); ++s)
    {
        std::shared_ptr<gz4d::ais::SocketListener> l(new gz4d::ais::SocketListener(*s));
        if (listeners.empty())
            q = l->NewOutQueue();
        else
            l->AddOutQueue(q);
        listeners.push_back(l);
    }

    MyScene scene(q);
    scene("AIS");
    
    return 0;
}


