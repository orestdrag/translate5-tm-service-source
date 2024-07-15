#ifndef _FILEBUFFER_INCLUDED_
#define _FILEBUFFER_INCLUDED_

#include <vector>
#include <string>

enum  FileBufferStatus{
        CLOSED = 1,
        OPEN = 2,
        MODIFIED = 4,        
    };

enum FilebufferType{
    OTHER = 0,
    TMD_FILEBUFFER,
    TMI_FILEBUFFER,
};

struct FileBuffer{
    int status ;//FILEBUFFERSTATUS
    std::vector<unsigned char> data;
    size_t offset = 0;
    FILE* file = nullptr;
    std::string fileName;   
    FilebufferType filebufferType;
    long long originalFileSize = 0;

    int ReadFromFile();
    //int WriteToFile(); 
    int Flush(bool forced = false);
    int SetOffset(size_t newOffset, int fileAnchor);

    int Write(const void* buff, size_t buffSize, size_t startingPosition);
    int Write(const void* buff, size_t buffSize);
    int Read(void* buff, size_t buffSize, size_t startingPosition);
    int Read(void* buff, size_t buffSize);

    bool isTMDFilebuffer();

} ;

#endif//_FILEBUFFER_INCLUDED_