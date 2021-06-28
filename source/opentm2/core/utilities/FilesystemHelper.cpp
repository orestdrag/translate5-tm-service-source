#include "FilesystemHelper.h"
#include "FilesystemWrapper.h"
#include "PropertyWrapper.H"
#include "EQF.H"
#include <fstream>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <dirent.h>

int __last_error_code = 0;

std::string parseDirectory(const std::string path){
    std::size_t found = path.rfind('/');
    if (found!=std::string::npos)
        return path.substr(0,found);
    return path;
}
std::string parseFilename(const std::string path){
    std::size_t found = path.rfind('/');

    if (found!=std::string::npos)
        return path.substr(found+1);
    return path;
}

std::string FilesystemHelper::FixPath(const std::string& path){
    std::string ret;

    //fix back slash 
    for(int i = 0; i < path.size(); i++){
        if( ( (i+1) < path.size()) && path[i] == '\\' && path[i+1] == '\\'){
            ret.push_back('/');
            i++;
        }else if(path[i] == '\\'){
            ret.push_back('/');
        }else if( ((i+1) < path.size()) && path[i]=='/' && path[i+1] == '/'){
            ret.push_back('/');
            i++;
        }else {
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


int FilesystemHelper::CloseFile(FILE*& ptr){
    if(ptr){
        fclose(ptr);
    }
    ptr = NULL;
    return FILEHELPER_NO_ERROR;
}

#ifdef __USING_FILESYSTEM
FILE* FilesystemHelper::FindFirstFile(const std::string& name){
    auto files = FindFiles(name);
    if(files.empty())
        return NULL;
    auto path = files[0].path().generic_string();
    return OpenFile(path, "wb");
}

std::vector<fs::directory_entry> FilesystemHelper::FindFiles(const std::string& name){
    const std::string fixedName = FixPath(name);
    const std::string dirPath = parseDirectory(name);
    std::string fileName = parseFilename(name);
    
    int pos = fileName.rfind('*');
    bool exactMatch = pos == std::string::npos;

    if(!exactMatch)
        fileName = fileName.substr(pos+1);

    std::vector< fs::directory_entry> files;
    for (const auto & file : fs::directory_iterator(dirPath)){
        if(exactMatch){
            if(file.path().generic_string().compare(fileName) == 0){
                files.push_back(file);
            }
        }else{
            if( file.path().generic_string().find(fileName) != std::string::npos ){
                files.push_back(file);
            }
        }
    }

    return files;
}
#endif



std::vector<std::string> selFiles;
int curSelFile = -1;
FILE* FilesystemHelper::FindFirstFile(const std::string& name){
    auto files = FindFiles(name);
    if(selFiles.empty()){
        __last_error_code == FILEHELPER_ERROR_NO_FILES_FOUND;
        return NULL;
    }
    curSelFile = 0;
    auto path = selFiles[curSelFile];
    return OpenFile(path, "wb");
}

FILE* FilesystemHelper::FindNextFile(){
    
    if(selFiles.empty()){
        __last_error_code == FILEHELPER_ERROR_NO_FILES_FOUND;
        return NULL;
    }

    curSelFile++;
    if(curSelFile >= selFiles.size()){
        __last_error_code == FILEHELPER_END_FILELIST;
        return NULL;
    }
    auto path = selFiles[curSelFile];
    return OpenFile(path, "wb");
}

std::vector<std::string> FilesystemHelper::FindFiles(const std::string& name){
    selFiles.clear();
    const std::string fixedName = FixPath(name);
    const std::string dirPath = parseDirectory(fixedName);
    std::string fileName = parseFilename(fixedName);
    
    int pos = fileName.rfind('*');
    bool exactMatch = pos == std::string::npos;

    if(!exactMatch)
        fileName = fileName.substr(pos+1);

 
    DIR *dir; struct dirent *diread;

    if ((dir = opendir(dirPath.c_str())) != nullptr) {
        while ((diread = readdir(dir)) != nullptr) {
            std::string file = diread->d_name;
            if(exactMatch){
                if(file.compare(fileName) == 0){
                        selFiles.push_back(file);
                    }
            }else{
                if( file.find(fileName) != std::string::npos ){
                    selFiles.push_back(file);
                }
            }
        }
        closedir (dir);
    } else {
        __last_error_code = FILEHELPER_ERROR_CANT_OPEN_DIR;
    }

    return selFiles;
}


int FilesystemHelper::WriteToFile(const std::string& path, const char* buff, const int buffsize){
    std::string fixedPath = FixPath(path);
    FILE *ptr = OpenFile(fixedPath, "wb");
    int errCode = WriteToFile(ptr, buff, buffsize);
    if(errCode == FILEHELPER_NO_ERROR){
        CloseFile(ptr);
    }
    return errCode;
}


int FilesystemHelper::WriteToFile(FILE*& ptr, const void* buff, const int buffsize){
    int errCode = FILEHELPER_NO_ERROR;
    if(ptr == NULL){
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else if ( fwrite(buff, buffsize, 1, ptr) != 1 ){
        errCode = ERROR_WRITE_FAULT;
    }
    //CloseFile(ptr);
    return errCode;
}

int FilesystemHelper::WriteToFile(FILE*& ptr, const char* buff, const int buffsize){
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

int FilesystemHelper::ReadFile(FILE*& ptr, char* buff, const int buffSize, int& bytesRead){
    int errCode = FILEHELPER_NO_ERROR;
    if(!ptr){
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{
        bytesRead = fread(buff, buffSize, 1, ptr);
        CloseFile(ptr);
    }
    return errCode;
}


int FilesystemHelper::ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead){
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

    FILE* ptr = OpenFile(path, "rb");
    if(!ptr){
        return -1;
    }
    int size = GetFileSize(ptr);
    CloseFile(ptr);
    return size;
}


int FilesystemHelper::GetFileSize(FILE*& ptr){
    if(!ptr){
        __last_error_code = FILEHELPER_FILE_PTR_IS_NULL;
        return -1;
    }
    int size = fseek(ptr, 0L, SEEK_SET);

    return size;
}

int FilesystemHelper::GetLastError(){
    return __last_error_code;
}

int FilesystemHelper::ResetLastError(){
    __last_error_code = FILEHELPER_NO_ERROR;
    return 0;
}


std::string FilesystemHelper::GetOtmDir(){
    int maxPath = 255;
    char OTMdir[maxPath];
    int res = properties_get_str(KEY_OTM_DIR, OTMdir, maxPath);
    
    //property OTM_dir must be saved during property_init, if not setup- then there were not property_init_call
    if(!strlen(OTMdir) || res != PROPERTY_NO_ERRORS){
        properties_init();
        res = properties_get_str(KEY_OTM_DIR, OTMdir, maxPath);
    }

    return OTMdir;
}

std::string FilesystemHelper::GetHomeDir(){
    int maxPath = 255;
    char HOMEdir[maxPath];
    int res = properties_get_str(KEY_HOME_DIR, HOMEdir, maxPath);
    
    //property HOME_Dir must be saved during property_init, if not setup- then there were not property_init_call
    if(!strlen(HOMEdir) || res != PROPERTY_NO_ERRORS){
        properties_init();
        res = properties_get_str(KEY_HOME_DIR, HOMEdir, maxPath);
    }
    return HOMEdir;
}

int FilesystemHelper::CreateDir(const std::string& dir, int rights) {
    struct stat st;
    int ret = stat(dir.c_str(), &st);
    if (ret)
        ret = mkdir(dir.c_str(), rights);
    return ret;
}
