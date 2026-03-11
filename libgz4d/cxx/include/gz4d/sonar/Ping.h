// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SONAR_PING_H
#define GZ4D_SONAR_PING_H


namespace gz4d
{
    namespace sonar
    {
        /// Generic Ping container
        /// PD is ping data type and NT is navigation type
        template <typename PD, typename NT> class Ping
        {
            Time timestamp;
            typename NT::Ptr navigation;
            PD data;
            public:
                typedef std::shared_ptr<Ping> Ptr;

                typedef PD data_type;
                typedef typename PD::channels_type channels_type;
                typedef typename PD::channels_type::value_type channel_type;


                Ping(){}
                Ping(Time t):timestamp(t){}
                Ping(PD const &ping_data, Time t):timestamp(t),data(ping_data){}

                PD &GetData() {return data;}
                channels_type &GetChannels() {return data.GetChannels();}

                typename NT::Ptr &GetNavigation() {return navigation;}
                void SetNavigation(typename NT::Ptr &nav){navigation = nav;}

                Time Timestamp() const {return timestamp;}
                void SetTimestamp(Time t) {timestamp = t;}
        };

    }
}

#endif
