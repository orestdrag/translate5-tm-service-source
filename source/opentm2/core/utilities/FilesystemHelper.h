#ifndef _FILESYSTEM_HELPER_H_
#define _FILESYSTEM_HELPER_H_

#include <string>
#include "win_types.h"

class FilesystemHelper{
    static std::string FixPath(const std::string& path);
public:
    static FILE* CreateFile(const std::string& path, const std::string& mode);

    static int WriteToFile(const std::string& path, const char* buff, const int buffsize);
    static int WriteToFile(FILE* ptr, const char* buff, const int buffsize);
    static int WriteToFile(FILE* ptr, const void* buff, const int buffsize);

    static int ReadFile(const std::string& path, char* buff, const int buffSize, int& bytesRead);
    static int ReadFile(FILE* ptr, char* buff, const int buffSize, int& bytesRead);
    static int ReadFile(FILE* ptr, void* buff, const int buffSize, int& bytesRead);

    static int DeleteFile(const std::string& path);
    static int GetFileSize(const std::string& path);

    static FILE* OpenFile(const std::string& path, const std::string& mode);
    static int CloseFile(FILE* ptr);


    enum FilesystemHelpersMessage{
        FILEHELPER_NOT_IMPLEMENTED = -1,
        FILEHELPER_NO_ERROR = 0,
        FILEHELPER_ERROR_WRITE_FAULT = ERROR_WRITE_FAULT,
        FILEHELPER_FILE_PTR_IS_NULL = ERROR_PATH_NOT_FOUND,

    };
};

#endif
