#ifndef IMU_H
#define IMU_H

#include "engine/Object.h"
#include <gz4d/IMU/BTSource.h>
#include <gz4d/IMU/Calibration.h>
#include <gz4d/IMU/Filter.h>
#include <gz4d/TimeSequence.h>

class IMU: public Object
{
    public:
        IMU();
        DERIVE_OBJECT(IMU);

        void DrawLowRes(float degradation, Window3D *win);
        void Idle(double time);

        void getHtmlInfo(std::stringstream &html, short level);
        int perform(int argc, const char **argv, Tcl_Interp *interp);
    private:
        void drawLines(std::vector<gz4d::Vector<float,3> > const &v, float colorGain, float xoffset, float yoffset);
        gz4d::IMU::BTSource imu;
        std::shared_ptr<gz4d::ThreadedQueue<gz4d::IMU::RawState> > dataQueue;

        gz4d::IMU::Calibration c;
        gz4d::IMU::Filter f;

        std::vector<gz4d::Vector<float,3> > mag_data;
        std::vector<gz4d::Vector<float,3> > mag_raw;
        std::vector<gz4d::Vector<float,3> > accel_data;
        std::vector<gz4d::Vector<float,3> > accel_raw;
        std::vector<gz4d::Vector<float,3> > gyro_data;
        std::vector<gz4d::Vector<float,3> > gyro_raw;

        std::size_t dindex;

        float fade;
        bool showMag;
        bool showAccel;
        bool showGyro;
};

#endif

