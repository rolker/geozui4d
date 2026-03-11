// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// EventHandler.h
///////////////////////////////////////////////////////////////////////////////
/**
 *  This class implements a handler for dispatching events through scripts
 *  and procedures, useful for scripts and non-Interactors.
 *  This class implements the following responsibilities:
 *  *   Execute a script or procedure when a selected event is encountered.
 *
 *  Created by mdp 8/21/2002
 */
///////////////////////////////////////////////////////////////////////////////

#include "EventHandler.h"
#include "engine/InputMediator.h"
#include "engine/ToglWindow.h"
#include "utilities/DottedAddress.h"
//#include "engine/Joystick.h"
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/** Constructor.  Initializes all variables to a neutral position.
 */
EventHandler::EventHandler(std::string name)
: Interactor(name)
{
    handler.script = NULL;
    nextIntAsChar = false;
}

/*---------------------------------------------------------------------------*/
/** Destructor.
 */
EventHandler::~EventHandler( void )
{
    if (handler.script)
        delete handler.script;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** HandleInput dispatches events as configured.
 */
int EventHandler::HandleInput(unsigned int inputType, InputInfo *info)
{
    std::string retval;
    char * script = NULL;
    int status, returnValue;

    if (type != Empty)
    {
        switch (type)
        {
            case VoidProcedure:
                if (inputToHandle == inputType)
                    return handler.voidProc();
                break;
            case StandardProcedure:
                return handler.stdProc(inputType, info);
                break;
            case SpecificScript:
                if ((inputType & AnyAction) != (inputToHandle & AnyAction))
                    return PASS_EVENT_THROUGH;
            case GeneralScript:
                if (hasPercent)
                {
                    fillCallback(script, handler.script, info);
                    status = ToglWindow::Tcl_Eval(script, retval);
                }
                else
                    status = ToglWindow::Tcl_Eval(handler.script, retval);
                if (status == TCL_OK)
                {
                    returnValue = atoi(retval.c_str());
                    delete script;
                    return returnValue;
                }
                return SINK_EVENT; //sink on error
            case Sink:
                if ((inputType & AnyAction) == (inputToHandle & AnyAction))
                    return SINK_EVENT;
                break; //pass it on
        }
    }
    return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
void EventHandler::setCallback(int (*voidProc)(), unsigned int inputType)
{
    type = VoidProcedure;
    handler.voidProc = voidProc;
    inputToHandle = inputType;
}

/*---------------------------------------------------------------------------*/
void EventHandler::setCallback(int (*stdProc)(unsigned int, InputInfo *))
{
    type = StandardProcedure;
    handler.stdProc = stdProc;
    inputToHandle = 0;
}

/*---------------------------------------------------------------------------*/
void EventHandler::setScript(char *script, unsigned int inputType)
{
    type = SpecificScript;
    handler.script = strdup(script);
    inputToHandle = inputType;
    scanForPercent();
}

/*---------------------------------------------------------------------------*/
void EventHandler::setSink(unsigned int inputType)
{
    type = Sink;
    inputToHandle = inputType;
}

/*---------------------------------------------------------------------------*/
void EventHandler::setScript(char *script)
{
    type = GeneralScript;
    handler.script = strdup(script);
    inputToHandle = 0;
    scanForPercent();
}

/*---------------------------------------------------------------------------*/
void EventHandler::fillCallback(char *&resultScript, char *baseScript,
                                InputInfo *info)
{
    std::string sval;
    char *scratch;
    int baseLen, resultAlloc, type, i;//, jval;
    long int val;
    char c;//, jc;
//    Joystick::JoystickInfoType *jInfo;
    InputInfo *tempInfo = NULL;

    baseLen = strlen(baseScript);
    resultAlloc = (baseLen + 10) * 2;
    resultScript = new char[resultAlloc + 1];
    for (i = 0; *baseScript;)
    {
        if (i + 10 >= resultAlloc)
        {
            resultAlloc *= 2;
            scratch = new char[resultAlloc + 1];
            strcpy(scratch, resultScript);
            delete resultScript;
            resultScript = scratch;
        }
        c = *baseScript++;
        if (c != '%')
            resultScript[i++] = c;
        else
        {
            c = *baseScript++;
            //first see if there is a modifier
            type = NoInput;
            switch (c)
            {
                case 'c':
                    nextIntAsChar = true;
                    //fall through
                case 'i':
                    type = DataAsInt;
                    break;
                case 'l':
                    type = DataAsLong;
                    break;
                case 'd':
                    type = DataAsDouble;
                    break;
                case 'a':
                    type = DataAsAddress;
                    break;
                case 's':
                    type = DataAsString;
                    break;
                case 'p':
                    type = DataAsPointer;
                    break;
            }
            if (type == NoInput)
            {
                type = info->type & DataTypeMask;
                if (type == NoInput)
                    type = DataAsInt;
            }
            else
            {
                c = *baseScript++;
            }

            //then check to see if they want to interpret the value using the flags
            if (c == 'I')
            {
                tempInfo = info;
                info = (InputInfo *)info->val.p;
                c = *baseScript++;
            }
            //now process the flag proper
            switch (c)
            {
                case 'T': //Major type
                    if (info->eventStream)
                        sval = info->eventStream->nameOfEventType(info->type);
                    else
                        sval = globalInputMediator->nameOfEventType(info->type);
                    append(resultScript, sval.c_str(), i, resultAlloc);
                    break;
                case 't': //Minor type
                    type = info->type & DataTypeMask;
                    switch (type)
                    {
                        case DataAsInt:
                            resultScript[i++] = 'i';
                            break;
                        case DataAsLong:
                            resultScript[i++] = 'l';
                            break;
                        case DataAsDouble:
                            resultScript[i++] = 'd';
                            break;
                        case DataAsAddress:
                            resultScript[i++] = 'a';
                            break;
                        case DataAsString:
                            resultScript[i++] = 's';
                            break;
                        case DataAsPointer:
                            resultScript[i++] = 'p';
                            break;
                        default:
                            resultScript[i++] = '{';
                            resultScript[i++] = '}';
                            break;
                    }
                    break;
                case 'S': //serialNumber
                    val = info->serialNumber;
                    append(resultScript, val, i);
                    break;
                case 'v': //virtualEvent
                    resultScript[i++] = (info->virtualEvent)? '1' : '0';
                    break;
                case 'W': //window
                    if (info->window)
                        sval = info->window->getAddress()->getAddress();
                    else
                        sval = "{}";
                    append(resultScript, sval.c_str(), i, resultAlloc);
                    break;
                case 'E': //eventStream
                    if (info->eventStream)
                        sval = info->eventStream->getAddress()->getAddress();
                    else
                        sval = "{}";
                    append(resultScript, sval.c_str(), i, resultAlloc);
                    break;
                case 'N': //numAdditionalInfo
                    val = info->numAdditionalInfo;
                    append(resultScript, val, i);
                    break;
                case 'V': //val
                    append(resultScript, info->val, i, resultAlloc, type);
                    break;
/*                case 'J': //val as joystick information
                    jInfo = (Joystick::JoystickInfoType*) info->val.p;
                    c = *baseScript++;
                    jc = *baseScript; //peek ahead
                    if (jc >= '1' && jc <= '9')
                    {
                        jval = jc - '1';
                        baseScript++;
                    }
                    else
                        jval = -1;
                    switch (c)
                    {
                        case 'A': //axes
                            if (jval >= 0)
                            {
                                if (jval >= jInfo->numAxes)
                                    append(resultScript, "{}", i, resultAlloc);
                                else
                                    append(resultScript, jInfo->Axes[jval], i);
                            }
                            else
                                append(resultScript, jInfo->numAxes, i);
                            break;
                        case 'B': //buttons
                            if (jval >= 0)
                            {
                                if (jval >= jInfo->numButtons)
                                    append(resultScript, "{}", i, resultAlloc);
                                else
                                    append(resultScript, jInfo->Buttons[jval], i);
                            }
                            else
                                append(resultScript, jInfo->numButtons, i);
                            break;
                        case 'H': //hats
                            if (jval >= 0)
                            {
                                if (jval >= jInfo->numHats)
                                    append(resultScript, "{}", i, resultAlloc);
                                else
                                    append(resultScript, jInfo->Hats[jval], i);
                            }
                            else
                                append(resultScript, jInfo->numHats, i);
                            break;
                        default: //leave as literal
                            resultScript[i++] = '%';
                            resultScript[i++] = 'J';
                            resultScript[i++] = c;
                            if (jval >= 0)
                                resultScript[i++] = jval + '1';
                            break;
                    }
                    break;*/
                case 'M': //modifier
                    append(resultScript, info->modifier, i, resultAlloc, type);
                    break;
                case 'X': //x
                    append(resultScript, info->x, i, resultAlloc, type);
                    break;
                case 'Y': //y
                    append(resultScript, info->y, i, resultAlloc, type);
                    break;
                case 'Z': //z
                    append(resultScript, info->z, i, resultAlloc, type);
                    break;
                case '0': case '1': case '2': case '3': //additionalInfo[0..9]
                case '4': case '5': case '6': case '7':
                case '8': case '9':
                    append(resultScript, info->additionalInfo[c - '0'], i, resultAlloc, type);
                    break;
                case '%': //print a %
                    resultScript[i++] = '%';
                    break;
                default: //leave as literal
                    resultScript[i++] = '%';
                    resultScript[i++] = c;
                    break;
            }
            nextIntAsChar = false;

            if (tempInfo) //put us back after interpreting an 'I'
            {
                info = tempInfo;
                tempInfo = NULL;
            }
        }
    }
    resultScript[i] = '\0';
}

/*---------------------------------------------------------------------------*/
void EventHandler::append(char *&resultScript, std::string sval, int &i,
                          int &resultAlloc)
{
    int sLen;
    char *scratch;

    sLen = sval.size();
    if ((i + sLen) >= resultAlloc)
    {
        resultAlloc += sLen * 2;
        scratch = new char[resultAlloc + 1];
        strcpy(scratch, resultScript);
        delete resultScript;
        resultScript = scratch;
    }
    sprintf(&resultScript[i], "%s", sval.c_str());
    i += sLen;
}

/*---------------------------------------------------------------------------*/
void EventHandler::append(char *&resultScript, long int val, int &i)
{
    int digits, negFactor;
    long pow10 = 10, lastpow10 = 1;
    
    //count the digits
    if (val < 0)
    {
        negFactor = -1;
        val = -val;
    }
    else
        negFactor = 1;

    for (digits = 1; val >= pow10 || pow10 < lastpow10; digits++)
    {
        lastpow10 = pow10; //detects overflow
        pow10 *= 10;
    }

    sprintf(&resultScript[i], "%ld", val * negFactor);
    i += digits;
    if (negFactor < 0)
        i++;
}

/*---------------------------------------------------------------------------*/
void EventHandler::append(char *&resultScript, InputInfo::polyInfo &info,
                          int &i, int &resultAlloc, int type)
{
    long int val;
    char value[20];
    Addressable *address;

    switch (type)
    {
        case DataAsInt:
            val = info.i;
            if (nextIntAsChar)
                resultScript[i++] = (char)val;
            else
                append(resultScript, val, i);
            break;
        case DataAsLong:
            val = info.l;
            append(resultScript, val, i);
            break;
        case DataAsDouble:
            sprintf(value, "%g", info.d);
            append(resultScript, value, i, resultAlloc);
            break;
        case DataAsAddress:
            DYNAMIC_CAST(address, Addressable *, info.a);
            if (address && address->getAddress())
                append(resultScript, address->getAddress()->getAddress(), i, resultAlloc);
            else
                append(resultScript, "{}", i, resultAlloc);
            break;
        case DataAsString:
            append(resultScript, info.s, i, resultAlloc);
            break;
        case DataAsPointer:
            sprintf(value, "@%lx", (unsigned long)info.p);
            append(resultScript, value, i, resultAlloc);
            break;
    }
}

/*---------------------------------------------------------------------------*/
void EventHandler::scanForPercent()
{
    const char *p;

    hasPercent = GL_FALSE;
    for (p = handler.script; *p; p++)
    {
        if (*p == '%')
        {
            hasPercent = GL_TRUE;
            return;
        }
    }
}
