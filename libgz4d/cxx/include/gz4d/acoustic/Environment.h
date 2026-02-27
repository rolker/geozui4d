#ifndef GZ4D_ACOUSTIC_ENVIRONMENT_H
#define GZ4D_ACOUSTIC_ENVIRONMENT_H

#include <gz4d/gz4d.h>

namespace gz4d
{
    namespace acoustic
    {
        /// \ingroup acoustic
        struct Environment
        {
            /// Sound speed in m/s
            double c;

            Environment():c(1500.0){}
            Environment(double c):c(c){}
        };
    }
}

#endif
