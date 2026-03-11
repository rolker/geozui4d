// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_XINPUT_H
#define GZ4D_XINPUT_H

//gz4d_cmake depends XInput

#include <gz4d/gz4d.h>

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <string>
#include <vector>
#include <map>

namespace gz4d
{
    /** Interface to XOrg XInput devices
    \ingroup devices
    */
    class XInput
    {
        public:
            XInput():display(NULL),window(None),screen_number(-1),device_info(NULL),device_count(0){}
            
            ~XInput()
            {
                XFreeDeviceList(device_info);
                XCloseDisplay(display);
            }
            
            bool Connect(Display * d = NULL, int s = -1, Window w = None)
            {
                if (!d)
                    display = XOpenDisplay(NULL);
                else
                    display = d;
                
                if (!display)
                    return false;
                
                if (s == -1)
                    screen_number = DefaultScreen(display);
                else
                    screen_number = s;
                std::cout << "Screen No:" << screen_number << std::endl;
                
                if (w == None)
                    window = DefaultRootWindow(display);
                else
                    window = w;
                
                if (window = None)
                    return false;
                
                BuildDeviceList();
                
                return true;
            }
            
            void BuildDeviceList()
            {
                device_info = XListInputDevices(display,&device_count);
                
                for (int i = 0; i < device_count; i++)
                {
                    Device d;
                    d.id = device_info[i].id;
                    device_by_ids[d.id] = i;
                    d.name = device_info[i].name;
                    device_by_names[d.name] = i;
                    d.isExtension = (device_info[i].use == IsXExtensionDevice);
                    d.keyCount = 0;
                    d.buttonCount = 0;
                    d.axisCount = 0;
                    d.xdevice = NULL;
                    d.keyPressEvent = 0;
                    d.keyReleaseEvent = 0;
                    d.buttonPressEvent = 0;
                    d.buttonPressGrabEvent = 0;
                    d.buttonReleaseEvent = 0;
                    d.motionNotifyEvent = 0;
                    d.stateNotifyEvent = 0;
                    
                    XAnyClassPtr cp = device_info[i].inputclassinfo;
                    for (int c = 0; c < device_info[i].num_classes; c++)
                    {
                        XKeyInfo *keyInfo;
                        XButtonInfo *buttonInfo;
                        XValuatorInfo * valuatorInfo;
                        switch(cp->c_class)
                        {
                            case KeyClass:
                                keyInfo = (XKeyInfo*)cp;
                                d.keyCount = keyInfo->num_keys;
                                break;
                            case ButtonClass:
                                buttonInfo = (XButtonInfo*)cp;
                                d.buttonCount = buttonInfo->num_buttons;
                                break;
                            case ValuatorClass:
                                valuatorInfo = (XValuatorInfo*)cp;
                                d.axisCount = valuatorInfo->num_axes;
                                break;
                            default:
                                break;
                        }
                        cp = (XAnyClassPtr)&(((char *)cp)[cp->length]);
                    }
                    
                    devices.push_back(d);
                }
            }
            
            void Print()
            {
                for(std::vector<Device>::iterator i = devices.begin(); i != devices.end(); i++)
                {
                    std::cout << "Device ID: " << i->id << std::endl;
                    std::cout << "  Name: " << i->name  << std::endl;
                    if (i->isExtension)
                        std::cout << "  Extension" << std::endl;
                    if (i->keyCount != 0)
                        std::cout << "    " << i->keyCount << " keys" << std::endl;
                    if (i->buttonCount != 0)
                        std::cout << "    " << i->buttonCount << " buttons" << std::endl;
                    if (i->axisCount != 0)
                        std::cout << "    " << i->axisCount << " axes" << std::endl;
                }
            }
            
            bool OpenDevice(int id)
            {
                int i(device_by_ids[id]);
                devices[i].xdevice = XOpenDevice(display,i);
                devices[i].eventClassCount = 0;
                
                if (devices[i].keyCount > 0)
                {
                    DeviceKeyPress(devices[i].xdevice,devices[i].keyPressEvent,devices[i].eventClasses[devices[i].eventClassCount]);
                    devices[i].eventClassCount++;
                    DeviceKeyRelease(devices[i].xdevice,devices[i].keyReleaseEvent,devices[i].eventClasses[devices[i].eventClassCount]);
                    devices[i].eventClassCount++;
                }
                
                if (devices[i].buttonCount > 0)
                {
                    DeviceButtonPress(devices[i].xdevice,devices[i].buttonPressEvent,devices[i].eventClasses[devices[i].eventClassCount]);
                    devices[i].eventClassCount++;
                    DeviceButtonPressGrab(devices[i].xdevice,devices[i].buttonPressGrabEvent,devices[i].eventClasses[devices[i].eventClassCount]);
                    devices[i].eventClassCount++;
                    DeviceButtonRelease(devices[i].xdevice,devices[i].buttonReleaseEvent,devices[i].eventClasses[devices[i].eventClassCount]);
                    devices[i].eventClassCount++;
                }
                
                if (devices[i].axisCount > 0)
                {
                    DeviceMotionNotify(devices[i].xdevice,devices[i].motionNotifyEvent,devices[i].eventClasses[devices[i].eventClassCount]);
                    devices[i].eventClassCount++;
                }
                
                DeviceStateNotify(devices[i].xdevice,devices[i].stateNotifyEvent,devices[i].eventClasses[devices[i].eventClassCount]);
                devices[i].eventClassCount++;
                XSelectExtensionEvent(display,window,devices[i].eventClasses,devices[i].eventClassCount);
                
                return true;
            }
            
        private:
            struct Device
            {
                XID id;
                std::string name;
                bool isExtension;
                int keyCount;
                int buttonCount;
                int axisCount;
                XDevice * xdevice;
                XEventClass eventClasses[7];
                int eventClassCount;
                int keyPressEvent;
                int keyReleaseEvent;
                int buttonPressEvent;
                int buttonPressGrabEvent;
                int buttonReleaseEvent;
                int motionNotifyEvent;
                int stateNotifyEvent;
            };

            Display * display;
            Window window;
            int screen_number;
            int device_count;
            XDeviceInfo * device_info;
            std::vector<Device> devices;
            std::map<int,int> device_by_ids;
            std::map<std::string,int> device_by_names;
    };

};
#endif
