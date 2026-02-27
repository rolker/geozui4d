#include"Isotrack.h"
#include<iostream>
#include<stdio.h>
#include<gz4d/glo.h>
#include<GL/glut.h>

using namespace std;

#ifdef WIN32
HANDLE Isotrack::hComm = NULL;
#endif
float Isotrack::x[4] = {0.0,0.0,0.0,0.0};
float Isotrack::y[4] = {0.0,0.0,0.0,0.0};
float Isotrack::z[4] = {0.0,0.0,0.0,0.0};
float Isotrack::q1[4] = {0.0,0.0,0.0,0.0};
float Isotrack::q2[4] = {0.0,0.0,0.0,0.0};
float Isotrack::q3[4] = {0.0,0.0,0.0,0.0};
float Isotrack::q4[4] = {0.0,0.0,0.0,0.0};

#ifdef WIN32
CRITICAL_SECTION * Isotrack::CriticalSection = NULL;
#endif

/* time test
#include <sys/timeb.h>
#include <time.h>
struct _timeb startTime, endTime;
int timeCounter;
*/

void Isotrack::GetPosition(int sensor, double * X, double * Y, double * Z,
						   double * Q1, double * Q2, double * Q3, double * Q4)
{
	sensor -= 1;
#ifdef WIN32
	EnterCriticalSection(CriticalSection);
#endif
//	cout << "Get Value: " << sensor << "\n";
	*X = x[sensor];
	*Y = y[sensor];
	*Z = z[sensor];
	*Q1 = q1[sensor];
	*Q2 = q2[sensor];
	*Q3 = q3[sensor];
	*Q4 = q4[sensor];

	//cout << "Isotrack::GetPosition, x: " << *X << ", y: " << *Y << ", z:" << *Z << endl;
	//cout << "Isotrack::GetPosition, x[sensor]: " << x[sensor] << ", y[sensor]: " << y[sensor] << ", z[sensor]:" << z[sensor] << endl;

//	cout << "Get value done.\n";
#ifdef WIN32
	LeaveCriticalSection(CriticalSection);
#endif
}



Isotrack::Isotrack()
{
}

void Isotrack::SetQuaternionFastrack()
{
#ifdef WIN32
	DWORD SizeWriten;
	char outstr[100];
	int i;

	for (i=1; i<=4; i++)
	{
		sprintf(outstr, "O%i,2,11,1\r",i);
		WriteFile(hComm, outstr, 10, &SizeWriten, 0);
	}
#endif
}

void Isotrack::SetQuaternionIsotrack()
{
#ifdef WIN32
	DWORD SizeWriten;
	char outstr[100];

	sprintf(outstr, "O2,11,1\r");
	WriteFile(hComm, outstr, 8, &SizeWriten, 0);
#endif
}

int Isotrack::Initialize(int d)
{
	
	// Open com port
#ifdef WIN32
	
	hComm = CreateFile( "COM1",  
		GENERIC_READ | GENERIC_WRITE, 
		0, 
		0, 
		OPEN_EXISTING,
		0,
		0);
	if (hComm == INVALID_HANDLE_VALUE)
	{
		cout << "Error opening port\n";
		return(1);
	}
	
	// set timeouts
	
	COMMTIMEOUTS cto = { MAXDWORD, 0, 0, 0, 0 };
	
	
	if(!SetCommTimeouts(hComm, &cto))
	{
		cout << "Error opening port\n";
		return(1);
	}
	
	// set DCB
	
	DCB dcb;
	
	memset(&dcb, 0, sizeof(dcb));
	
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = 115200;
	dcb.fBinary = 1;
	dcb.fParity = 0;
	dcb.fOutxCtsFlow = 0;
	dcb.fOutxDsrFlow = 0;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = 0;
	dcb.fTXContinueOnXoff = TRUE;
	dcb.fOutX = 0;
	dcb.fInX = 0;
	dcb.fErrorChar = 0;
	dcb.fNull = 0;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fAbortOnError = 0;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	
	if(!SetCommState(hComm, &dcb))
	{
		cout << "Error opening port\n";
		return(1);
	}
	
	// Reset
	
	char Key[1];
	
	Key[0] = 25;
	
	DWORD SizeWritten;
	
	WriteFile(hComm, "\r", 1, &SizeWritten, 0);
	WriteFile(hComm, Key, 1, &SizeWritten, 0);
	if (d == 1)
	{
		cout << "Resetting";
		cout.flush();
	}
	
	Sleep(250);
	
	
	while(GetStatus())
	{
		if (d == 1)
		{
			cout << ".";
			cout.flush();
		}
		Sleep(250);
	}
	
	
	if (d == 1)
	{
		cout << "Done\n";
		cout.flush();
	}
	
	//SetQuaternionFastrack();
	SetQuaternionIsotrack();
	
	// set units to cm

	WriteFile(hComm, "u", 1, &SizeWritten, 0);

	// set binary format
	
	//	WriteFile(hComm, "f", 3, &SizeWritten, 0);

	// Set up critical section
	
	CriticalSection = new CRITICAL_SECTION;

	InitializeCriticalSection(CriticalSection); 
#endif	
	
	return 0;	
}

void Isotrack::Calibrate()
{
#ifdef WIN32
	DWORD SizeWriten;
	//	char command[100];
	
	cout << "Calibrating...\n";
	cout.flush();
	
    WriteFile(hComm, "R1\r", 3, &SizeWriten, 0);
	
	while(GetStatus())
	{
		Sleep(250);
	}
	
	cout << "Done.\n";
	cout.flush();
#endif	
}

int Isotrack::GetStatus()
{
#ifdef WIN32
	DWORD SizeRead, SizeWritten;
	char Buffer[100];
	
	Buffer[0] = 0;
	
	WriteFile(hComm, "S", 1, &SizeWritten, 0);
	ReadFile(hComm, Buffer, 100, &SizeRead, 0);
	
	while (Buffer[0] == '2' && Buffer[1] == '1' && Buffer[2] == '*')
	{
		WriteFile(hComm, "\rS", 2, &SizeWritten, 0);
		ReadFile(hComm, Buffer, 100, &SizeRead, 0);
//		cout << Buffer;
//		cout.flush();
	}
	
	if (Buffer[0] == '2' && Buffer[1] == '1' && Buffer[2] == 'S')
	{
		return 0;
	}
	else
	{
		return 1;
	}
#endif
	return 0;
}

#ifdef WIN32
long WINAPI Isotrack::PositionThread(long dummy)
{
	DWORD SizeRead, SizeWritten;
	char Buffer[100];
	int i;
	char dc;
	int sensor;
	
	WriteFile(hComm, "C", 1, &SizeWritten, 0);
	
	/* time test
	_ftime( &startTime );
	timeCounter = 0;
	*/

	while (1)
	{
		SizeRead = 0;
		while (SizeRead == 0)
		{
			ReadFile(hComm, Buffer, 1, &SizeRead, 0);
		}
		while (Buffer[0] != '0')
		{
			SizeRead = 0;
			while (SizeRead == 0)
			{
				ReadFile(hComm, Buffer, 1, &SizeRead, 0);
			}
		}
		
		SizeRead = 0;
		while (SizeRead == 0)
		{
			ReadFile(hComm, &Buffer[1], 1, &SizeRead, 0);
		}
		
		if(Buffer[1] >= '1' && Buffer[1] <= '4')
		{
			sensor = Buffer[1]-'1'; // change the ascii value to an int, also subtract 1.
//			cout << sensor << "\n";
			for (i=2; i <= 52; i++)
			{
				SizeRead = 0;
				while (SizeRead == 0)
				{
					ReadFile(hComm, &Buffer[i], 1, &SizeRead, 0);
				}
			}
			Buffer[53]=0;
			EnterCriticalSection(CriticalSection); 
//			cout << Buffer << "\n";
			sscanf(&Buffer[2],"%c %f%f%f%f%f%f%f",&dc,&x[sensor],&y[sensor],&z[sensor],&q1[sensor],&q2[sensor],&q3[sensor],&q4[sensor]);
			LeaveCriticalSection(CriticalSection);

		}

		/* time test
		if( ++timeCounter == 100 )
		{
			_ftime( &endTime );

			unsigned long timeDelta_ms = 1000 * ( endTime.time - startTime.time )
										+ (unsigned long)( endTime.millitm - startTime.millitm );

			cout << "Avg. Isotrack time [ms]: " << timeDelta_ms/timeCounter << endl;

			_ftime( &startTime );
			timeCounter = 0;
		}
		*/
		
	}
	
	return 0;
}
#endif

void Isotrack::LaunchPositionThread()
{
#ifdef WIN32
	LPDWORD id = 0;

	hPositionThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)PositionThread,NULL,0,id);

	//SetThreadPriority(hPositionThread,THREAD_PRIORITY_ABOVE_NORMAL);
#endif
}


void Isotrack::PrintValues()
{
#ifdef WIN32
	int i;

	EnterCriticalSection(CriticalSection);
	for (i = 0; i <=3; i++)
	{
		cout << i+1 << ": " << x[i] << ", " << y[i] << ", " << z[i] << ", " << q1[i] << ", " << q2[i] << ", " << q3[i] << ", " << q4[i] << "\n";
	}
	cout.flush();
	LeaveCriticalSection(CriticalSection);
#endif
}

