#include <gz4d/IMU/BTSource.h>
#include <gz4d/IMU/Filter.h>
#include <gz4d/time.h>
#include<gz4d/glut/Application.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/Event.h>
#include <gz4d/vector/operations.h>

#include <iostream>

class IMUScene: public gz4d::EventHandler, public gz4d::glut::Application<IMUScene>
{
    gz4d::IMU::Filter filter;
    gz4d::IMU::BTSource imu;
    gz4d::IMU::Calibration c;

    std::shared_ptr<gz4d::ThreadedQueue<gz4d::IMU::RawState> > q;

    std::vector<gz4d::Vector<float,3> > mag_data;
    std::vector<gz4d::Vector<float,3> > mag_raw;
    std::vector<gz4d::Vector<float,3> > accel_data;
    std::vector<gz4d::Vector<float,3> > accel_raw;
    std::vector<gz4d::Vector<float,3> > gyro_data;
    std::vector<gz4d::Vector<float,3> > gyro_raw;

    std::vector<gz4d::Vector<float,3> > vlengths;

    gz4d::Vector<float,3> minmag;
    gz4d::Vector<float,3> maxmag;

    std::size_t dindex;

    gz4d::EventGroup HandleEvent(gz4d::Event const &e)
    {
        gz4d::EventGroup ret;
        if (e.type == gz4d::Event::Idle)
        {
            while (!q->empty())
            {
                gz4d::IMU::RawState rs = q->pop();
                gz4d::IMU::State fs = filter(rs);
                gz4d::IMU::State s = c(fs);
                gz4d::IMU::State rcs = c(rs);
                mag_data[dindex] = s.mag;
                accel_data[dindex] = s.accel;
                gyro_data[dindex] = s.gyro;
                mag_raw[dindex] = rcs.mag;
                accel_raw[dindex] = rcs.accel;
                gyro_raw[dindex] = rcs.gyro;

                vlengths[dindex][0] = norm(accel_data[dindex])-1.0;
                vlengths[dindex][1] = norm(mag_data[dindex])-1.0;

                for (int i = 0; i < 3; ++i)
                {
                    if (fs.mag[i] > maxmag[i]) maxmag[i] = fs.mag[i];
                    if (fs.mag[i] < minmag[i]) minmag[i] = fs.mag[i];
                    mag_data[dindex][i] = -1.0+2.0*(fs.mag[i]-minmag[i])/(maxmag[i]-minmag[i]);
                }

                ++dindex;
                if (dindex == mag_data.size())
                    dindex = 0;
                if (!ret.count(gz4d::Event::Draw))
                    ret.Add(gz4d::Event(gz4d::Event::Draw));
            }
        }
        return ret;
    }

    void drawLines(std::vector<gz4d::Vector<float,3> > const &v, float colorGain, float xmin, float xmax, float ymin, float ymax)
    {
        for (int i = 0; i < 3; ++i)
        {
            glBegin(GL_LINE_STRIP);
            for (std::vector<gz4d::Vector<float,3> >::const_iterator d = v.begin(); d != v.end(); ++d)
            {
                std::size_t j = d-v.begin();
                float c;
                if (j < dindex)
                    c = ((v.size()-dindex)+j)/float(v.size());
                else
                    c = (j-dindex)/float(v.size());
                c *= colorGain;

                switch (i)
                {
                case 0:
                    glColor3f(1.0*c,0.0,0.0);
                    break;
                case 1:
                    glColor3f(0.0,1.0*c,0.0);
                    break;
                default:
                    glColor3f(0.0,0.0,1.0*c);
                }
                glVertex3f(xmin+(xmax-xmin)*(d-v.begin())/float(v.size()),ymin+(ymax-ymin)/2.0+(*d)[i]*(ymax-ymin)/2.0,0.0);
            }
            glEnd();
        }
    }
    public:

    void Draw()
    {
        glDisable(GL_LIGHTING);
//         glPushMatrix();
//         glRotatef(-90.0,1.0,0.0,0.0);
//
//         glBegin(GL_LINES);
//
//         for (std::vector<gz4d::Vector<float,3> >::iterator d = mag_data.begin(); d != mag_data.end(); ++d)
//         {
//             std::size_t i = d-mag_data.begin();
//             float c;
//             if (i < dindex)
//                 c = ((mag_data.size()-dindex)+i)/float(mag_data.size());
//             else
//                 c = (i-dindex)/float(mag_data.size());
//
//             glColor3f(0.5*c,0.5*c,0.5*c);
//             glVertex3f(0.0,0.0,0.0);
//             glColor3f(1.0*c,1.0*c,1.0*c);
//             gz4d::glo::Vertex(*d);
//         }
//
//         for (std::vector<gz4d::Vector<float,3> >::iterator d = accel_data.begin(); d != accel_data.end(); ++d)
//         {
//             std::size_t i = d-accel_data.begin();
//             float c;
//             if (i < dindex)
//                 c = ((accel_data.size()-dindex)+i)/float(accel_data.size());
//             else
//                 c = (i-dindex)/float(accel_data.size());
//
//             glColor3f(0.125*c,0.125*c,0.5*c);
//             glVertex3f(0.0,0.0,0.0);
//             glColor3f(0.25*c,0.25*c,1.0*c);
//             gz4d::glo::Vertex(*d);
//
//         }
//
//         glEnd();
//         glPopMatrix();

//         glMatrixMode(GL_PROJECTION);
//         glPushMatrix();
//         glLoadIdentity();
//         glOrtho(-1.0,1.0,-1.0,1.0,-1.0,1.0);
//         glMatrixMode(GL_MODELVIEW);
//         glLoadIdentity();

        //drawLines(mag_data,1.0,-.3,.3,.6,.9);
        //drawLines(mag_raw,0.5,-.3,.3,.6,.9);
        drawLines(mag_raw,0.5,-1,1,-.5,.5);
        //drawLines(accel_data,1.0,-1.0,-.4,.6,0.9);
        //drawLines(accel_raw,0.5,-1.0,-.4,.6,0.9);
        //drawLines(gyro_data,1.0,0.4,1.0,.6,.9);
        //drawLines(gyro_raw,0.5,0.4,1.0,.6,.9);

        //drawLines(vlengths,1.0,-.3,.3,-.9,-.6);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    typedef std::shared_ptr<IMUScene> Ptr;
    IMUScene():q(imu.NewOutQueue()),mag_data(1500),mag_raw(1500),accel_data(1500),accel_raw(1500),gyro_data(1500),gyro_raw(1500),vlengths(1500),minmag(1024.0),maxmag(0.0),dindex(0)
    {
        imu.Open("00:06:66:00:D9:EE");
    }
};


int main(int argc, char **argv)
{
    IMUScene s;
    s("IMU");
}
