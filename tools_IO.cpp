#pragma once
#include "tools_IO.h"
//----------------------------------------------------------------------------------------------------------------------------------------
void GetFileNames(const char* szInputSearchFolder, const char* szInputSearchExtension, std::vector<std::string>& name)
{
	name.clear();

	int				flag = false;
	WIN32_FIND_DATA FindFileData;
	char			szFullName[1024];

	sprintf_s(szFullName, "%s%s", szInputSearchFolder, szInputSearchExtension);
	HANDLE hFind = FindFirstFile(szFullName, &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE) { flag = true; }
	if (flag == false) return;
	
	do
	{
		if (  (std::string(FindFileData.cFileName) != std::string("..") && (std::string(FindFileData.cFileName) != std::string("."))))
		{
			name.push_back(FindFileData.cFileName);
		}
	} 
	while (FindNextFile(hFind, &FindFileData) != 0);
}
//----------------------------------------------------------------------------------------------------------------------------------------