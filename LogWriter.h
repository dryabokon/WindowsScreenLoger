#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <atlimage.h>
#include <Windows.h>
#include <winuser.h>
//----------------------------------------------------------------------------------------------------------------------------------------
#include "aes.h"
#include "tools_IO.h"
//----------------------------------------------------------------------------------------------------------------------------------------
class LogWriter
{
public:
	LogWriter();
	int GetCaptureFreq();
	int	DoIteration();

private:
	char* GetFullStatistic(size_t &N);
	void UpdateStatistic(std::string Item);
	void StageStatistic(int DayNumber);
	void StageScreenshots(int DayNumber);
	void DoScreenshot(CImage* ScreenShot,float scale);
	
	void Tick();
	void ToTime(size_t Ticks, char* szTime);
	void GetTheLocalTime(std::string& result);
	
	void InitFolders();
	void CleanupFolder(const char* szFolderName);
	void CleanupFolderIfNeeded(int DayNumber);
	
	long Subsort(long i);
	
	void utf8_decode(const std::string &str, std::wstring& wstrTo);
	void utf8_encode(const std::wstring &wstr, std::string& strTo);
	AES *Aes;


	std::vector<size_t> IDs;
	std::vector<CImage*> Screenshots;
	std::vector<size_t> Sizes;
	std::vector<std::string> Captions;
	std::vector<long> Freqs;

	std::string sBaseFolderName;
	std::string sPrevStatsName;
	std::string sMagicWord;
	
	int	CaptureFreq		= 1000;
	int	Freq_Caption	= 1;
	int	Freq_WriteToDisk= 10;
	int EncryptionModeOn= 1;
	int SortedModeOn	= 1;
	int	Minimal_hits	= 20;
	CImage emptySCreenshot;

	long TickTime;
	size_t last_position;
};
//----------------------------------------------------------------------------------------------------------------------------------------
