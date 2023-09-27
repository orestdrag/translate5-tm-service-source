#ifndef _FILEBUFFER_INCLUDED_
#define _FILEBUFFER_INCLUDED_

#include <vector>
#include <string>
#include <memory>
#include <map>

enum  FileBufferStatus{
        CLOSED = 1,
        OPEN = 2,
        MODIFIED = 4,        
    };

struct FileBuffer{
    int status ;//FILEBUFFERSTATUS
    std::vector<unsigned char> data;
    size_t offset = 0;
    FILE* file = nullptr;
    std::string fileName;   
    long long originalFileSize = 0;

    virtual int ReadFromFile();
    //int WriteToFile(); 
    virtual int Flush();
    int SetOffset(size_t newOffset, int fileAnchor);

    int Write(const void* buff, size_t buffSize, size_t startingPosition);
    int Write(const void* buff, size_t buffSize);
    int Read(void* buff, size_t buffSize, size_t startingPosition);
    int Read(void* buff, size_t buffSize);
    virtual ~FileBuffer()=default;

} ;


#endif//_FILEBUFFER_INCLUDED_