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

FILE* FilesystemHelper::CreateFile(const std::string& path, const std::string& mode){
    std::string fixedPath = FixPath(path);

    FILE* fp = fopen(fixedPath.c_str(), mode.c_str());

    return fp;
}

FILE* FilesystemHelper::OpenFile(const std::string& path, const std::string& mode){
    std::string fixedPath = FixPath(path);
    FILE *ptr = fopen(fixedPath.c_str(), mode.c_str());
    return ptr;
}


int FilesystemHelper::DeleteFile(const std::string& path){
    std::string fixedPath = FixPath(path);
    if(int errCode = remove(path.c_str())){
        return errCode;
    }else{
        return FILEHELPER_NO_ERROR;
    }
}
/*
int FilesystemHelper::DeleteFile(FILE*  ptr){
    if(!ptr)
        return FILEHELPER_FILE_PTR_IS_NULL;
    
    return FILEHELPER_NO_ERROR;
}//*/


int FilesystemHelper::CloseFile(FILE* ptr){
    if(ptr){
        fclose(ptr);
    }
    return FILEHELPER_NO_ERROR;
}

int FilesystemHelper::WriteToFile(const std::string& path, const char* buff, const int buffsize){
    std::string fixedPath = FixPath(path);
    FILE *ptr = OpenFile(fixedPath, "wb");
    return WriteToFile(ptr, buff, buffsize);
}


int FilesystemHelper::WriteToFile(FILE* ptr, const void* buff, const int buffsize){
    int errCode = FILEHELPER_NO_ERROR;
    if(ptr == NULL){
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else if ( fwrite(buff, buffsize, 1, ptr) != 1 ){
        errCode = ERROR_WRITE_FAULT;
    }
    CloseFile(ptr);
    return errCode;
}

int FilesystemHelper::WriteToFile(FILE* ptr, const char* buff, const int buffsize){
    int errCode = FILEHELPER_NO_ERROR;
    if(ptr == NULL){
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else if ( fwrite(buff, buffsize, 1, ptr) != 1 ){
        errCode = ERROR_WRITE_FAULT;
    }
    CloseFile(ptr);
    return errCode;
}


int FilesystemHelper::ReadFile(const std::string& path, char* buff, const int buffSize, int& bytesRead){
    std::string fixedPath = FixPath(path);
    FILE *ptr = OpenFile(fixedPath, "rb");

    return ReadFile(ptr, buff, buffSize, bytesRead);
}

int FilesystemHelper::ReadFile(FILE* ptr, char* buff, const int buffSize, int& bytesRead){
    int errCode = FILEHELPER_NO_ERROR;
    if(!ptr){
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{
        bytesRead = fread(buff, buffSize, 1, ptr);
        CloseFile(ptr);
    }
    return errCode;
}

int FilesystemHelper::GetFileSize(const std::string& path){
    std::string fixedPath = FixPath(path);

    return FILEHELPER_NOT_IMPLEMENTED;
}

