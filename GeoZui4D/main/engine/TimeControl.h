#ifndef TIMECONTROL_H
#define TIMECONTROL_H

#include "SDV_types.h"

#include "engine/ToglWindow.h"

class RTSource;
class TimePieces;

/** This controls time aspects of geoZui.
 * \sa Scene3D
 * \author Roland Arsenault (rja)
 * \date 2002?
 * \image html GeoZui_time.jpg
*/
class TimeControl: public ToglWindow
{
public:
	TimeControl();

	void initDisplay();
	
	void updateCurrentTime(int frameFlag = GL_FALSE);
	double getFrameTime(){return frameTime;};
	void getSysTimeStr(char * tstr);
	static void getTimeStr(char * tstr,double tm);
	void setBBox(BBox * ptrBBox);
	BBox *getBBox() {return bbox;}
	double getGZTime(){return currentGZTime;}
	double getScale(){return scale;}
        double getOffset(){return offset;}
	double getSysTime(){return currentSysTime;};
	void setGZTime(double time,int fractionFlag=0);
	void RegisterWithTcl();
	void stop();
	void play();
	void rev();
	void multiplyScale(double mult);
	void setTimeAndScale(double time, double scale);
        void setOffsetAndScale(double offset, double scale);
	void print(double dt);
	void setRTsource(RTSource * s){rt = s;};
	RTSource *getRTsource(){return rt;};
	void useRT();
	void dontuseRT(){usingRT = 0;};
	void repeat(){loopAtEnd = 1;};
	void dontRepeat(){loopAtEnd = 0;};
	void loop();
	void dontLoop(){looping = 0;};
	void setLoopStart();
	void setLoopEnd();
	int isRT();
	int isPaused(){return paused;};
	void setRecording(int r = 1){recording  = r;};
	void setTimezone(float t);
	float getTimeZone() {return timezone;};

	// togl stuff
	void Display();
	void Reshape(int x = 0, int y = 0, int w = 0, int h = 0);

protected:
	
	virtual int HandleInput(unsigned int inputType, InputInfo *info);
	void drawTicks(double interval, double startPosition, int startTime, int loopLimit, char const * displayFormat, int subInterval, float bottom, float top, float left, float right);
	void drawTimeScale(float bottom, float top, float left, float right);
	
private:
  int isLeap(long y);
  void initButtons();

	TimePieces *tp, *tstartp;

	double currentSysTime;  // system time in seconds
	double lastSysTime; // time of previous frame
static	double currentGZTime;	// transformed sys time
	double lastGZTime;	// gz time of last frame
	double scale;	// how to scale sys time to gz time
	double offset;	// offset from sys time to gz time
	double frameTime;
	
	double targetTime; // time to which we are translating 
	double startTranslateGZTime;
	double startTranslateSysTime;
	double lastZoomTime;
	int mouseY;
	double zooming;

    	BBox * bbox;

    	int timeScaleVisible;

   	static char * GZtimestr;
   	static char * oldGZtimestr;
    static double GZtimeFraction;
   	static int tcl;

	// real-time time source (external to system time)
	RTSource * rt;
	int usingRT;

	// Looping
	double loopStart;
	double loopEnd;
	int looping, loopAtEnd;
	
	//buttons
	GLubyte buttonPaused[48];
	GLubyte buttonUnpaused[48];
	GLubyte buttonForwardActive[48];
	GLubyte buttonForward[48];
	GLubyte buttonReverseActive[48];
	GLubyte buttonReverse[48];
	GLubyte buttonRTActive[48];
	GLubyte buttonRTInactive[48];
	GLubyte buttonOneActive[48];
	GLubyte buttonOneInactive[48];
	
	int paused;
	
	int startX,scrollClick; // where we started clicking and dragging time
	int startY;
	double startTime; // time we started clicking and dragging time
	double startScale;
	double spp; // seconds per pixel
	
	int scrollStart,scrollEnd,scrollAreaStart, scrollAreaEnd,scrollAreaWidth,scrollMiddle;
	double pageBackStart, pageFwdStart;
	
	Point3f bgColor, tickColor, textColor;
	
	float timezone;
	
	// video recording
	int recording;
};

#endif
