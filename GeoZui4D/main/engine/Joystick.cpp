#include"engine/Joystick.h"

#include <stdlib.h>
#include <iostream>
#include <string.h>

using namespace std;

Joystick::Joystick(string name)
:InputMediator(name)
{
    int i;

  // Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK ) < 0 )
    {
      std::cerr << "Problem initializing SDL" << endl;
    }

  int jscount = SDL_NumJoysticks();
  //continuousAxis = GL_FALSE;
  continuousAxis = GL_TRUE;

  if(jscount > 0)
  {
    std::cout << "Opening joystick 0" << std::endl;
    js=SDL_JoystickOpen(0);
    if(js)
    {
      joystickInfo.numAxes = SDL_JoystickNumAxes(js);
      joystickInfo.numButtons = SDL_JoystickNumButtons(js);
      joystickInfo.numHats = SDL_JoystickNumHats(js);
      std::cout << "Number of Axes: " << joystickInfo.numAxes  << endl;
      std::cout << "Number of Buttons: " << joystickInfo.numButtons << endl;
      std::cout << "Number of Hats: " << joystickInfo.numHats << endl;

      joystickInfo.Axes = new int[joystickInfo.numAxes];
      joystickInfo.Buttons = new int[joystickInfo.numButtons];
      joystickInfo.Hats = new int[joystickInfo.numHats];
      newAxes = new int[joystickInfo.numAxes];
      newButtons = new int[joystickInfo.numButtons];
      newHats = new int[joystickInfo.numHats];

      for(i=0; i<joystickInfo.numAxes; i++)
      {
        joystickInfo.Axes[i]=SDL_JoystickGetAxis(js,i);
      }

      for(i=0; i<joystickInfo.numButtons; i++)
      {
        joystickInfo.Buttons[i]=SDL_JoystickGetButton(js,i);
      }

      for(i=0; i<joystickInfo.numHats; i++)
      {
        joystickInfo.Hats[i]=SDL_JoystickGetHat(js,i);
      }

    }
    else
    {
      std::cerr << "failed" << endl;
    }
  }
  else
  {
    js = NULL;
  }
}

Joystick::~Joystick()
{

  SDL_JoystickClose(js);

  // hopefully, we are donw with SDL by now!
  SDL_Quit();
}

int Joystick::HandleInput(unsigned int inputType, InputInfo *info)
{
  if (inputType == NoInput) //Idle event
  {
    Idle();
    return PASS_EVENT_THROUGH;
  }

  if (info->eventStream == this)
	return InputMediator::HandleInput(inputType, info);
  else
	return PASS_EVENT_THROUGH;
}

void Joystick::Idle()
{
  int i;
  int type;
  InputInfo *info;


  if(js)
  {
	  SDL_JoystickUpdate();
    for(i=0; i<joystickInfo.numAxes; i++)
    {
      newAxes[i]=SDL_JoystickGetAxis(js,i);
      if ((newAxes[i] != joystickInfo.Axes[i]) || (continuousAxis && (newAxes[i] != 0)))
      {
        joystickInfo.Axes[i] = newAxes[i];
        switch(i)
        {
        case 0:
          type = JoystickAxis1;
          break;
        case 1:
          type = JoystickAxis2;
          break;
        case 2:
          type = JoystickAxis3;
          break;
        case 3:
          type = JoystickAxis4;
          break;
        default:
          type = JoystickOther;
        };

        info = new InputInfo((ToglWindow *)NULL, type);
        info->val.p = &joystickInfo;
		info->eventStream = this;
        InputMediator::HandleInput(type, info);
        delete info;
      }
    }

    for(i=0; i<joystickInfo.numButtons; i++)
    {
      newButtons[i]=SDL_JoystickGetButton(js,i);
      if(newButtons[i]!=joystickInfo.Buttons[i])
      {
        joystickInfo.Buttons[i] = newButtons[i];
        switch(i)
        {
        case 0:
          type = JoystickButton1;
          break;
        case 1:
          type = JoystickButton2;
          break;
        case 2:
          type = JoystickButton3;
          break;
        case 3:
          type = JoystickButton4;
          break;
        default:
          type = JoystickOther;
        };
        if(joystickInfo.Buttons[i]==1)
        {
          type |= JoystickButtonDown;
        }
        else
        {
          type |= JoystickButtonUp;
        }
        info = new InputInfo((ToglWindow *)NULL, type);
        info->val.p = &joystickInfo;
		info->eventStream = this;
        InputMediator::HandleInput(type, info);
        delete info;
      }
    }

    for(i=0; i<joystickInfo.numHats; i++)
    {
      newHats[i]=SDL_JoystickGetHat(js,i);
      if(newHats[i]!=joystickInfo.Hats[i])
      {
        joystickInfo.Hats[i] = newHats[i];
        type = JoystickHat;
        info = new InputInfo((ToglWindow *)NULL, type);
        info->val.p = &joystickInfo;
		info->eventStream = this;
        InputMediator::HandleInput(type, info);
        delete info;
      }
    }
  }
}

/* ========================================================================= */
/**
 *  This function returns the string name of the passed inputType.  Child
 *	classes should redefine this function if they define their own special
 *	class of events.
 */
string Joystick::nameOfEventType(unsigned int inputType)
{
	unsigned int val;

	if ((inputType & StreamSpecific) == 0)
		return InputMediator::nameOfEventType(inputType);

	if ((inputType & JoystickAction) == 0)
		return "unknown";

	if (inputType & JoystickButtonAction)
	{
		val = inputType & JoystickButtonMask;
		if ((inputType & JoystickButtonDown) == JoystickButtonDown)
		{
			if (val & JoystickButton1)
				return "joystick1Down";
			if (val & JoystickButton2)
				return "joystick2Down";
			if (val & JoystickButton3)
				return "joystick3Down";
			if (val & JoystickButton4)
				return "joystick4Down";
			if (val & JoystickOther)
				return "joystickOtherDown";
		}
		if ((inputType & JoystickButtonUp) == JoystickButtonUp)
		{
			if (val & JoystickButton1)
				return "joystick1Up";
			if (val & JoystickButton2)
				return "joystick2Up";
			if (val & JoystickButton3)
				return "joystick3Up";
			if (val & JoystickButton4)
				return "joystick4Up";
			if (val & JoystickOther)
				return "joystickOtherUp";
		}
	}
	if (inputType & JoystickAxisAction)
	{
		val = inputType & JoystickAxisAction;
		if (val & JoystickAxis1)
			return "joystick1Axis";
		if (val & JoystickAxis2)
			return "joystick2Axis";
		if (val & JoystickAxis3)
			return "joystick3Axis";
		if (val & JoystickAxis4)
			return "joystick4Axis";
		if (val & JoystickOther)
			return "joystickOtherAxis";
	}
	if (inputType == JoystickHat)
		return "joystickHat";
	return "unknown";
}

/* ========================================================================= */
/**
 *  This function returns the type value of the passed event name.  Child
 *	classes should redefine this function if they define their own special
 *	class of events.
 */
unsigned int Joystick::valueOfSpecificEventType(string eventType)
{
	char c, digit = 0;
	unsigned int retval = InvalidInput;

	if (eventType.substr(0,8) == "joystick")
	{
		c = eventType[8];
		if (c == 'H' && eventType == "joystickHat")
			return JoystickHat;
		else if ((c >= '1' && c <= '4') || c == 'O')
		{
			if (c == 'O')
				retval = JoystickOther;
			else
				digit = c - '0';

			if (eventType.substr(9) == "Axis")
				return retval | (digit? (JoystickAxisBase << digit) : 0);
			if (eventType.substr(6) == "Down")
				return retval | JoystickButtonDown | (digit? (JoystickButtonBase << digit) : 0);
			if (eventType.substr(6) == "Up")
				return retval | JoystickButtonUp | (digit? (JoystickButtonBase << digit) : 0);
		}
	}
	else
		return InputMediator::valueOfSpecificEventType(eventType);

	return InvalidInput;
}

/*--------------------------Scripting Support--------------------------------*/
/*---------------------------------------------------------------------------*/
int Joystick::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	char retval[64];

	if (argc >= 2)
	{
		if (!strcmp(argv[0], "continuouslyGenerate"))
		{
			if (!strcmp(argv[1], "axis"))
			{
				if (argc >= 3)
					continuousAxis = atoi(argv[2]);
				sprintf(retval, "%d", continuousAxis);
				Tcl_SetResult(interp, retval, TCL_VOLATILE);
				return TCL_OK;
			}

			Tcl_AppendResult(interp, "Usage: continuouslyGenerate axis [value]", NULL);
			return TCL_ERROR;
		}
	}
	return InputMediator::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Joystick::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "continuouslyGenerate ", NULL);
	return InputMediator::list(interp);
}
