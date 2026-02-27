#ifndef GZ4D_SIMRAD_NET_CHANNEL_H
#define GZ4D_SIMRAD_NET_CHANNEL_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad/net/ParameterManager.h>
#include <gz4d/sonar/Simrad/net/SubscriptionManager.h>
#include <gz4d/sonar/Simrad/net/BottomDetection.h>
#include <gz4d/sonar/Simrad/net/TargetDetection.h>
#include <gz4d/sonar/Simrad/net/Sample.h>
#include <gz4d/sonar/Simrad/net/ParameterGroup.h>

namespace gz4d
{
    namespace Simrad
    {
        namespace net
        {

            class Channel
            {
                ParameterManager::Ptr parameterManager;
                SubscriptionManager::Ptr subscriptionManager;
    
                BottomDetectionSubscription::Ptr bottomDetection;
                TargetDetectionSubscription::Ptr targetDetection;
                SampleSubscription::Ptr samples;
    
                ParameterGroup::Map parameters;
    
                std::string name;
                void Subscribe()
                {
                    if(!subscribed)
                    {
                        parameters["channel"]->Add("frequency","TransceiverMgr/"+name+"/Frequency");
                        parameters["channel"]->Add("beamType","TransceiverMgr/"+name+"/BeamType");
                        parameters["channel"]->Add("sampleInterval","TransceiverMgr/"+name+"/SampleInterval");
                        parameters["channel"]->Add("transmitPower","TransceiverMgr/"+name+"/TransmitPower");
                        parameters["channel"]->Add("absorptionCoefficient","TransceiverMgr/"+name+"/AbsorptionCoefficient");
                        parameters["channel"]->Add("soundSpeed","TransceiverMgr/"+name+"/SoundVelocity");
                        parameters["channel"]->Add("transducerName","TransceiverMgr/"+name+"/TransducerName");
                        parameters["channel"]->Add("equivalentBeamAngle","TransceiverMgr/"+name+"/EquivalentBeamAngle");
                        parameters["channel"]->Add("angleSensitivityY","TransceiverMgr/"+name+"/AngleSensitivityAlongship");
                        parameters["channel"]->Add("angleSensitivityX","TransceiverMgr/"+name+"/AngleSensitivityAthwartship");
                        parameters["channel"]->Add("beamWidthY","TransceiverMgr/"+name+"/BeamWidthAlongship");
                        parameters["channel"]->Add("beamWidthX","TransceiverMgr/"+name+"/BeamWidthAthwartship");
                        parameters["channel"]->Add("directionX","TransceiverMgr/"+name+"/DirectionX");
                        parameters["channel"]->Add("directionY","TransceiverMgr/"+name+"/DirectionY");
                        parameters["channel"]->Add("gain","TransceiverMgr/"+name+"/Gain");
                        parameters["channel"]->Add("pulseLength","TransceiverMgr/"+name+"/PulseLength");
                        parameters["channel"]->Add("transducerDepth","TransceiverMgr/"+name+"/TransducerDepth");
                        subscribed = true;
                    }
                }
                
                bool subscribed;
                public:
                    typedef std::shared_ptr<Channel> Ptr;
                    typedef std::vector<Ptr> List;
    
                    Channel(ParameterManager::Ptr &p, SubscriptionManager::Ptr &s, ParameterGroup::Map const &params, std::string const &name)
                    :parameterManager(p),subscriptionManager(s),parameters(params),name(name)
                    {
                        parameters["channel"] = ParameterGroup::Ptr(new ParameterGroup(p));
                    }
                    
                    ~Channel(){}
    
                    std::string GetName(){return name;}
    
                    BottomDetectionSubscription::Ptr &GetBottomDetection()
                    {
                        if(!bottomDetection)
                        {
                            Subscribe();
                            bottomDetection = BottomDetectionSubscription::Ptr(new BottomDetectionSubscription(name,parameters));
                            bottomDetection->SetParameter("UpperDetectorLimit","5.0");
                            Subscription::Ptr sp(bottomDetection);
                            subscriptionManager->Subscribe(sp);
                        }
                        return bottomDetection;
                    }
                    
                    TargetDetectionSubscription::Ptr &GetTargetDetection()
                    {
                        if(!targetDetection)
                        {
                            Subscribe();
                            targetDetection = TargetDetectionSubscription::Ptr(new TargetDetectionSubscription(name,parameters));
                            Subscription::Ptr sp(targetDetection);
                            subscriptionManager->Subscribe(sp);
                        }
                        return targetDetection;
                    }
                    
                    SampleSubscription::Ptr &GetSamples(float range = 150.0)
                    {
                        if(!samples)
                        {
                            Subscribe();
                            samples = SampleSubscription::Ptr(new SampleSubscription(name,parameters));
                            samples->SetParameter("SampleDataType","Power");
                            samples->SetParameter("Range",boost::lexical_cast<std::string>(range));
                            
                            //samples->SetParameter("SampleDataType","Sv");
                            Subscription::Ptr sp(samples);
                            subscriptionManager->Subscribe(sp);
                        }
                        return samples;
                    }
                    
    
                    void UpdateSubscription(Subscription::Ptr &s)
                    {
                        if(s)
                            subscriptionManager->Update(s);
                    }
                    
                    ParameterGroup::Map const &GetParameters()
                    {
                        Subscribe();
                        return parameters;
                    }
                    
            };
        }
    }
}

#endif
