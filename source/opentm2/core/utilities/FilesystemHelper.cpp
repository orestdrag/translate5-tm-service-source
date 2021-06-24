#include "FilesystemHelper.h"
#include "FilesystemWrapper.h"
#include <fstream>

std::string FilesystemHelper::FixPath(const std::string& path){
    std::string ret;
    //fix double back slash
    for(int i = 1; i < path.size(); i++){
        if(path[i-1] == '\\' && path[i] == '\\'){
            ret.push_back('/');
            i++;
        }else if(path[i] == '\\'){
            ret.push_back('/');
        }else{
            ret.push_back(path[i]);
        }
    }
    return ret;
}

int FilesystemHelper::CreateFile(const std::string& path){
    std::string fixedPath = FixPath(path);
    std::ofstream outfile(fixedPath);
    outfile.close();

    return FILEHELPER_NO_ERROR;
}

FILE* FilesystemHelper::OpenFile(const std::string& path, const std::string& mode){
    std::string fixedPath = FixPath(path);
    FILE *ptr = fopen(fixedPath.c_str(), mode.c_str());
    return ptr;
}


int FilesystemHelper::DeleteFile(const std::string& path){
    return FILEHELPER_NO_ERROR;
}

int FilesystemHelper::CloseFile(FILE* ptr){
    return FILEHELPER_NOT_IMPLEMENTED;
}

int FilesystemHelper::WriteToFile(const std::string& path, char* buff, const int buffsize){
    std::string fixedPath = FixPath(path);
    FILE *ptr = OpenFile(fixedPath, "wb");
    fwrite(buff, buffsize, 1, ptr);
    CloseFile(ptr);
    return FILEHELPER_NO_ERROR;
}

int FilesystemHelper::ReadFile(const std::string& path, char* buff, const int buffSize, int& bytesRead){
    std::string fixedPath = FixPath(path);
    FILE *ptr = OpenFile(fixedPath, "rb");
    bytesRead = fread(buff, buffSize, 1, ptr);
    CloseFile(ptr);
    return FILEHELPER_NO_ERROR;
}

int FilesystemHelper::GetFileSize(const std::string& path){
    std::string fixedPath = FixPath(path);

    return FILEHELPER_NOT_IMPLEMENTED;
}

