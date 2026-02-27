#ifndef GZ4D_DTAG_DTGMULTISENSORSTREAM_H
#define GZ4D_DTAG_DTGMULTISENSORSTREAM_H

#include <gz4d/dtag.h>
#include <gz4d/dtag/DataStream.h>
#include <vector>

namespace gz4d
{
    namespace dtag
    {
        class MultiSensorStream: public DataStream
        {
            public:
                struct SampleSet
                {
                    float period;
                    float sampleTime;
                    uint16_t channelCount;
                    std::vector<std::vector<int16_t> > samples; ///< Vector containing one sample of each sensor

                    typedef std::shared_ptr<SampleSet> Ptr;
                };

                MultiSensorStream()
                : DataStream(0x20)
                {
                }
                
                SampleSet::Ptr Next()
                {
                    do
                    {
                        DataStream::Next();
                    }
                    while(currentChunk && currentChunk.IsConfig());
                    
                    if(currentChunk)
                    {
                        std::vector<int16_t> data = currentChunk.ReadVector<int16_t>(sensorCount*currentChunk.GetSampleCount());
                        SampleSet::Ptr ret(new SampleSet);
                        ret->period = period/1000.0;
                        ret->sampleTime = currentChunk.GetStartTime()/1000.0;
                        ret->channelCount = sensorCount;
                        for(std::vector<int16_t>::iterator i = data.begin(); i != data.end(); i += sensorCount)
                            ret->samples.push_back(std::vector<int16_t>(i,i+sensorCount));
                        return ret;
                    }
                    return SampleSet::Ptr();
                }
                
                std::string GetLabel(int i) const 
                {
                    std::string sensorLabels[] = {
                        "AccelerometerX",
                        "AccelerometerY",
                        "AccelerometerZ",
                        "MagnetometerX",
                        "MagnetometerY",
                        "MagnetometerZ",
                        "Pressure",
                        "Temperature",
                        "SaltWaterSensor",
                        "BatteryVoltage",
                        "MagnetometerBridgeVoltage",
                        "PressureBridgeVoltage"
                    };
                    
                    return sensorLabels[i];
                }
                
                uint16_t GetSensorCount() const {return sensorCount;}
                using DataStream::operator=;
            private:
                void ParseConfig()
                {
                    period = currentConfig.Read<uint16_t>(0);
                    sensorCount = currentConfig.Read<uint16_t>(2);
                    nBits = currentConfig.Read<uint16_t>(4);
                    adcType = currentConfig.Read<uint16_t>(6);
                    compressionType = currentConfig.Read<uint16_t>(8);
                    blockLength = currentConfig.Read<uint16_t>(10);
                    overlap = currentConfig.Read<uint16_t>(12);
                }
                
    
            // data from config chunk
                uint16_t period;
                uint16_t sensorCount;
                uint16_t nBits;
                uint16_t adcType;
                uint16_t compressionType;
                uint16_t blockLength;
                uint16_t overlap;
    
            //sample
                enum SensorChannel
                {
                    AccelerometerX = 0,
                    AccelerometerY,
                    AccelerometerZ,
                    MagnetometerX,
                    MagnetometerY,
                    MagnetometerZ,
                    Presssure,
                    Temperature,
                    SaltWaterSensor,
                    BatteryVoltage,
                    MagnetometerBridgeVoltage,
                    PressureBridgeVoltage
                };

        };
    }
}

#endif
