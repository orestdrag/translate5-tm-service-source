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
#include <map>

#include <errno.h>


#include <filesystem>

int __last_error_code = 0;


PFileBufferMap pFileBuffers = NULL;
PFileBufferMap getFileBufferInstance(){
    if(!pFileBuffers){
        LogMessage(INFO, "FILEBUFFERS not exist yet-> creating new instance");
        pFileBuffers = new FileBufferMap();
    }
    return pFileBuffers;
}

PFileBufferMap GetPFileBufferMap(){
    return pFileBuffers;
}

void SetFileBufferMap(const PFileBufferMap pbm){
    if(pFileBuffers){
        if(pFileBuffers->empty()){
            LogMessage(INFO,"SetFileBufferMap:: deleting old empty buffer and replacing with new");
            delete pFileBuffers;
        }else{
            LogMessage(ERROR,"SetFileBufferMap:: existing filebuffer not empty, so can't replace it");
        }
    }
    LogMessage(INFO, "SetFileBufferMap:: replaced filebuffer");
    pFileBuffers = pbm;
}


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
    const char* cpath = path.c_str();
    //LogMessage2(WARNING,"TEMPORARY HARDCODED useBuffer= true in FilesystemHelper::CreateFile, fName = ", cpath);
    bool useBuffer = false ;//true; //false;
    if(    (strcasestr(cpath, ".TMI") )
        || (strcasestr(cpath, ".TMD") )
        //|| (strcasestr(cpath, ".MEM") )
    ){
        LogMessage4(INFO, "filesystem_open_file::Openning data file(with ext. .TMI, .TMD, .MEM => forcing to use filebuffers, fName = ", 
                        cpath, ", useFilebuffer before = ", intToA(useBuffer));
        useBuffer = true;
    }
    return OpenFile(path, mode, useBuffer);
}

int FilesystemHelper::MoveFile(std::string oldPath, std::string newPath){
    std::string fixedOldPath = FixPath(oldPath);
    std::string fixedNewPath = FixPath(newPath);
    errno = 0;
    
   if(rename(fixedOldPath.c_str(), fixedNewPath.c_str()) == -1){
       LogMessage6(ERROR, "MoveFile:: cannot move ", fixedOldPath.c_str()," to ", fixedNewPath.c_str(), ", error = ", strerror(errno));
       return errno;
    }else{
        LogMessage4(INFO,"MoveFile:: file moved from ", fixedOldPath.c_str()," to ", fixedNewPath.c_str());
    }
    return 0;

}

FILE* FilesystemHelper::OpenFile(const std::string& path, const std::string& mode, bool useBuffer){
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);
    long fileSize = 0;
    FileBuffer* pFb = NULL;
    LogMessage6(INFO, "FilesystemHelper::OpenFile, path =", path.c_str(), ", mode = ", mode.c_str(), ", useBuffer = ", intToA(useBuffer));
    FILE *ptr = fopen(fixedPath.c_str(), mode.c_str());
    if(!ptr){
        LogMessage2(ERROR, "FilesystemHelper::OpenFile:: can't open file ", path.c_str());
    }else if(useBuffer){
        if(!FindFiles(fixedPath).empty()){
            fileSize = GetFileSize(ptr);//GetFileSize(fixedPath);
        }


        if(getFileBufferInstance()->find(fixedPath) != getFileBufferInstance()->end()){
            LogMessage2(WARNING, "OpenFile::Filebuffer wasn't created, it's already exists for ", fixedPath.c_str());
            pFb = &(*getFileBufferInstance())[fixedPath];
            if(pFb->file == 0){
                pFb->file = ptr;
                pFb->offset = 0;
            }else{
                LogMessage2(ERROR,"File buffer already have filepointer, ", fixedPath.c_str());
            }
        }else{
            pFb = &(*getFileBufferInstance())[fixedPath];
            //fileBuffers[fixedPath].resize(1048575);//F FFFF
            //fileBuffers[fixedPath].resize(65536);//0x1000
            //fileBuffers[fixedPath].resize(32768);
            if( fileSize > 0 ){
                pFb->data.resize(fileSize);
                LogMessage4(INFO, "OpenFile:: file size >0  -> Filebuffer resized to filesize(",intToA(fileSize),"), fname = ", fixedPath.c_str());
                int readed = fread(&pFb->data[0], fileSize, 1, ptr);
                LogMessage6(INFO, "OpenFile:: file size >0  -> Filebuffer reading to buffer, size = ",intToA(fileSize),"), fname = ", fixedPath.c_str(), "; readed = ", intToA(readed));
            }else{
                LogMessage2(INFO, "OpenFile:: file size <=0  -> Filebuffer resized to default value, fname = ", fixedPath.c_str());
                pFb->data.resize(16384);
            }
            LogMessage4(INFO, "OpenFile::Filebuffer created for file ", fixedPath.c_str(), " with size = ", intToA(pFb->data.size()));
            pFb->status = FileBufferStatus::OPEN;
        }

        pFb->file = ptr;
        pFb->offset = 0;
    }

    LogMessage8(DEBUG, "FilesystemHelper::OpenFile():: path = ", fixedPath.c_str(), "; mode = ", mode.c_str(), "; ptr = ", intToA((long int)ptr), "size = ", intToA(fileSize));

    if(ptr == NULL){
        __last_error_code = FILEHELPER_FILE_PTR_IS_NULL;
    }
    return ptr;
}

std::string FilesystemHelper::GetFileName(HFILE ptr){
    if(ptr){
        const int MAXSIZE = 0xFFF;
        char filename[MAXSIZE];
        char proclnk[MAXSIZE];

        int fno = fileno(ptr);
        sprintf(proclnk, "/proc/self/fd/%d", fno);
        int r = readlink(proclnk, filename, MAXSIZE-1);
        filename[r] = '\0';

        return filename;
    }else{
        LogMessage(ERROR,"FilesystemHelper::GetFileName:: file ptr is null");
        return "";
    }
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


int FilesystemHelper::WriteToBuffer(FILE *& ptr, const void* buff, const int buffSize, const int startingPosition){
    std::string fName = GetFileName(ptr);
    FileBuffer* pFb = NULL;
    int offset = startingPosition;

    if(getFileBufferInstance()->find(fName)!= getFileBufferInstance()->end()){
        pFb = &(*getFileBufferInstance())[fName];    

        pFb->status |= MODIFIED;
        if(offset + buffSize > pFb->data.size()){
            LogMessage6(DEBUG, "FilesystemHelper::WriteToBuffer::Resizing file ", fName.c_str()," from ", intToA(pFb->data.size())," to ", intToA(offset+buffSize));
            pFb->data.resize(offset + buffSize);
        }

        void* pStPos = &(pFb->data[offset]);
        memcpy(pStPos, buff, buffSize);
        pFb->offset += buffSize;
    }else{
        LogMessage2(ERROR, "FilesystemHelper::WriteToBuffer:: can't find buffer for file ", fName.c_str());
        return -1;
    }
    LogMessage4(DEBUG, "FilesystemHelper::WriteToBuffer:: success, ", intToA(buffSize)," bytes written to buffer for ", fName.c_str());
    return 0;
}

int FilesystemHelper::ReadBuffer(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPos){
    std::string fName = GetFileName(ptr);
    int offset = startingPos;
    FileBuffer* pFb = NULL;

    if(getFileBufferInstance()->find(fName) == getFileBufferInstance()->end()){
        LogMessage2(DEBUG,"ReadBuffer:: file not found in buffers, fName = ", fName.c_str());
        return FILEHELPER_WARNING_BUFFER_FOR_FILE_NOT_OPENED;
    }
    pFb = &(*getFileBufferInstance())[fName];
    if(startingPos < 0){
        offset = pFb->offset;
    }

    if(pFb->data.size()< offset + buffSize){
        LogMessage2(ERROR,"ReadBuffer:: Trying to read not existing bytes from buffer, fName = ", fName.c_str());
        return FILEHELPER_WARNING_FILE_IS_SMALLER_THAN_REQUESTED;
    }
    PUCHAR p = &(pFb->data[offset]);
    memcpy(buff, p, buffSize);
    bytesRead = buffSize;
    LogMessage6(INFO, "ReadBuffer::", intToA(buffSize)," bytes read from buffer to ", fName.c_str(), " starting from ", intToA(offset) );
    return 0;
}

int FilesystemHelper::FlushBufferIntoFile(const std::string& fName){ 
    if(getFileBufferInstance()->find(fName)!= getFileBufferInstance()->end()){
        WriteBuffToFile(fName);
        LogMessage2(ERROR, "TEMPORARY_COMMENTED::FilesystemHelper::FlushBufferIntoFile erasing of filebuffer ", fName.c_str());
        (*getFileBufferInstance())[fName].file = nullptr;
        (*getFileBufferInstance())[fName].offset = 0;
        (*getFileBufferInstance())[fName].status = CLOSED;
        //pFileBuffers->erase(fName);
    }else{
        LogMessage2(INFO,"FilesystemHelper::FlushBufferIntoFile:: filebuffer not found, fName = ", fName.c_str());
    }
    return 0;
}

int FilesystemHelper::WriteBuffToFile(std::string fName, bool tempFile){
    LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 142");
#ifdef TEMPORARY_COMMENTED
    if(tempFile) 
        return 0;
    #endif

    FileBuffer* pFb = NULL;
    if(getFileBufferInstance()->find(fName)!= getFileBufferInstance()->end()){
        pFb = &(*getFileBufferInstance())[fName];

        if(pFb->status & FileBufferStatus::MODIFIED){
            LogMessage(INFO, "WriteBuffToFile:: writing files from buffer");
            PUCHAR bufStart = &pFb->data[0];
            int size = pFb->data.size();
            
            if(tempFile)
                fName+="_buff";
            
            HFILE ptr = fopen(fName.c_str(),"w+b");
            WriteToFile(ptr, bufStart, size);
            fclose(ptr);
        }
    }else{
        LogMessage2(ERROR,"WriteBuffToFile:: buffer not found, fName = ", fName.c_str());
    }
    
    return 0;
}


int FilesystemHelper::FlushAllBuffers(){
    if(!pFileBuffers){
        return -1;
    }

    for(auto file : *pFileBuffers){
        WriteBuffToFile(file.first);
    }
    return 0;

}

int FilesystemHelper::CloseFile(FILE*& ptr){
    LogMessage(DEBUG, "called FilesystemHelper::CloseFile()");
    if(ptr){
        std::string fName = GetFileName(ptr);
        LogMessage4(DEBUG, "FilesystemHelper::CloseFile(", intToA((long int) ptr) , "), fName = ", fName.c_str() );
        fclose(ptr);
        FlushBufferIntoFile(fName);

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



//std::vector<std::string> selFiles;
int curSelFile = -1;
FILE* FilesystemHelper::FindFirstFile(const std::string& name){
    LogMessage2(FATAL, "called not implemented function FilesystemHelper::FindFirstFile(), fName = ", name.c_str());
    LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 143");
#ifdef TEMPORARY_COMMENTED
    auto files = FindFiles(name);
    if(selFiles.empty()){

        LogMessage3(INFO, "FilesystemHelper::FindFiles(",name.c_str() , ") - files not found");
        __last_error_code == FILEHELPER_ERROR_NO_FILES_FOUND;
        return NULL;
    }
    curSelFile = 0;
    auto path = selFiles[curSelFile];
    return OpenFile(path, "wb");
    #endif

    return NULL;
}

FILE* FilesystemHelper::FindNextFile(){
    LogMessage(FATAL, "called not implemented function FilesystemHelper::FindNextFile()");
    LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 144");
#ifdef TEMPORARY_COMMENTED
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
    #endif 

}

std::vector<std::string> FilesystemHelper::FindFiles(const std::string& name){
    std::vector<std::string> selFiles;
    //if(selFiles.size())
    //    selFiles.clear();
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

int FilesystemHelper::WriteToFile(FILE*& ptr, const void* buff, const long unsigned int buffSize, int &iBytesWritten, const int startingPosition){
    return WriteToFileBuff(ptr,buff,buffSize,iBytesWritten, startingPosition);
}


int FilesystemHelper::WriteToFileBuff(FILE*& ptr, const void* buff, const long unsigned int buffSize, int &iBytesWritten, const int startingPosition){
    std::string fName = GetFileName(ptr);

    if(getFileBufferInstance()->find(fName) != getFileBufferInstance()->end()){

        LogMessage6(INFO,"Writing ", intToA(buffSize), " bytes to filebuffer ", fName.c_str(),
            " starting from position ", intToA(startingPosition));
        WriteToBuffer(ptr, buff, buffSize, startingPosition);
        iBytesWritten = buffSize;
    }else{
        LogMessage4(INFO, "File is not opened in filebuffers-> writting to file, fName = ", fName.c_str(), ", fId = ", intToA((long)ptr));

        LogMessage6(INFO,"Writing ", intToA(buffSize), " bytes to file ", fName.c_str(),
            " starting from position ", intToA(startingPosition));

        #ifndef TEMPORARY_HARDCODED
        {
            std::string filesInBuf = "FilesystemHelper::WriteToFileBuff:: files in filebuffer:\n";
            for (auto it = getFileBufferInstance()->begin(); it != getFileBufferInstance()->end(); ++it)
            {
                filesInBuf += it->first + '\n';
            }
            LogMessage(INFO, filesInBuf.c_str());
        }

        #endif 
        long lPart = startingPosition, hPart = 0;
        int ret = 0;
        if(startingPosition >=0){
            ret = SetFileCursor(ptr, lPart, hPart, FILE_BEGIN);
        }
        iBytesWritten =  WriteToFile(ptr, buff, buffSize);
    }
    return 0;
}


int FilesystemHelper::SetOffsetInFilebuffer(FILE* ptr,int offset){
    FileBuffer* pFb = NULL;

    if(ptr){
        std::string fName = GetFileName(ptr);
        int fSize = GetFileSize(ptr);
        if(getFileBufferInstance()->find(fName) != getFileBufferInstance()->end()){

            pFb = &(*getFileBufferInstance())[fName]; 
            if(offset <= fSize){       
                LogMessage4(INFO, "FilesystemHelper::SetOffsetInFilebuffer -> changing offset from ", intToA(pFb->offset), " to ", intToA(offset));
                pFb->offset = offset;
            }else{
                LogMessage6(WARNING, "FilesystemHelper::SetOffsetInFilebuffer -> can't change offset from ", 
                        intToA(pFb->offset), " to ", intToA(offset), " because it's bigger than size:", intToA(fSize));
            }
        }else{
            LogMessage2(INFO, "FilesystemHelper::SetOffsetInFilebuffer -> filebuffer not found, fName = ", fName.c_str());
        }
    }else{
        LogMessage(ERROR, "FilesystemHelper::SetOffsetInFilebuffer -> file pointer is null");
    }
}


int FilesystemHelper::TruncateFileForBytes(HFILE ptr, int numOfBytes){
    LogMessage4(INFO, "Try to truncate file ", FilesystemHelper::GetFileName(ptr).c_str(), " for ", intToA(numOfBytes));
    off_t lDistance = numOfBytes;
    int retCode = ftruncate(ptr->_fileno, lDistance);
    return retCode;
}

short FilesystemHelper::SetFileCursor(HFILE fp,long LoPart,long& HiPart,short OffSet)
{
    DWORD ret = 0 ;

    LogMessage6(INFO, "SetFilePointer for file ", FilesystemHelper::GetFileName((HFILE)fp).c_str(), "; offset is ", intToA(LoPart), ", direction is ", intToA(OffSet));
    //loff_t res ; //It is also LONGLONG variable.
    LONGLONG res ; //It is also LONGLONG variable. LONG is long :)

    unsigned int whence = 0 ;
    
    if(OffSet == FILE_BEGIN)
        whence = SEEK_SET ;
    else if(OffSet == FILE_CURRENT)
        whence = SEEK_CUR ;
    else if(OffSet == FILE_END)
        whence = SEEK_END ;
    else
        LogMessage(FATAL, "SetFilePointerEx::WRONG dwMoveMethod");

    if(OffSet != FILE_BEGIN){           
        LogMessage(WARNING, "SetFilePointerEx::FILE_CURRENT/FILE_END not implemented/tested");
    }
    
    int size = FilesystemHelper::GetFileSize(fp);
    if(size < LoPart){
        LogMessage2(WARNING, "SetFilePointer::File is smaller than requested position -> writing, fname = ", 
            FilesystemHelper::GetFileName(fp).c_str());
        TruncateFileForBytes(fp, LoPart);
    }

    res = fseek(fp, LoPart, whence);   

    if(res >= 0){
        fp->_offset = LoPart;
        LARGE_INTEGER li ;
        li.QuadPart = res ; //It will move High & Low Order bits.
        ret = li.LowPart ;

        HiPart = li.HighPart ;

    }else{
        int k = errno ;
        switch(errno){
        case EBADF : 
            LogMessage2(ERROR, "fd is not an open file descriptor. fname = ", ""
            //FilesystemHelper::GetFileName(fp).c_str()
            ) ;
            break ;
        case EFAULT : 
            LogMessage2(ERROR,"Problem with copying results to user space, fname =  ", ""
            //FilesystemHelper::GetFileName(fp).c_str() 
            ) ;
            break ;
        case EINVAL : 
            LogMessage2(ERROR,"cw whence is invalid, fname = ", ""
            //FilesystemHelper::GetFileName(fp).c_str() 
            ) ;
            break ;
        default:
            LogMessage2(ERROR," default, fname = ", "" 
            //FilesystemHelper::GetFileName(fp).c_str() 
            );
        }

    }
    LogMessage2(INFO,"SetFilePointer success, file = ", FilesystemHelper::GetFileName(fp).c_str());
    return ret ;
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

/*
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
//*/


int FilesystemHelper::ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPos){
    int err = 0, size = 0;
    err = ReadBuffer(ptr, buff, buffSize, bytesRead, startingPos);
    if(err == FILEHELPER_WARNING_BUFFER_FOR_FILE_NOT_OPENED){
        LogMessage2(INFO, "File not found in buffers -> reading from disk, fName = ", GetFileName(ptr).c_str());

        if((size = GetFileSize(ptr)) < startingPos+buffSize){
            err = FILEHELPER_WARNING_FILE_IS_SMALLER_THAN_REQUESTED;
        }else{
            long lPart = startingPos, hPart = 0;
            err = SetFileCursor(ptr, lPart, hPart, FILE_BEGIN);
            err = ReadFile(ptr, buff, buffSize, bytesRead);
        }
    }
    return err;
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

/*
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
//*/


int FilesystemHelper::ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead){
    int errCode = FILEHELPER_NO_ERROR;
    if(!ptr){
        LogMessage(ERROR,"FilesystemHelper::ReadFile():: FILEHELPER_FILE_PTR_IS_NULL");
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{
        bytesRead = fread(buff, buffSize, 1, ptr);
        if(!bytesRead){
            LogMessage4(WARNING, "FilesystemHelper::ReadFile():: bytes not readed from ", intToA((long int)ptr), ", path = ", GetFileName(ptr).c_str());
            errCode = FILEHELPER_ERROR_FILE_NOT_READ;
        }else{
            bytesRead = buffSize;
            LogMessage2(DEBUG, "FilesystemHelper::ReadFile():: readed ", intToA(bytesRead)/*, "; data = ", (char*)buff*/);
        }
    }
    return __last_error_code = errCode;
}


int FilesystemHelper::ReadFileBuff(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPosition){
    int errCode = FILEHELPER_NO_ERROR;
    int offset = startingPosition;
    FileBuffer* pFb = NULL;
    if(!ptr){
        LogMessage(ERROR, "FilesystemHelper::ReadFile File pointer is null");
        return __last_error_code = errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }
    std::string fName = GetFileName(ptr);
    if(getFileBufferInstance()->find(fName) != getFileBufferInstance()->end()){
        pFb = &(*getFileBufferInstance())[fName];
        if(offset < 0){
            LogMessage4(DEBUG, "FilesystemHelper::ReadFileBuff::  offset[", intToA(offset),"] is less than 0 ->  using pFb->offset", intToA(pFb->offset));
            offset = pFb->offset; 
        }
        if(offset + buffSize <= pFb->data.size()){
            memcpy(buff, &(pFb->data[offset]), buffSize);
            bytesRead = buffSize;
            pFb->offset += bytesRead; 
            LogMessage6(DEBUG, "FilesystemHelper::ReadFileBuff:: success, fName = ", fName.c_str(), "; offset = ", intToA(offset), "; bytesRead = ", intToA(bytesRead));
        }else{ 
            LogMessage(ERROR, "FilesystemHelper::ReadFileBuff:: requested file position not exists");
            __last_error_code = errCode = FILEHELPER_ERROR_READING_OUT_OF_RANGE;
        }

    }else{
        LogMessage2(INFO, "FilesystemHelper::ReadFileBuff, file buff not found-> using reading directly from file, fName = ", fName.c_str());
        errCode = ReadFile(ptr, buff, buffSize, bytesRead);
    }
    return errCode;
}



#include <sys/stat.h>

long FnGetFileSize(std::string&& filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    int logLevel = rc ==0? INFO : ERROR;
    LogMessage6(logLevel, "FnGetFileSize:: for ", filename.c_str(), " is ", intToA(stat_buf.st_size)," , rc = ", intToA(rc) );
    return rc == 0 ? stat_buf.st_size : -1;
}

long FdGetFileSize(int fd)
{
    struct stat stat_buf;
    int rc = fstat(fd, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int FilesystemHelper::GetFileSize(const std::string& path){

    auto size = FnGetFileSize(path.c_str());

    LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 145");
#ifdef TEMPORARY_COMMENTED
    int size = -1;
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);

    FILE* ptr = fopen(fixedPath.c_str(), "rb");
    
    if(!ptr){
        LogMessage2(ERROR, "FilesystemHelper::GetFileSize,can't open file ", path.c_str());
    }else{
        size = GetFileSize(fixedPath);
        fclose(ptr);
    }
    return size;
    LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 146");
#ifdef TEMPORARY_COMMENTED
    if(pFileBuffers->find(path) != pFileBuffers->end() && fileBuffers[path].data.size()>0){
        size = fileBuffers[path].data.size();
        LogMessage3(DEBUG, "FilesystemHelper::GetFileSize(", path.c_str(), ") used filebuffer ");
    }else{
        FILE* ptr = OpenFile(path, "rb", false);
        LogMessage3(DEBUG, "FilesystemHelper::GetFileSize(", path.c_str(), ") filebuffer not found->reading file");
        if(!ptr){
            __last_error_code = FILEHELPER_FILE_PTR_IS_NULL;
            LogMessage(ERROR, "FilesystemHelper::GetFileSize()::FILEHELPER_FILE_PTR_IS_NULL ");
        }else{
            size = GetFileSize(ptr);
            CloseFile(ptr);            
        }
    }
    LogMessage4(DEBUG, "FilesystemHelper::GetFileSize(", path.c_str(), ") = ",  intToA(size));
    #endif
    #endif
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
    
    std::string fName = GetFileName(ptr);
    LogMessage6(DEBUG, "FilesystemHelper::GetFileSize(", intToA((long int)ptr), ") = ", intToA(size),", fName = ",fName.c_str());
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
    const int maxPath = 255;
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
