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
#include "LogWrapper.h"
#include "EQF.H"

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

std::string FilesystemHelper::FixPath(std::string& path){
    std::string ret;
    //
    if(path.find('/') == std::string::npos){
        char folderPath[255];
        properties_get_str(KEY_OTM_DIR, folderPath,255);
        properties_get_str_or_default(KEY_DEFAULT_DIR, folderPath, 255, folderPath);
        path = std::string(folderPath) + '/' + path;
    }
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
    return OpenFile(path, mode);
}

FILE* FilesystemHelper::OpenFile(const std::string& path, const std::string& mode){
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);
    FILE *ptr = fopen(fixedPath.c_str(), mode.c_str());
    LogMessage6(DEBUG, "FilesystemHelper::OpenFile():: path = ", fixedPath.c_str(), "; mode = ", mode.c_str(), "; ptr = ", intToA((long int)ptr));
    if(ptr == NULL){
        __last_error_code = FILEHELPER_FILE_PTR_IS_NULL;
    }
    return ptr;
}

std::string FilesystemHelper::GetFileName(HFILE ptr){
    const int MAXSIZE = 0xFFF;
    char filename[MAXSIZE];
    char proclnk[MAXSIZE];

    int fno = fileno(ptr);
    sprintf(proclnk, "/proc/self/fd/%d", fno);
    int r = readlink(proclnk, filename, MAXSIZE-1);
    filename[r] = '\0';

    return filename;
}


int FilesystemHelper::DeleteFile(const std::string& path){
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);
    if(int errCode = remove(path.c_str())){
        LogMessage4(ERROR, "FilesystemHelper::DeleteFile(",fixedPath.c_str() , ") ERROR res = ", intToA(errCode));
        return errCode;
    }else{
        LogMessage3(DEBUG, "FilesystemHelper::DeleteFile(",fixedPath.c_str() , ") res = FILEHELPER_NO_ERROR");
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
    LogMessage3(DEBUG, "FilesystemHelper::CloseFile(", intToA((long int) ptr) , ")");
    if(ptr){
        fclose(ptr);
    }
    ptr = NULL;
    return __last_error_code = FILEHELPER_NO_ERROR;
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

        LogMessage3(INFO, "FilesystemHelper::FindFiles(",name.c_str() , ") - files not found");
        __last_error_code == FILEHELPER_ERROR_NO_FILES_FOUND;
        return NULL;
    }
    curSelFile = 0;
    auto path = selFiles[curSelFile];
    return OpenFile(path, "wb");
}

FILE* FilesystemHelper::FindNextFile(){
    
    if(selFiles.empty()){
        LogMessage(INFO, "FilesystemHelper::FindNextFile()::FILEHELPER_ERROR_NO_FILES_FOUND");
        __last_error_code == FILEHELPER_ERROR_NO_FILES_FOUND;
        return NULL;
    }

    curSelFile++;
    if(curSelFile >= selFiles.size()){    
        LogMessage(INFO, "FilesystemHelper::FindNextFile()::FILEHELPER_END_FILELIST");    
        __last_error_code == FILEHELPER_END_FILELIST;
        return NULL;
    }
    auto path = selFiles[curSelFile];
    return OpenFile(path, "wb");
}

std::vector<std::string> FilesystemHelper::FindFiles(const std::string& name){
    selFiles.clear();
    std::string fixedPath = name;
    fixedPath = FixPath(fixedPath);
    const std::string dirPath = parseDirectory(fixedPath);
    std::string fileName = parseFilename(fixedPath);
    
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

        LogMessage3(ERROR, "FilesystemHelper::FindFiles:: dir = ",dirPath.c_str() , "; can't open directory");
        __last_error_code = FILEHELPER_ERROR_CANT_OPEN_DIR;
    }

    return selFiles;
}


int FilesystemHelper::WriteToFile(const std::string& path, const char* buff, const int buffsize){
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);
    FILE *ptr = OpenFile(fixedPath, "wb");
    int oldSize = 0;
    if(CheckLogLevel(DEBUG)){
        oldSize = GetFileSize(ptr);
    }
    int errCode = WriteToFile(ptr, buff, buffsize);
    //if(errCode == FILEHELPER_NO_ERROR){
        CloseFile(ptr);
    //}
    /*
    if(CheckLogLevel(DEBUG)){
        std::string msg = "FilesystemHelper::WriteToFile" + " buff = " + buff;
        msg += ", buffsize = " + std::to_string(buffsize) + ", path = " + path;
        LogMessage(DEBUG, msg.c_str());
    }//*/
    return __last_error_code = errCode;
}

int FilesystemHelper::WriteToFile(const std::string& path, const unsigned char* buff, const int buffsize){
    return WriteToFile(path, (const char*)buff, buffsize);
}


int FilesystemHelper::WriteToFile(FILE*& ptr, const void* buff, const int buffsize){
    int errCode = FILEHELPER_NO_ERROR;
    int writenBytes = buffsize;
    int oldSize = 0;
    if(CheckLogLevel(DEBUG)){
        oldSize = GetFileSize(ptr);
    }
    if(ptr == NULL){
        __last_error_code = errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{ 
        writenBytes *= fwrite(buff, buffsize, 1, ptr);
        if ( writenBytes <=0 ){
            LogMessage(ERROR,"FilesystemHelper::WriteToFile():: ERROR_WRITE_FAULT");
            __last_error_code = errCode = ERROR_WRITE_FAULT;
        }
    }
    if(CheckLogLevel(DEBUG)){
        std::string msg = "FilesystemHelper::WriteToFile(" + std::to_string((long int) ptr) + ") buff = " + "void";
        msg += ", buffsize = " + std::to_string(buffsize) + ", path = " + GetFileName(ptr);
        msg += ", file size = " + std::to_string(GetFileSize(ptr)) +", oldSize = " + std::to_string(oldSize);
        LogMessage(DEBUG, msg.c_str());
    }
    //CloseFile(ptr);
    //return __last_error_code = errCode;
    return writenBytes;
}

int FilesystemHelper::WriteToFile(FILE*& ptr, const char* buff, const int buffsize){
    int errCode = FILEHELPER_NO_ERROR;
    int writenBytes = buffsize;
    int oldSize = 0;
    if(CheckLogLevel(DEBUG)){
        oldSize = GetFileSize(ptr);
    }
    if(ptr == NULL){
        LogMessage(ERROR,"FilesystemHelper::WriteToFile():: FILEHELPER_FILE_PTR_IS_NULL");
        __last_error_code = errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{ 
        writenBytes *= fwrite(buff, buffsize, 1, ptr);
        if ( writenBytes <=0 ){
            LogMessage(ERROR,"FilesystemHelper::WriteToFile():: ERROR_WRITE_FAULT");
            __last_error_code = errCode = ERROR_WRITE_FAULT;
        }
    }
     if(CheckLogLevel(DEBUG)){
        std::string msg = "FilesystemHelper::WriteToFile(" + std::to_string((long int) ptr) + ") buff = " + buff;
        msg += ", buffsize = " + std::to_string(buffsize) + ", path = " + GetFileName(ptr);
        msg += ", file size = " + std::to_string(GetFileSize(ptr)) +", oldSize = " + std::to_string(oldSize);
        LogMessage(DEBUG, msg.c_str());
    }
    //CloseFile(ptr);
    //return __last_error_code = errCode;
    return writenBytes;
}


int FilesystemHelper::ReadFile(const std::string& path, char* buff, 
                                    const int buffSize, int& bytesRead, const std::string& mode ){
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);
    FILE *ptr = OpenFile(fixedPath, mode);

    int errcode = ReadFile(ptr, buff, buffSize, bytesRead);
    CloseFile(ptr);
    return __last_error_code = errcode;
}

int FilesystemHelper::ReadFile(FILE*& ptr, char* buff, const int buffSize, int& bytesRead){
    int errCode = FILEHELPER_NO_ERROR;
    if(!ptr){
        LogMessage(ERROR,"FilesystemHelper::ReadFile():: FILEHELPER_FILE_PTR_IS_NULL");
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{
        bytesRead = fread(buff, buffSize, 1, ptr);
        if(!bytesRead){
            LogMessage2(WARNING, "FilesystemHelper::ReadFile():: bytes not readed from ", intToA((long int)ptr));
            errCode = FILEHELPER_ERROR_FILE_NOT_READ;
        }else{
            LogMessage4(DEBUG, "FilesystemHelper::ReadFile():: readed ", intToA(bytesRead), "; data = ", buff);
        }
    }
    return __last_error_code = errCode;
}


int FilesystemHelper::ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead){
    int errCode = FILEHELPER_NO_ERROR;
    if(!ptr){
        LogMessage(ERROR,"FilesystemHelper::ReadFile():: FILEHELPER_FILE_PTR_IS_NULL");
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{
        bytesRead = fread(buff, buffSize, 1, ptr);
        if(!bytesRead){
            LogMessage2(WARNING, "FilesystemHelper::ReadFile():: bytes not readed from ", intToA((long int)ptr));
            errCode = FILEHELPER_ERROR_FILE_NOT_READ;
        }else{
            bytesRead = buffSize;
            LogMessage2(DEBUG, "FilesystemHelper::ReadFile():: readed ", intToA(bytesRead)/*, "; data = ", (char*)buff*/);
        }
    }
    return __last_error_code = errCode;
}

int FilesystemHelper::GetFileSize(const std::string& path){

    FILE* ptr = OpenFile(path, "rb");
    int size = -1;
    if(!ptr){
        __last_error_code = FILEHELPER_FILE_PTR_IS_NULL;
        LogMessage(ERROR, "FilesystemHelper::GetFileSize()::FILEHELPER_FILE_PTR_IS_NULL ");
    }else{
        size = GetFileSize(ptr);
        CloseFile(ptr);
        LogMessage4(DEBUG, "FilesystemHelper::GetFileSize(", path.c_str(), ") = ",  intToA(size));
    }
    return size;
}


int FilesystemHelper::GetFileSize(FILE*& ptr){
    if(!ptr){
        __last_error_code = FILEHELPER_FILE_PTR_IS_NULL;
        return -1;
    }
    int pos = ftell(ptr);
    fseek(ptr, 0L, SEEK_END);
    int size = ftell(ptr);
    fseek(ptr, 0L, pos);
    
    LogMessage4(DEBUG, "FilesystemHelper::GetFileSize(", intToA((long int)ptr), ") = ", intToA(size));
    return size;
}

int FilesystemHelper::GetLastError(){
    return __last_error_code;
}

int FilesystemHelper::ResetLastError(){
    LogMessage(INFO, "FilesystemHelper::ResetLastError()");
    __last_error_code = FILEHELPER_NO_ERROR;
    return 0;
}


std::string FilesystemHelper::GetOtmDir(){
    int maxPath = 255;
    char OTMdir[maxPath];
    int res = properties_get_str(KEY_OTM_DIR, OTMdir, maxPath);
    
    //property OTM_dir must be saved during property_init, if not setup- then there were not property_init_call
    if(!strlen(OTMdir) || res != PROPERTY_NO_ERRORS){
        LogMessage4(WARNING, "FilesystemHelper::GetOtmDir()::can't access OTM dir->try to init properties.\n res = ", intToA(res), ", OTMdir = ", OTMdir);
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
        LogMessage4(WARNING, "FilesystemHelper::GetHomeDir()::can't access Home dir->try to init properties.\n res = ", intToA(res), ", OTMdir = ", HOMEdir);
        properties_init();
        res = properties_get_str(KEY_HOME_DIR, HOMEdir, maxPath);
    }
    return HOMEdir;
}

int FilesystemHelper::CreateDir(const std::string& dir, int rights) {
    struct stat st;
    int ret = stat(dir.c_str(), &st);
    LogMessage6(DEBUG, "FilesystemHelper::CreateDir(", dir.c_str(),"; rights = ", intToA(rights),")::stat():: ret = ", intToA(ret));
    if (ret){
        ret = mkdir(dir.c_str(), rights);
        LogMessage6(INFO, "FilesystemHelper::CreateDir(", dir.c_str(),"; rights = ", intToA(rights),") was created, ret = ", intToA(ret));
    }
    return ret;
}

bool FilesystemHelper::DirExists(const std::string& path){

    bool bExists = false;
    DIR *pDir = opendir (path.c_str());

    if (pDir != NULL)
    {
        bExists = true;    
        (void) closedir (pDir);
        LogMessage3(DEBUG, "FilesystemHelper::DirExists(", path.c_str(),") exists");
    }else{
        LogMessage3(INFO, "FilesystemHelper::DirExists(", path.c_str(),") not exists");
    }

    return bExists;

}
