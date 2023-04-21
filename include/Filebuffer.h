#ifndef _FILEBUFFER_INCLUDED_
#define _FILEBUFFER_INCLUDED_

#include <vector>
#include <string>

enum  FileBufferStatus{
        CLOSED = 1,
        OPEN = 2,
        MODIFIED = 4,        
    };

struct FileBuffer{
    int status ;//FILEBUFFERSTATUS
    std::vector<unsigned char> data;
    size_t offset = 0;
    FILE* file = NULL;
    std::string fileName;   
    long long originalFileSize = 0;

    size_t ReadFromFile();
    size_t WriteToFile(); 
    size_t Flush();
    int    SetOffset(size_t newOffset, int fileAnchor);

    size_t Write(const void* buff, size_t buffSize, size_t startingPosition);
    size_t Write(const void* buff, size_t buffSize);
    size_t Read(void* buff, size_t buffSize, size_t startingPosition);
    size_t Read(void* buff, size_t buffSize);


} ;

#endif//_FILEBUFFER_INCLUDED_