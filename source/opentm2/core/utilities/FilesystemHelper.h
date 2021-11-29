#ifndef _FILESYSTEM_HELPER_H_
#define _FILESYSTEM_HELPER_H_

#include <string>
#include "win_types.h"
#include <vector>
#include <map>


#ifdef _USING_FILESYSTEM_
#include <filesystem>
//#include <experimental/filesystem>
//namespace fs = std::filesystem;
namespace fs = std::experimental::filesystem;
#endif

enum  FileBufferStatus{
        CLOSED = 1,
        OPEN = 2,
        MODIFIED = 4,
        
    };

struct FileBuffer{
    int status ;//FILEBUFFERSTATUS
    std::vector<UCHAR> data;
    long offset = 0;
    FILE* file = NULL;
    
} ;

typedef std::map <std::string, FileBuffer> FileBufferMap, *PFileBufferMap;

PFileBufferMap getFileBufferInstance();
PFileBufferMap GetPFileBufferMap();
void SetFileBufferMap(const PFileBufferMap pbm);

class FilesystemHelper{
public:
    static std::string FixPath(std::string& path);

    static FILE* CreateFile(const std::string& path, const std::string& mode);
    static int MoveFile(std::string oldPath, std::string newPath);
    
    static int CreateDir(const std::string& path, int rights = 0700);
    static bool DirExists(const std::string& path);

    static int WriteToFile(const std::string& path, const char* buff, const int buffsize);
    static int WriteToFile(const std::string& path, const unsigned char* buff, const int buffsize);
    //static int WriteToFile(FILE*& ptr, const char* buff, const int buffsize);
    static int WriteToFile(FILE*& ptr, const void* buff, const int buffsize);

    static int WriteToFile(FILE*& ptr, const void* buff, const long unsigned int buffSize, int &iBytesWritten, const int startingPosition);
    static int WriteToFileBuff(FILE*& ptr, const void* buff, const long unsigned int buffSize, int &iBytesWritten, const int startingPosition);
    static int ReadFileBuff(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPosition);
    static int SetOffsetInFilebuffer(FILE* ptr,int offset);

    static int ReadFile(const std::string& path, char* buff, const int buffSize, 
                            int& bytesRead, const std::string& mode = "rb");
    //static int ReadFile(FILE*& ptr, char* buff, const int buffSize, int& bytesRead);
    static int ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead);
    static int ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPos);

    static int DeleteFile(const std::string& path);
    static int GetFileSize(const std::string& path);
    static int GetFileSize(FILE*& ptr);

    static FILE* OpenFile(const std::string& path, const std::string& mode, bool useBuffer = true);
    static int CloseFile(FILE*& ptr);
    static int CloseFileBuffer(const std::string& path);

    static FILE* FindFirstFile(const std::string& name);
    static FILE* FindNextFile();

    static std::string GetFileName(HFILE ptr);
    
    static PFileBufferMap GetPFileBufferMap();
    static void SetFileBufferMap(const PFileBufferMap pbm);

    static int TruncateFileForBytes(HFILE ptr, int numOfBytes);
    static short SetFileCursor(HFILE fp,long LoPart,long& HiPart,short OffSet);
    

    static int WriteBuffToFile(std::string fName, bool tempFile = false);
    static int FlushAllBuffers();

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
        FILEHELPER_ERROR_FILE_NOT_READ,
        FILEHELPER_ERROR_READING_OUT_OF_RANGE,
        
        FILEHELPER_ERROR_NO_FILES_FOUND,

        FILEHELPER_ERROR_CANT_OPEN_DIR,

        FILEHELPER_WARNING_FILE_IS_SMALLER_THAN_REQUESTED,
        FILEHELPER_WARNING_BUFFER_FOR_FILE_NOT_OPENED,

    };

    static std::string GetOtmDir();
    static std::string GetHomeDir();

    private:

    //buffers
    static int WriteToBuffer(FILE *& ptr, const void* buff, const int buffSize, const int startingPosition);
    static int FlushBufferIntoFile(const std::string& fName);
    static int ReadBuffer(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPos);
};

#endif
