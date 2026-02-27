/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
//  InputInfo.h
///////////////////////////////////////////////////////////////////////////////

#ifndef INPUTEVENTINFO_H
#define INPUTEVENTINFO_H

#ifndef NULL
#define NULL 0
#endif

class ToglWindow;
class InputMediator;
class Addressable;
struct Togl; /* added by mdp 7/13/2000 */
/**
 * This is a storage class for event information for Interactors.
 *
 * \author Matt Plumlee (mdp)
 * \date 3/2/2000
 */
class InputInfo
{   public:
        InputInfo(ToglWindow *originator, int inputType = 0,
                    int additionalInfo = 0, int virtualEvent = 0);
        InputInfo(int inputType = 0, int additionalInfo = 0,
                            int virtualEvent = 1);
        ~InputInfo();

        /* Common Values */
        int type;										//%T
        long serialNumber;								//%S
        int virtualEvent; /* true or false value */		//%v
        ToglWindow *window;								//%W
        InputMediator *eventStream;						//%E
        const int numAdditionalInfo;					//%N

        /* Generic union for following types */
        union polyInfo
//        class polyInfo // changed to class by rja to prevent any value from getting cast to an Addressable which caused a crash in EventHandler GeneralScript case.
//Changed back to union by mdp 2/8/2007 because using class would require a rework on the Trigger calls (doable, but takes a bit longer and may not track everything down right away)
         {
         public:
//             polyInfo(){i=0;l=0;d=0.0;s=NULL;a=NULL;p=NULL;};
             int i;										//%i (prefix)
             long l;										//%l (prefix)
             double d;									//%d (prefix)
             const char *s;								//%s (prefix)
             Addressable *a;								//%a (prefix)
             const void *p;								//%p (prefix)
         };

        /* Generic info */
        polyInfo val;									//%V
        polyInfo modifier; /* added by mdp 7/18/2000 */	//%M

        /* Position info */
        polyInfo x;										//%X
        polyInfo y;										//%Y
        polyInfo z;										//%Z

        /* AdditionalInfo */
        polyInfo *const additionalInfo;					//%(#)

    protected:
        static long serialNumberGenerator;
};

#endif
