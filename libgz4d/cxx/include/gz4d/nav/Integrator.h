#ifndef GZ4D_NAV_INTEGRATOR_H
#define GZ4D_NAV_INTEGRATOR_H

#include <gz4d/thread/Producer.h>
#include <gz4d/time.h>

namespace gz4d
{
    namespace nav
    {

        template<typename DT, typename NT, typename NCT> class Integrator: public Producer<DT>
        {
            NCT navContainer;
            gz4d::time::Duration timeout;
            public:
                typedef std::shared_ptr<Integrator> Ptr;

                Integrator(NCT nc, double timeout=5.0);
                void Consume(DT dt);
        };

        template<typename DT, typename NT, typename NCT> inline Integrator<DT,NT,NCT>::Integrator(NCT nc, double timeout)
            :navContainer(nc),timeout(timeout)
            {
            }

        template<typename DT, typename NT, typename NCT> inline void Integrator<DT,NT,NCT>::Consume(DT dt)
        {
            NT nav = (*navContainer)(dt->Timestamp(),timeout);
            dt->SetNavigation(nav);
            Producer<DT>::Send(dt);
        }

    }
}

#endif
