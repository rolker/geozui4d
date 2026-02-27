#ifndef GZ4D_RTSP_RTSP_H
#define GZ4D_RTSP_RTSP_H

#include <gz4d/gz4d.h>

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

#include <string>
#include <memory>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

//gz4d_cmake depends liveMedia

namespace gz4d
{
    namespace rtsp
    {
        class Rtsp
        {
            typedef std::shared_ptr<TaskScheduler>  TaskSchedulerPtr;
            typedef std::shared_ptr<Authenticator> AuthenticatorPtr;

                class QTSaver: public BasicUsageEnvironment{

                    TaskSchedulerPtr scheduler;
                    AuthenticatorPtr auth;

                    RTSPClient *rtspClient;
                    MediaSession *session;

                    //std::string qtFilename;
                    //QuickTimeFileSink *qt;
                    std::string outFilename;
                    Medium *outFileSink;

                    uint32_t bufferSize;
                    
                    boost::thread taskSchedulerThread;
                    char doneFlag;

                    struct RtspResult
                    {
                        RTSPClient * client;
                        int resultCode;
                        std::string resultString;
                        QTSaver *env;

                        RtspResult(RTSPClient * client, int rc, char * rs)
                        :client(client),resultCode(rc)
                        {
                            if(rs)
                            {
                                resultString = rs;
                                delete[] rs;
                            }
                            env = dynamic_cast<QTSaver*>(&client->envir());
                        }

                    };

                    void play(RtspResult const &res)
                    {
                        tick();
                    }

                    static void play(RTSPClient * client, int resultCode, char * resultString)
                    {

                        RtspResult res(client,resultCode,resultString);
                        res.env->play(res);
                    }

                    void outfileDone()
                    {
                    }

                    static void outfileDone(void * e)
                    {
                        QTSaver *s = reinterpret_cast<QTSaver *>(e);
                        s->outfileDone();
                    }

                    void setup(RtspResult const &res)
                    {
                        if(res.resultCode == 0){
                            if(outFilename.size() > 4){
                                std::string ext = outFilename.substr(outFilename.size()-3);
                                if(ext == "mov"){
                                    QuickTimeFileSink *qt = QuickTimeFileSink::createNew(*this, *session, outFilename.c_str(),bufferSize);
                                    qt->startPlaying(outfileDone,this);
                                    outFileSink = qt;
                                }
                                else
                                {
                                    AVIFileSink *avi = AVIFileSink::createNew(*this, *session, outFilename.c_str(),bufferSize);
                                    avi->startPlaying(outfileDone,this);
                                    outFileSink = avi;
                                }
                            }

                            res.client->sendPlayCommand(*session,play,0.0,-1.0,1.0,auth.get());
                        }

                    }

                    static void setup(RTSPClient * client, int resultCode, char * resultString)
                    {
                        RtspResult res(client,resultCode,resultString);
                        res.env->setup(res);
                    }

                    void describe(RtspResult const &res)
                    {
                        if(res.resultCode == 0){
                            
                            session = MediaSession::createNew(*this, res.resultString.c_str());

                            MediaSubsessionIterator iter(*session);
                            MediaSubsession *subsession;
                            subsession = iter.next();
                            subsession->initiate();

                            int socketNum = subsession->rtpSource()->RTPgs()->socketNum();
                            setReceiveBufferTo(*this, socketNum, bufferSize);

                            res.client->sendSetupCommand(*subsession,setup,False,False,False,auth.get());
                        }

                    }

                    static void describe(RTSPClient  * client, int resultCode, char * resultString)
                    {
                        RtspResult res(client,resultCode,resultString);
                        res.env->describe(res);
                    }

                    void teardown(RtspResult const &res)
                    {
                        Medium::close(outFileSink);
                        Medium::close(session);
                        Medium::close(rtspClient);
                        doneFlag = 1;
                    }

                    static void teardown(RTSPClient  * client, int resultCode, char * resultString)
                    {
                        RtspResult res(client,resultCode,resultString);
                        res.env->teardown(res);
                    }

                    void tick()
                    {
                        if (boost::this_thread::interruption_requested())
                        {
                            rtspClient->sendTeardownCommand(*session,teardown,auth.get());
                        }
                        else
                        {
                            scheduler->scheduleDelayedTask(100000,tick,this);
                        }
                    }
                    
                    static void tick(void * e)
                    {
                        QTSaver *s = reinterpret_cast<QTSaver *>(e);
                        s->tick();
                    }


                    public:
                        QTSaver(std::string const &url, AuthenticatorPtr auth, std::string const &fname, uint32_t bufferSize)
                        :BasicUsageEnvironment(*BasicTaskScheduler::createNew()),scheduler(&taskScheduler()),auth(auth),outFilename(fname),bufferSize(bufferSize)
                        {
                            rtspClient = RTSPClient::createNew(*this,url.c_str());
                            rtspClient->sendDescribeCommand(describe,auth.get());
                            
                            doneFlag = 0;
                            taskSchedulerThread = boost::thread(boost::ref(*this)).move();
                        }

                        ~QTSaver()
                        {
                            taskSchedulerThread.interrupt();
                            taskSchedulerThread.join();
                        }

                        void operator()()
                        {
                            scheduler->doEventLoop(&doneFlag);
                        }

                };

                
                typedef std::shared_ptr<QTSaver> QTSaverPtr;

                
                std::string url;
                std::shared_ptr<Authenticator> auth;
                QTSaverPtr qtSaver;
                uint32_t bufferSize;

            public:
                typedef std::shared_ptr<Rtsp> Ptr;
                
                Rtsp(std::string const &url, std::string const &uname="", std::string const &pwd="")
                :url(url),auth(new Authenticator),bufferSize(1000000)
                {
                    auth->setUsernameAndPassword(uname.c_str(),pwd.c_str());
                }

                void setBufferSize(uint32_t bs)
                {
                    bufferSize = bs;
                }

                uint32_t getBufferSize() const
                {
                    return bufferSize;
                }

                void save(std::string const &fname)
                {
                    qtSaver = QTSaverPtr(new QTSaver(url,auth,fname,bufferSize));
                }

                void stop()
                {
                    qtSaver.reset();
                }
        };

        
    }
}

#endif