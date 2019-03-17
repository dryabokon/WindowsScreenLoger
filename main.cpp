//----------------------------------------------------------------------------------------------------------------------------------------
#include <windows.h>
#include <process.h>
#include <time.h>
#include <direct.h>
#include "LogWriter.h"
//----------------------------------------------------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	LogWriter LW;
	while (true)
	{
		Sleep(LW.GetCaptureFreq());
		if (LW.DoIteration() !=0)return 0;
	}
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------

