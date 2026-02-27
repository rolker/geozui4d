#ifndef GZ4D_IMU_FILTER_H
#define GZ4D_IMU_FILTER_H

#include <gz4d/IMU/Calibration.h>
#include <gz4d/thread/Producer.h>
#include <gz4d/SimpleKalman.h>
#include <boost/circular_buffer.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/stats.hpp>


namespace gz4d
{
    namespace IMU
    {
        class Filter: public Producer<State>
        {
            Calibration c;

            SimpleKalman<double> tk;
            std::vector<SimpleKalman<double> > mk;
            std::vector<SimpleKalman<double> > ak;
            std::vector<SimpleKalman<double> > gk;
            boost::circular_buffer<RawState> buffer;

            public:
                Filter()
                :buffer(15),mQ(0.00005),aQ(0.01),gQ(0.1),mR(15.0),aR(1.0),gR(1.0)
                {
                }
                
                Filter(Calibration const &cal)
                :c(c),buffer(15),mQ(0.00005),aQ(0.01),gQ(0.1),mR(15.0),aR(1.0),gR(1.0)
                {
                }
                

                void Consume(RawState raw)
                {
                    Send(operator()(raw));
                }
                
                State operator()(RawState const &raw)
                {
                    State ret;
                    if (mk.size() == 0)
                    {
                        tk = SimpleKalman<double>((raw.time-c.samplePeriod).secondsSinceEpoch().asSeconds(),c.samplePeriod.asSeconds(),c.timeVariance,0.0);
                        mk.push_back(SimpleKalman<double>(raw.mag[0],0.0,mR,mQ));
                        mk.push_back(SimpleKalman<double>(raw.mag[1],0.0,mR,mQ));
                        mk.push_back(SimpleKalman<double>(raw.mag[2],0.0,mR,mQ));
                        
                        ak.push_back(SimpleKalman<double>(raw.accel[0],0.0,aR,aQ));
                        ak.push_back(SimpleKalman<double>(raw.accel[1],0.0,aR,aQ));
                        ak.push_back(SimpleKalman<double>(raw.accel[2],0.0,aR,aQ));
                        
                        gk.push_back(SimpleKalman<double>(raw.gyro[0],0.0,gR,gQ));
                        gk.push_back(SimpleKalman<double>(raw.gyro[1],0.0,gR,gQ));
                        gk.push_back(SimpleKalman<double>(raw.gyro[2],0.0,gR,gQ));
                    }
                    ret.time = Time(tk(raw.time.secondsSinceEpoch().asSeconds()));
                    //std::cerr << tk.GetState().x << "\t" << tk.GetState().P << std::endl;
                    
                    buffer.push_back(raw);
                    
                    using namespace boost::accumulators;
                    
                    accumulator_set<float, stats<tag::lazy_variance> > magxacc,magyacc,magzacc;
                    for (boost::circular_buffer<RawState>::iterator i = buffer.begin(); i!= buffer.end(); ++i)
                    {
                        magxacc(static_cast<float>(i->mag[0]));
                        magyacc(static_cast<float>(i->mag[1]));
                        magzacc(static_cast<float>(i->mag[2]));
                    }
                    for (int i = 0; i<3; ++i)
                    {
                        mk[i].GetParams().Q(0,0)=mQ;
                        mk[i].GetParams().Q(1,1)=mQ;
                    }
                    //ret.mag[0] = mk[0](raw.mag[0],std::max(mR,variance(magxacc)));
                    //ret.mag[1] = mk[1](raw.mag[1],std::max(mR,variance(magyacc)));
                    //ret.mag[2] = mk[2](raw.mag[2],std::max(mR,variance(magzacc)));
                    
                    /*    accumulator_set<float, stats<tag::lazy_variance> > axacc,ayacc,azacc;
                     *        for(boost::circular_buffer<RawState>::iterator i = buffer.begin(); i!= buffer.end(); ++i)
                     *        {
                     *            axacc(static_cast<float>(i->accel[0]));
                     *            ayacc(static_cast<float>(i->accel[1]));
                     *            azacc(static_cast<float>(i->accel[2]));
                     }
                     ret.accel[0] = ak[0](raw.accel[0],variance(axacc));
                     ret.accel[1] = ak[1](raw.accel[1],variance(ayacc));
                     ret.accel[2] = ak[2](raw.accel[2],variance(azacc));*/
                    for (int i = 0; i<3; ++i)
                    {
                        ak[i].GetParams().Q(0,0)=aQ;
                        ak[i].GetParams().Q(1,1)=aQ;
                    }
                    
                    ret.accel[0] = ak[0](raw.accel[0],aR);
                    ret.accel[1] = ak[1](raw.accel[1],aR);
                    ret.accel[2] = ak[2](raw.accel[2],aR);
                    
                    /*    accumulator_set<float, stats<tag::lazy_variance> > gxacc,gyacc,gzacc;
                     *        for(boost::circular_buffer<RawState>::iterator i = buffer.begin(); i!= buffer.end(); ++i)
                     *        {
                     *            gxacc(static_cast<float>(i->gyro[0]));
                     *            gyacc(static_cast<float>(i->gyro[1]));
                     *            gzacc(static_cast<float>(i->gyro[2]));
                     }
                     ret.gyro[0] = gk[0](raw.gyro[0],variance(gxacc));
                     ret.gyro[1] = gk[1](raw.gyro[1],variance(gyacc));
                     ret.gyro[2] = gk[2](raw.gyro[2],variance(gzacc));*/
                    for (int i = 0; i<3; ++i)
                    {
                        gk[i].GetParams().Q(0,0)=gQ;
                        gk[i].GetParams().Q(1,1)=gQ;
                    }
                    ret.gyro[0] = gk[0](raw.gyro[0],gR);
                    ret.gyro[1] = gk[1](raw.gyro[1],gR);
                    ret.gyro[2] = gk[2](raw.gyro[2],gR);
                    
                    return ret;
                }
                

                float mQ, aQ, gQ;
                float mR, aR, gR;
        };
    }
}

#endif
