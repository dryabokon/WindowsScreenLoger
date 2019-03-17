#pragma once
#include <vector>
#include <Windows.h>
#include <winuser.h>
//----------------------------------------------------------------------------------------------------------------------------------------
void GetFileNames(const char* szInputSearchFolder, const char* szInputSearchExtension, std::vector<std::string>& name);
//----------------------------------------------------------------------------------------------------------------------------------------