#include "../_Tools/aes.h"
#include "../_Tools/tools_IO.h"
//----------------------------------------------------------------------------------------------------------------------------------------
void DecryptTheFile(const char* szFileNameIn, const char* szFileNameOut,size_t size=0,size_t size_out=0)
{
	uint8_t key[] = { 0x10, 0x01, 0x02, 0x03,0x04, 0x05, 0x06, 0x07,0x08, 0x09, 0x0a, 0x0b,0x0c, 0x0d, 0x0e, 0x0f,0x10, 0x11, 0x12, 0x13,0x14, 0x15, 0x16, 0x17,0x18, 0x19, 0x1a, 0x1b,0x1c, 0x1d, 0x1e, 0x1f };
	AES* Aes = new AES(sizeof(key), key);//!!! check if destructor needed
	Aes->DecryptFileOnDisk(szFileNameIn, szFileNameOut, size_out);
}
//----------------------------------------------------------------------------------------------------------------------------------------
void DecryptAndRemoveFilesInFolder(const char* szFolderName,const char* szExtIn, const char* szExtOut,char mode = 'b')
{
	std::vector<std::string> names;
	GetFileNames(szFolderName, szExtIn, names);
	for (size_t i = 0; i < names.size(); i++)
	{
		char szBufer[1024];
		sprintf(szBufer, "%s%s", szFolderName,names[i].c_str());
		int L = strlen(szBufer);
		szBufer[L-3] = szExtOut[strlen(szExtOut)-3];
		szBufer[L-2] = szExtOut[strlen(szExtOut)-2];
		szBufer[L-1] = szExtOut[strlen(szExtOut)-1];
		DecryptTheFile((std::string(szFolderName)+names[i]).c_str(), szBufer, mode);
		DeleteFileA((std::string(szFolderName) + names[i]).c_str());
	}

}
//----------------------------------------------------------------------------------------------------------------------------------------
void main()
{
	std::string sBaseFolderName("");
	DecryptAndRemoveFilesInFolder(sBaseFolderName.c_str(),"*.dat","*.txt",'t');
	
	char szBufer[1024];
	for (size_t i = 1; i <= 31; i++)
	{
		sprintf(szBufer, "%s%02d\\", sBaseFolderName.c_str(), i);
		DecryptAndRemoveFilesInFolder(szBufer, "*.dat", "*.jpg",'b');
	}
	
	/*uint8_t key[] = { 0x10, 0x01, 0x02, 0x03,0x04, 0x05, 0x06, 0x07,0x08, 0x09, 0x0a, 0x0b,0x0c, 0x0d, 0x0e, 0x0f,0x10, 0x11, 0x12, 0x13,0x14, 0x15, 0x16, 0x17,0x18, 0x19, 0x1a, 0x1b,0x1c, 0x1d, 0x1e, 0x1f };
	AES* Aes = new AES(sizeof(key), key);
	Aes->DecryptFileOnDisk("C:\\tmp\\06.dat", "C:\\tmp\\06.txt", 0);*/
	




}
//----------------------------------------------------------------------------------------------------------------------------------------