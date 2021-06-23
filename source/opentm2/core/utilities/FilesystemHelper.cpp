#include "FilesystemHelper.h"
#include "FilesystemWrapper.h"
#include <fstream>

int FilesystemHelper::CreateFile(const std::string& path){
    std::ofstream outfile(path);
    outfile.close();

    return FILEHELPER_NO_ERROR;
}

FILE* FilesystemHelper::OpenFile(const std::string& path, const std::string& mode){
    FILE *ptr = fopen(path.c_str(), mode.c_str());
    return ptr;
}


int FilesystemHelper::DeleteFile(const std::string& path){
    return FILEHELPER_NO_ERROR;
}

int FilesystemHelper::CloseFile(FILE* ptr){
    return FILEHELPER_NOT_IMPLEMENTED;
}

int FilesystemHelper::WriteToFile(const std::string& path, char* buff, const int buffsize){
    FILE *ptr = OpenFile(path, "wb");
    fwrite(buff, buffsize, 1, ptr);
    CloseFile(ptr);
    return FILEHELPER_NO_ERROR;
}
int FilesystemHelper::ReadFile(const std::string& path, char* buff, const int buffSize, int& bytesRead){
    FILE *ptr = OpenFile(path, "rb");
    bytesRead = fread(buff, buffSize, 1, ptr);
    CloseFile(ptr);
    return FILEHELPER_NO_ERROR;
}

int FilesystemHelper::GetFileSize(const std::string& path){
    return FILEHELPER_NOT_IMPLEMENTED;
}
