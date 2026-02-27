#ifndef GZ4D_OGR_ENVELOPE_H
#define GZ4D_OGR_ENVELOPE_H

#include <gz4d/ogr/common.h>
#include <memory>
#include <gz4d/Box.h>
#include <ogr_core.h>

namespace gz4d
{
    namespace ogr
    {
        class Layer;
        class Geometry;

        class Envelope
        {
            friend class Layer;
            friend class Geometry;

            std::shared_ptr<OGREnvelope> envelope;
            public:
                Envelope()
                :envelope(new OGREnvelope)
                {
                }
                
                Envelope(Envelope const &e)
                :envelope(new OGREnvelope)
                {
                    envelope->Merge(*(e.envelope));
                }
                
                Envelope(OGREnvelope const &e)
                :envelope(new OGREnvelope)
                {
                    envelope->Merge(e);
                }
                
                Envelope(Box<Vector<double,2> > const &b)
                :envelope(new OGREnvelope)
                {
                    envelope->Merge(b.getMin()[0],b.getMin()[1]);
                    envelope->Merge(b.getMax()[0],b.getMax()[1]);
                }
                

                operator bool()
                {
                    return envelope->IsInit();
                }
                
                void Merge(Envelope const &o)
                {
                    envelope->Merge(*(o.envelope));
                }
                
                void Merge(double x, double y)
                {
                    envelope->Merge(x,y);
                }
                
                bool Intersects(Envelope const &o) const
                {
                    return envelope->Intersects(*(o.envelope));
                }

                bool Contains(Envelope const &o) const
                {
                    return envelope->Contains(*(o.envelope));
                }

                operator Box< Vector<double,2> >() const
                {
                    Box<Vector<double,2> > ret;
                    ret.setMin(Vector<double,2>(envelope->MinX,envelope->MinY));
                    ret.setMax(Vector<double,2>(envelope->MaxX,envelope->MaxY));
                    return ret;
                }
        };
    }
}

#endif
