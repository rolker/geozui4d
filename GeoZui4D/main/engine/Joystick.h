// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include<SDL/SDL.h>
#include"engine/InputMediator.h"

class Joystick : public InputMediator
{ 
public:
  enum JoystickEventType
  {
	JoystickButtonBase	= 0x00010000,
    JoystickButton1		= 0x80010000,
    JoystickButton2		= 0x80020000,
    JoystickButton3		= 0x80040000,
    JoystickButton4		= 0x80080000,
	JoystickButtonMask	= 0x080F0000,
    JoystickButtonDown	= 0x80100000,
    JoystickButtonUp	= 0x80200000,
    JoystickButtonAction= 0x00300000,
	JoystickAxisBase	= 0x00400000,
    JoystickAxis1		= 0x80400000,
    JoystickAxis2		= 0x80800000,
    JoystickAxis3		= 0x81000000,
    JoystickAxis4		= 0x82000000,
	JoystickAxisAction	= 0x0BC00000, //includes JoystickOther
    JoystickHat			= 0x84000000,
    JoystickOther		= 0x88000000,
    JoystickAction		= 0x0FFF0000
  };

  Joystick(std::string name);
  ~Joystick();

  virtual std::string nameOfEventType(unsigned int inputType);
  virtual unsigned int valueOfSpecificEventType(std::string eventType);

  int HandleInput(unsigned int inputType, InputInfo *info);
  void Idle();

  /*---Scripting funcitons---*/
  virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
  virtual int list(Tcl_Interp *interp);

  struct JoystickInfoType
  {
    int numAxes, numButtons, numHats;
    int * Axes;
    int * Buttons;
    int * Hats;
  };

protected:

  SDL_Joystick * js;

  JoystickInfoType joystickInfo;
  GLboolean continuousAxis;

  int * newAxes;
  int * newButtons;
  int * newHats;
};


#endif
