// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_IMU_CALIBRATION_H
#define GZ4D_IMU_CALIBRATION_H

#include <gz4d/IMU.h>

namespace gz4d
{
    namespace IMU
    {
        struct Calibration
        {
            time::Duration samplePeriod;
            double timeVariance;

            Vector<float,3> mag_center;
            Vector<float,3> mag_axis;

            Vector<float,3> accel_neg1;
            Vector<float,3> accel_zero;
            Vector<float,3> accel_pos1;

            Calibration()
                :samplePeriod(0.01),timeVariance(0.000064),
                mag_center(564.937570236, 547.879546614, 537.122258728),
                mag_axis(22.5574159598, 23.2369038534, 17.6252277),
                accel_neg1(291.0,319.7,292.1),
                accel_zero(478.0,514.0,492.5),
                accel_pos1(663.3,702.6,680.1)
            {}
            template <typename T> State operator()(T const &s) const
            {
                State ret;
                ret.time = s.time;
                ret.mag[0] = (s.mag[0] - mag_center[0])/mag_axis[0];
                ret.mag[1] = -(s.mag[1] - mag_center[1])/mag_axis[1];
                ret.mag[2] = (s.mag[2] - mag_center[2])/mag_axis[2];

                if (s.accel[0] < accel_zero[0])
                    ret.accel[0] = -(s.accel[0]-accel_zero[0])/(accel_zero[0]-accel_neg1[0]);
                else
                    ret.accel[0] = -(s.accel[0]-accel_zero[0])/(accel_pos1[0]-accel_zero[0]);

                if (s.accel[1] < accel_zero[1])
                    ret.accel[1] = (s.accel[1]-accel_zero[1])/(accel_zero[1]-accel_neg1[1]);
                else
                    ret.accel[1] = (s.accel[1]-accel_zero[1])/(accel_pos1[1]-accel_zero[1]);

                if (s.accel[2] < accel_zero[2])
                    ret.accel[2] = -(s.accel[2]-accel_zero[2])/(accel_zero[2]-accel_neg1[2]);
                else
                    ret.accel[2] = -(s.accel[2]-accel_zero[2])/(accel_pos1[2]-accel_zero[2]);

                ret.gyro = s.gyro/1000.0;

                return ret;
            }

            // mags 564.937570236 547.879546614 537.122258728 22.5574159598 23.2369038534 17.6252277
            /*
            flat.csv             (477.62552426602758, 2.5350225681648908) (512.26842420611149, 2.3305847547405745) (680.10065907729177, 2.9856736219383033)
            flat_upside_down.csv (480.3038277511962, 20.493813099517052)  (526.93460925039869, 20.284400285911147) (292.11483253588517, 23.891119708797785)
            side_1.csv           (291.01373283395753, 2.9099237688220989) (508.96754057428217, 2.3510063107757153) (483.75031210986265, 2.6467695655087926)
            side_2.csv           (663.29351535836179, 2.5657258675115653) (510.11945392491469, 2.2041607939521684) (498.67576791808875, 3.9767848198581155)
            end_1.csv            (479.92100840336133, 3.434096462114264)  (702.57310924369744, 3.6328903326036572) (498.65042016806723, 4.7651888990890567)
            end_2.csv            (476.31681034482756, 3.0720449985136771) (319.65625, 3.2277411099137931)          (487.5625, 4.3495420258620685)
            */
        };
    }
}

#endif
