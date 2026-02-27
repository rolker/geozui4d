#ifndef GZ4D_BEAMPATTERN_H
#define GZ4D_BEAMPATTERN_H

#include <gz4d/gz4d.h>
#include <gz4d/data/RasterMeta.h>
#include <gz4d/acoustic/Element.h>

namespace gz4d
{
    namespace acoustic
    {
        /// Constructs the 3D beam pattern of an acoustic device.
        /// A device's angular power response (dB re 1) is sampled and stored in a rectangular
        /// grid representing sperical coordinates.
        template <typename RasterType, typename RadianType>
        void BeamPattern(Element const &element, SphericalRaster<RasterType, RadianType> &raster, double w, Environment const &env, double amplitude = 1.0);
    }
}


template <typename RasterType, typename RadianType>
void gz4d::acoustic::BeamPattern(Element const &element, SphericalRaster<RasterType, RadianType> &raster, double w, Environment const &env, double amplitude )
{
    unsigned int rows = raster.GetRaster()->GetRows();
    unsigned int cols = raster.GetRaster()->GetColumns();
    Wave wave(w);
    wave.SetPhasor(Phasor(amplitude));
    for(unsigned int r = 0; r < rows; r++)
    {
        RadianType phi = raster.GetRowInterval().Map((double)r/(double)(rows-1));
        for(unsigned int c = 0; c < cols; c++)
        {
            RadianType theta = raster.GetColInterval().Map((double)c/(double)(cols-1));
            wave.SetDirection(Wave::Direction(theta,phi));
            RasterType resp = element.Response(wave, env);
            resp *= resp;
            resp = 10.0*log10(resp);
            raster.GetRaster()->GetRow(r)[c] = resp;
        }
    }
}

#endif
