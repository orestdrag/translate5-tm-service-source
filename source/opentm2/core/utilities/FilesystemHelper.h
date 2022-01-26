#ifndef _FILESYSTEM_HELPER_H_
#define _FILESYSTEM_HELPER_H_

#include <string>
#include "win_types.h"
#include <vector>
#include <map>
#include <mutex>

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
    //FILE* file = NULL;
    std::mutex lock;
    
} ;

typedef std::map <std::string, FileBuffer> FileBufferMap, *PFileBufferMap;

PFileBufferMap getFileBufferInstance();

class FilesystemHelper{
public:

    //helping functions
    static std::string FixPath     (std::string& path);
    static std::string GetFileName (HFILE ptr);
    static std::string GetOtmDir   ();
    static std::string GetHomeDir  ();
    static  std::vector<std::string> FindFiles(std::string&& name);

    //functions to work with filesystem
    static int  CreateFile (std::string&& path, std::string&& mode);
    static int  DeleteFile (std::string&& path);
    static int  MoveFile   (std::string&& oldPath, std::string&& newPath);    
    static int  CreateDir  (std::string&& path, int rights = 0700);
    static bool DirExists  (std::string&& path);

    //functions to work with filebuffers
    static bool FilebufferExists     (std::string&& fName);
    static int LoadFileToFileBuffer  (std::string&& fName);
    static int CloseFileBuffer       (std::string&& fName);
    static int WriteToFileBuffer     (std::string&& fName, const void* buff, const size_t buffSize, size_t &iBytesWritten, const size_t startingPosition = -1);
    static int ReadFileBuffer        (std::string&& fName, void* buff, const size_t buffSize, size_t& bytesRead, const size_t startingPosition = -1);
    static int SetOffsetInFilebuffer (std::string&& fName, size_t offset);

    static int WriteToFileBuffer     (HFILE fPtr, const void* buff, const size_t buffSize, size_t &iBytesWritten, const size_t startingPosition = -1);
    static int ReadFileBuffer        (HFILE fPtr, void* buff, const size_t buffSize, size_t& bytesRead, const size_t startingPosition = -1);
    static int SetOffsetInFilebuffer (HFILE fPtr, size_t offset);

    static int CreateFilebuffer     (std::string&& name);// don't create file on disk 
    static std::vector<UCHAR>* GetFileBufferData(std::string&& name);
    
    static size_t GetFileBufferSize (std::string&& path);
    static size_t GetTotalFileBuffersSize();

    static int FlushBufferToFile(std::string&& fName);
    static int FlushAllBuffers();

    static int ReadFile(std::string&& path, char* buff, const size_t buffSize, 
                            size_t& bytesRead, const size_t startingPos = -1, std::string&& mode = "rb");


    static size_t GetFileSizeDisk (const std::string& path);
    static size_t GetFileSizeDisk (FILE*& ptr);
    static bool   FileExistsDisk(std::string&& path);
    static FILE*  OpenFileDisk(const std::string& path, const std::string& mode);
    static int    CloseFileDisk(FILE*& ptr);

    static int    WriteToFileDisk   (std::string fName, const void* buff, const size_t buffSize, size_t &iBytesWritten, const size_t startingPosition = -1);
    static int    WriteToFileDisk   (HFILE pFile, const void* buff, const size_t buffSize, size_t &iBytesWritten, const size_t startingPosition = -1);
    static int    ReadFileFromDisk  (std::string fName, const void* buff, const size_t buffSize, size_t& iBytesRead);
    static int    ReadFileFromDisk  (HFILE fName, const void* buff, const size_t buffSize, size_t& iBytesRead);
    static short  SetFileCursorDisk (HFILE fp,long LoPart,long& HiPart,size_t OffSet);  

    //static FILE* FindFirstFile(const std::string& name);
    //static FILE* FindNextFile();
    //static int WriteToFile(const std::string& path, const char* buff, const int buffsize);
    //static int WriteToFile(FILE*& ptr, const void* buff, const int buffsize);
    //static int ReadFile(FILE*& ptr, char* buff, const int buffSize, int& bytesRead);
    //static int ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead);
    //static int ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPos = 0);    

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
        FILEHELPER_WARNING_FILEBUFFER_EXISTS
    };

private:

    static int TruncateFileForBytes(HFILE ptr, int numOfBytes);

    //buffers
    //static int WriteToBuffer(FILE *& ptr, const void* buff, const int buffSize, const int startingPosition);
    //static int FlushBufferIntoFile(const std::string& fName);
    //static int ReadBuffer(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPos);
};

#endif
