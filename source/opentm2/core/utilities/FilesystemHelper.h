#ifndef FILESYSTEM_HELPER_H
#define FILESYSTEM_HELPER_H

#include <string>


class FilesystemHelper{
public:
    static int CreateFile(const std::string& path);
    static int WriteToFile(const std::string& path, char* buff, const int buffsize);
    static int ReadFile(const std::string& path, char* buff, const int buffSize, int& bytesRead);
    static int DeleteFile(const std::string& path);
    static int GetFileSize(const std::string& path);

    static FILE* OpenFile(const std::string& path, const std::string& mode);
    static int CloseFile(FILE* ptr);


    enum FilesystemHelpersMessage{
        FILEHELPER_NO_ERROR = 0,
        
        FILEHELPER_FILE_PTR_IS_NULL,

        FILEHELPER_NOT_IMPLEMENTED,
    };
};

#endif
