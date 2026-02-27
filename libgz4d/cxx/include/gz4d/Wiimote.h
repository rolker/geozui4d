#ifndef GZ4D_WIIMOTE_H
#define GZ4D_WIIMOTE_H

#include <gz4d/gz4d.h>
#include <string>
#include <queue>
#include <gz4d/socket/BTL2CAPSocket.h>
#include <boost/thread/thread.hpp>
#include <gz4d/Vector.h>
#include <gz4d/thread/Producer.h>
#include <gz4d/Event.h>
#include <gz4d/time.h>
#include <iostream>
#include <iomanip>


namespace gz4d
{

#define WIIMOTE_BUTTON_1 0x0002
#define WIIMOTE_BUTTON_2 0x0001
#define WIIMOTE_BUTTON_B 0x0004
#define WIIMOTE_BUTTON_A 0x0008
#define WIIMOTE_BUTTON_M 0x0010
#define WIIMOTE_BUTTON_H 0x0080
#define WIIMOTE_BUTTON_L 0x0100
#define WIIMOTE_BUTTON_R 0x0200
#define WIIMOTE_BUTTON_D 0x0400
#define WIIMOTE_BUTTON_U 0x0800
#define WIIMOTE_BUTTON_P 0x1000

#define WIIMOTE_BUTTON_MASK 0x1F9F

#define WIIMOTE_NUNCHUK_BUTTON_C 0x1
#define WIIMOTE_NUNCHUK_BUTTON_Z 0x2

#define WIIMOTE_NUNCHUK_BUTTON_MASK 0x03

#define WIIMOTE_LED_1 0x1
#define WIIMOTE_LED_2 0x2
#define WIIMOTE_LED_4 0x4
#define WIIMOTE_LED_8 0x8

    /**
    \ingroup devices
    */
    class Wiimote: public Producer<Event>
    {
    public:
        typedef std::shared_ptr<Wiimote> Ptr;

        enum Button
        {
            A,
            B,
            Up,
            Down,
            Right,
            Left,
            Plus,
            Minus,
            Home,
            One,
            Two,
            C,
            Z
        };

        Wiimote()
        :inQueue(new ThreadedQueue<Event>)
        {
            wiimote.haveCalibration = false;
            wiimote.buttons = 0;
            lastButtons = 0;
            nunchuk.present = false;
            nunchuk.haveCalibration = false;
            nunchuk.buttons = 0;
            lastNunchukButtons = 0;
            rumbleMask = 0;
        }
        
        ~Wiimote()
        {
            socketThread.interrupt();
            socketThread.join();
        }
        

        ThreadedQueue<Event>::Ptr &GetInQueue(){return inQueue;}
        void open(std::string const &address)
        {
            Wiimote::address = address;
            control_socket.Connect(gz4d::BTL2CAPSocket::Address(address,17));
            data_socket.Connect(gz4d::BTL2CAPSocket::Address(address,19));
            data_socket.SetNonBlocking();
            
            socketThread = std::move(boost::thread(boost::ref(*this)));
            inQueue->push(Event(Event::Null,1)); // ask for calibration
            inQueue->push(Event(Event::Null,2)); // ask to send motion
        }
        
        void close()
        {
            socketThread.interrupt();
            socketThread.join();
            
            control_socket.Close();
            data_socket.Close();
        }
        
        void reopen()
        {
            control_socket = gz4d::BTL2CAPSocket();
            control_socket.Connect(gz4d::BTL2CAPSocket::Address(address,17));
            data_socket = gz4d::BTL2CAPSocket();
            data_socket.Connect(gz4d::BTL2CAPSocket::Address(address,19));
            data_socket.SetNonBlocking();
            
            socketThread = std::move(boost::thread(boost::ref(*this)));
            inQueue->push(Event(Event::Null,1)); // ask for calibration
            inQueue->push(Event(Event::Null,2)); // ask to send motion
        }
        

        void operator()()
        {
            
            while (!boost::this_thread::interruption_requested())
            {
                std::string b;
                b.resize(1024);
                try
                {
                    data_socket.Recv(b);
                    if( !b.empty() )
                    {
                        unsigned char const * buffer = (unsigned char const *)b.data();
                        switch(buffer[0])
                        {
                            case 0xa1:
                                switch (buffer[1])
                                {
                                    case 0x37:
                                    {
                                        Joystick j;
                                        Accelerometer na;
                                        unsigned char nb;
                                        
                                        j[0] = (buffer[17]^0x17) + 0x17;
                                        j[1] = (buffer[18]^0x17) + 0x17;
                                        if(nunchuk.joystick != j)
                                        {
                                            nunchuk.joystick = j;
                                            if (nunchuk.haveCalibration)
                                            {
                                                Vector<float,2> js;
                                                if (nunchuk.joystick[0] < nunchuk.joystickCalibration.center[0])
                                                    js[0] = -1.0+(nunchuk.joystick[0]-nunchuk.joystickCalibration.min[0])/nunchuk.joystickCalibration.min_center_diff[0];
                                                else
                                                    js[0] = (nunchuk.joystick[0]-nunchuk.joystickCalibration.center[0])/nunchuk.joystickCalibration.center_max_diff[0];
                                                if (nunchuk.joystick[1] < nunchuk.joystickCalibration.center[1])
                                                    js[1] = -1.0+(nunchuk.joystick[1]-nunchuk.joystickCalibration.min[1])/nunchuk.joystickCalibration.min_center_diff[1];
                                                else
                                                    js[1] = (nunchuk.joystick[1]-nunchuk.joystickCalibration.center[1])/nunchuk.joystickCalibration.center_max_diff[1];
                                                Send(Event(Event::Axis,2,js[0],js[1]));
                                            }
                                        }
                                        
                                        na[0] = (buffer[19]^0x17) + 0x17;
                                        na[1] = (buffer[20]^0x17) + 0x17;
                                        na[2] = (buffer[21]^0x17) + 0x17;
                                        if(na != nunchuk.accelerometer)
                                        {
                                            nunchuk.accelerometer = na;
                                            if(nunchuk.haveCalibration)
                                            {
                                                Vector<float,3> ca = (nunchuk.accelerometer - nunchuk.accelerometerCalibration.zero)/ nunchuk.accelerometerCalibration.diff;
                                                Send(Event(Event::Axis,1,ca[0],ca[1],ca[2]));
                                            }
                                        }
                                        
                                        switch(buffer[22]& WIIMOTE_NUNCHUK_BUTTON_MASK)
                                        {
                                            case 0:
                                                nb = WIIMOTE_NUNCHUK_BUTTON_Z;
                                                break;
                                            case 1:
                                                nb = WIIMOTE_NUNCHUK_BUTTON_C;
                                                break;
                                            case 2:
                                                nb = WIIMOTE_NUNCHUK_BUTTON_C | WIIMOTE_NUNCHUK_BUTTON_Z;
                                                break;
                                            default:
                                                nb = 0;
                                        }
                                        nunchuk.buttons = nb & WIIMOTE_NUNCHUK_BUTTON_MASK;
                                    }
                                            case 0x31:
                                            {
                                                Accelerometer a;
                                                a[0] = buffer[4];
                                                a[1] = buffer[5];
                                                a[2] = buffer[6];
                                                if(a != wiimote.accelerometer)
                                                {
                                                    wiimote.accelerometer = a;
                                                    if (wiimote.haveCalibration)
                                                    {
                                                        Vector<float,3> ca = (wiimote.accelerometer - wiimote.calibration.zero) / wiimote.calibration.diff;
                                                        Send(Event(Event::Axis,0,ca[0],ca[1],ca[2]));
                                                    }
                                                }
                                            }
                                            case 0x30:
                                            {
                                                unsigned short b;
                                                b = buffer[2] << 8 | buffer[3];
                                                wiimote.buttons = b & WIIMOTE_BUTTON_MASK;
                                            }
                                            break;
                                            case 0x21:
                                            {
                                                unsigned char size = (buffer[4] >> 4)+1;
                                                unsigned char tmp[2];
                                                tmp[0] = buffer[6];
                                                tmp[1] = buffer[5];
                                                unsigned short offset = *((unsigned short *)tmp);
                                                if (size == 0x08 && offset  == 0x16)
                                                {
                                                    wiimote.calibration.zero[0] = buffer[7];
                                                    wiimote.calibration.zero[1] = buffer[8];
                                                    wiimote.calibration.zero[2] = buffer[9];
                                                    wiimote.calibration.one[0] = buffer[11];
                                                    wiimote.calibration.one[1] = buffer[12];
                                                    wiimote.calibration.one[2] = buffer[13];
                                                    wiimote.calibration.diff = wiimote.calibration.one - wiimote.calibration.zero;
                                                    wiimote.haveCalibration = true;
                                                }
                                                else if (size == 0x10 && offset  == 0x20)
                                                {
                                                    nunchuk.accelerometerCalibration.zero[0] = (buffer[7]^0x17) + 0x17;
                                                    nunchuk.accelerometerCalibration.zero[1] = (buffer[8]^0x17) + 0x17;
                                                    nunchuk.accelerometerCalibration.zero[2] = (buffer[9]^0x17) + 0x17;
                                                    nunchuk.accelerometerCalibration.one[0] = (buffer[11]^0x17) + 0x17;
                                                    nunchuk.accelerometerCalibration.one[1] = (buffer[12]^0x17) + 0x17;
                                                    nunchuk.accelerometerCalibration.one[2] = (buffer[13]^0x17) + 0x17;
                                                    nunchuk.accelerometerCalibration.diff = nunchuk.accelerometerCalibration.one - nunchuk.accelerometerCalibration.zero;
                                                    
                                                    nunchuk.joystickCalibration.max[0] = (buffer[15]^0x17) + 0x17;
                                                    nunchuk.joystickCalibration.min[0] = (buffer[16]^0x17) + 0x17;
                                                    nunchuk.joystickCalibration.center[0] = (buffer[17]^0x17) + 0x17;
                                                    nunchuk.joystickCalibration.max[1] = (buffer[18]^0x17) + 0x17;
                                                    nunchuk.joystickCalibration.min[1] = (buffer[19]^0x17) + 0x17;
                                                    nunchuk.joystickCalibration.center[1] = (buffer[20]^0x17) + 0x17;
                                                    nunchuk.joystickCalibration.min_center_diff = nunchuk.joystickCalibration.center - nunchuk.joystickCalibration.min;
                                                    nunchuk.joystickCalibration.center_max_diff = nunchuk.joystickCalibration.max - nunchuk.joystickCalibration.center;
                                                    nunchuk.haveCalibration = true;
                                                }
                                                else
                                                {
                                                    std::cerr << std::endl;
                                                }
                                            }
                                            break;
                                            case 0x20:
                                            {
                                                wiimote.battery = buffer[7];
                                                setNunchukAttached(buffer[4]&0x2);
                                            }
                                            break;
                                            case 0x22:
                                            {
                                                //std::cerr << "Acknowledge output report: " << int(buffer[5]) << std::endl;
                                            }
                                            break;
                                            default:
                                            {
                                                std::cerr << "Unknown 0xa1: " << (short)buffer[1] << std::endl;
                                                for(int i = 0; i < b.size(); ++i)
                                                    std::cerr << (short)(buffer[i]) << ", ";
                                                std::cerr << std::endl;
                                            }
                                }
                                break;
                                            default:
                                                std::cerr << "Unknown: " << (short)(buffer[0]) << " , " << (short)(buffer[1]) << std::endl;
                                                break;
                        }
                    }
                }
                catch (Socket::WouldBlockException)
                {
                    if(!inQueue->empty())
                    {
                        Event e = inQueue->pop();
                        switch (e.type)
                        {
                            case Event::LED:
                                setLights(e.value);
                                break;
                            case Event::Rumble:
                                rumble(time::Duration(e.value));
                            case Event::Null:
                                switch(e.id)
                                {
                                    case 0:
                                        getStatus();
                                        break;
                                    case 1:
                                        getCalibration();
                                        break;
                                    case 2:
                                        setSendMotion();
                                        break;
                                    case 3:
                                        getNunchukCalibration();
                                        break;
                                        
                                }
                                    default:
                                        break;
                        }
                    }
                    else
                    {
                        boost::this_thread::yield();
                    }
                }
                checkForButtonEvents();
                
                
            }
        }
        
    private:
        void send(std::string data)
        {
            
            if(data[0] == 0x52)
            {
                if(data[1] == 0x13)
                {
                    rumbleMask = data[2] & 0x1;
                }
                else if(data[1] == 0x11 || data[1] == 0x17)
                {
                    data[2] |= rumbleMask;
                }
            }
            control_socket.Send(data);
        }
        
        void checkForButtonEvents()
        {
            if (lastButtons != wiimote.buttons)
            {
                unsigned short changed(lastButtons ^ wiimote.buttons);
                if (changed & WIIMOTE_BUTTON_1) SendKeyEvent(One,wiimote.buttons &WIIMOTE_BUTTON_1);
                if (changed & WIIMOTE_BUTTON_2) SendKeyEvent(Two,wiimote.buttons &WIIMOTE_BUTTON_2);
                if (changed & WIIMOTE_BUTTON_B) SendKeyEvent(B,wiimote.buttons &WIIMOTE_BUTTON_B);
                if (changed & WIIMOTE_BUTTON_A) SendKeyEvent(A,wiimote.buttons &WIIMOTE_BUTTON_A);
                if (changed & WIIMOTE_BUTTON_M) SendKeyEvent(Minus,wiimote.buttons &WIIMOTE_BUTTON_M);
                if (changed & WIIMOTE_BUTTON_H) SendKeyEvent(Home,wiimote.buttons &WIIMOTE_BUTTON_H);
                if (changed & WIIMOTE_BUTTON_L) SendKeyEvent(Left,wiimote.buttons &WIIMOTE_BUTTON_L);
                if (changed & WIIMOTE_BUTTON_R) SendKeyEvent(Right,wiimote.buttons &WIIMOTE_BUTTON_R);
                if (changed & WIIMOTE_BUTTON_D) SendKeyEvent(Down,wiimote.buttons &WIIMOTE_BUTTON_D);
                if (changed & WIIMOTE_BUTTON_U) SendKeyEvent(Up,wiimote.buttons &WIIMOTE_BUTTON_U);
                if (changed & WIIMOTE_BUTTON_P) SendKeyEvent(Plus,wiimote.buttons &WIIMOTE_BUTTON_P);
            }
            lastButtons = wiimote.buttons;
            if (nunchuk.present && lastNunchukButtons != nunchuk.buttons)
            {
                unsigned short changed(lastNunchukButtons ^ nunchuk.buttons);
                if (changed & WIIMOTE_NUNCHUK_BUTTON_C) SendKeyEvent(C,nunchuk.buttons &WIIMOTE_NUNCHUK_BUTTON_C);
                if (changed & WIIMOTE_NUNCHUK_BUTTON_Z) SendKeyEvent(Z,nunchuk.buttons &WIIMOTE_NUNCHUK_BUTTON_Z);
            }
            lastNunchukButtons = nunchuk.buttons;
        }
        
        void SendKeyEvent(Button id, bool value)
        {
            Send(Event(Event::Key,id,value));
        }
        
        
        bool setSendMotion()
        {
            std::string data(4,0);
            data[0] = 0x52;
            data[1] = 0x12;
            data[2] = 0x00;
            data[3] = 0x37;
            send(data);
            return true;
        }
        
        
        bool setLights(unsigned char lights)
        {
            std::string data(3,0);
            data[0] = 0x52;
            data[1] = 0x11;
            data[2] = lights<<4;
            send(data);
            return true;
        }
        
        bool rumble(time::Duration duration)
        {
            std::string data(3,0);
            data[0] = 0x52;
            data[1] = 0x13;
            data[2] = 0x01;
            send(data);
            Sleep(duration);
            data[2] = 0x00;
            send(data);
            return true;
        }
        

        typedef Vector<float, 3> Accelerometer;
        typedef Vector<float, 2> Joystick;

        struct AccelerometerCalibration
        {
            Accelerometer zero; ///< value of accelerometer at zero g.
            Accelerometer one;  ///< value of accelerometer at one g.
            Vector<float,3> diff;   ///< difference in value between zero and one g.
        };

        struct JoystickCalibration
        {
            Joystick min;  ///< minimum joystick value
            Joystick center;  ///< center joystick value
            Joystick max;  ///< max joystick value

            Joystick min_center_diff;
            Joystick center_max_diff;
        };

        struct WiimoteState
        {
            unsigned short buttons; ///< state of the buttons
            Accelerometer accelerometer;
            bool haveCalibration; ///< True if we have calibration data
            AccelerometerCalibration calibration;
            unsigned char battery; ///< Battery charge level
        };

        struct NunchukState
        {
            bool present;      ///< Nunchuk present flag
            unsigned char buttons; ///< state of the buttons
            Accelerometer accelerometer;
            Joystick joystick;
            bool haveCalibration; ///< True if we have calibration data
            AccelerometerCalibration accelerometerCalibration;
            JoystickCalibration joystickCalibration;
        };

        bool getStatus()
        {
            std::string data(3,0);
            data[0] = 0x52;
            data[1] = 0x15;
            data[2] = 0x00;
            send(data);
            return true;
        }
        

        bool getCalibration()
        {
            std::string data(8,0);
            data[0] = 0x52;
            data[1] = 0x17;
            data[2] = 0x00;
            data[3] = 0x00;
            data[4] = 0x00;
            data[5] = 0x16;
            data[6] = 0x00;
            data[7] = 0x08;
            send(data);
            return true;
        }
        
        bool getNunchukCalibration()
        {
            std::string data(8,0);
            data[0] = 0x52;
            data[1] = 0x17;
            data[2] = 0x04;
            data[3] = 0xA4;
            data[4] = 0x00;
            data[5] = 0x20;
            data[6] = 0x00;
            data[7] = 0x10; // asking for 16 bytes
            send(data);
            return true;
        }
        

        bool enableNunchuk()
        {
            std::string data(23,0x00);
            data[0] = 0x52;
            data[1] = 0x16;
            data[2] = 0x04;
            data[3] = 0xa4;
            data[4] = 0x00;
            data[5] = 0x40;
            data[6] = 0x01;
            send(data);
            inQueue->push(Event(Event::Null,3)); // ask for calibration
            return true;
        }
        
        bool setNunchukAttached(bool attached)
        {
            if (nunchuk.present == attached)
                return true;
            nunchuk.present = attached;
            nunchuk.haveCalibration = false;
            if (attached)
            {
                enableNunchuk();
                Sleep(seconds(0.001));
            }
            inQueue->push(Event(Event::Null,2)); // set output format
            return true;
        }
        

        BTL2CAPSocket control_socket;
        BTL2CAPSocket data_socket;

        unsigned char rumbleMask;

        unsigned short lastButtons;
        unsigned char lastNunchukButtons;

        WiimoteState wiimote;
        NunchukState nunchuk;

        boost::thread socketThread;

        ThreadedQueue<Event>::Ptr inQueue;
        std::string address;

    };
    

}

#endif
