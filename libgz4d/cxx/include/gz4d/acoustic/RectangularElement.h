#ifndef GZ4D_ACOUSTIC_RECTANGULARELEMENT_H
#define GZ4D_ACOUSTIC_RECTANGULARELEMENT_H

#include <gz4d/gz4d.h>
#include <gz4d/acoustic/Element.h>
#include <gz4d/acoustic/LineElement.h>

namespace gz4d
{
    namespace acoustic
    {
        /// Rectangular shaped acoustic element.
        /// \ingroup acoustic
        class RectangularElement: public Element
        {
            public:
                RectangularElement(){}
                RectangularElement(Size const &s):xLine(s[0]),yLine(s[1],M_PI/2.0){}
                RectangularElement(RectangularElement const &e):xLine(e.xLine),yLine(e.yLine){}

                /// Element response for given Wave Direction.
                /// Returns a response value according to Wave's 
                /// direction vector (K).
                /// see \ref acoustic for coordinate space details.
                virtual double Response(Wave const &wave, Environment const &env) const
                {
                    return xLine.Response(wave, env)*yLine.Response(wave, env);
                }

                virtual double beamWidth(double w, Environment const &env, double threshold = -3.0, orientation_type direction = 0.0)
                {
                    double est_eff_len = xLine.getLength()*fabs(cos(direction))+yLine.getLength()*fabs(sin(direction));
                    double theta = LineElement(est_eff_len).beamWidth(w,env,threshold);
                    return Element::beamWidth(w,env,threshold,direction,theta);
                }

            protected:
                LineElement xLine;
                LineElement yLine;
        };
    }
}

#endif
