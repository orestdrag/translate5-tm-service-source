#ifndef _FILESYSTEM_HELPER_H_
#define _FILESYSTEM_HELPER_H_

#include <string>
#include "win_types.h"
#include <vector>

#ifdef _USING_FILESYSTEM_
#include <filesystem>
//#include <experimental/filesystem>
//namespace fs = std::filesystem;
namespace fs = std::experimental::filesystem;
#endif

class FilesystemHelper{
public:
    static std::string FixPath(const std::string& path);

    static FILE* CreateFile(const std::string& path, const std::string& mode);
    
    static int CreateDir(const std::string& path, int rights = 0700);
    static bool DirExists(const std::string& path);
    
    static int WriteToFile(const std::string& path, const char* buff, const int buffsize);
    static int WriteToFile(FILE*& ptr, const char* buff, const int buffsize);
    static int WriteToFile(FILE*& ptr, const void* buff, const int buffsize);

    static int ReadFile(const std::string& path, char* buff, const int buffSize, 
                            int& bytesRead, const std::string& mode = "rb");
    static int ReadFile(FILE*& ptr, char* buff, const int buffSize, int& bytesRead);
    static int ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead);

    static int DeleteFile(const std::string& path);
    static int GetFileSize(const std::string& path);
    static int GetFileSize(FILE*& ptr);

    static FILE* OpenFile(const std::string& path, const std::string& mode);
    static int CloseFile(FILE*& ptr);

    static FILE* FindFirstFile(const std::string& name);
    static FILE* FindNextFile();

#ifdef _USING_FILESYSTEM_
    static  std::vector<fs::directory_entry> FindFiles(const std::string& name);
#endif
    static  std::vector<std::string> FindFiles(const std::string& name);

    static int GetLastError();
    static int ResetLastError();

    enum FilesystemHelpersMessage{
        FILEHELPER_NOT_IMPLEMENTED = -1,
        FILEHELPER_NO_ERROR = 0,
        
        FILEHELPER_ERROR_WRITE_FAULT = ERROR_WRITE_FAULT,
        FILEHELPER_FILE_PTR_IS_NULL = ERROR_PATH_NOT_FOUND,
        FILEHELPER_END_FILELIST,

        FILEHELPER_ERROR_NO_FILES_FOUND,

        FILEHELPER_ERROR_CANT_OPEN_DIR,

    };

    static std::string GetOtmDir();
    static std::string GetHomeDir();
};

#endif
