#include "FilesystemHelper.h"
#include "FilesystemWrapper.h"
#include "Property.h"
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
#include <filesystem>
#include <errno.h>


#include <filesystem>
#include <folly/portability/GFlags.h>
#include "EncodingHelper.h"
//#define LOG_FILE_WRITE 1

int __last_error_code = 0;

std::string FilesystemHelper::_memDir;
std::string FilesystemHelper::_tableDir;
std::string FilesystemHelper::_homeDir;
std::string FilesystemHelper::_otmDir;
std::string FilesystemHelper::_tempDir;

PFileBufferMap FilesystemHelper::getFileBufferInstance(){
    static FileBufferMap map;
    return &map;
}

std::mutex FilesystemHelper::fsLock;
std::recursive_mutex FilesystemHelper::loadingTm;


#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <filesystem>

bool atomicSave(const std::string& filename, const std::string& data) {
    try {
        // Step 1: Create a temporary file in the same directory
        std::string tempFilename = filename + ".tmp";
        
        // Step 2: Write data to the temporary file
        {
            std::ofstream tempFile(tempFilename, std::ios::out | std::ios::trunc | std::ios::binary);
            if (!tempFile) {
                std::cerr << "Error: Unable to open temporary file for writing\n";
                return false;
            }

            tempFile.write(data.c_str(), data.size());
            if (!tempFile) {
                std::cerr << "Error: Write operation failed\n";
                return false;
            }

            // Flush the buffer to ensure data is written to the disk
            tempFile.flush();
            if (!tempFile) {
                std::cerr << "Error: Flush operation failed\n";
                return false;
            }
        }

        // Step 3: Ensure data is physically written to disk
        if (std::filesystem::exists(tempFilename)) {
            std::filesystem::permissions(tempFilename, std::filesystem::perms::owner_read | 
                                                      std::filesystem::perms::owner_write);
        }
        // Step 4: Rename the temporary file to the target file (atomic on most filesystems)
        std::filesystem::rename(tempFilename, filename);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << "\n";
        return false;
    }
}
////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <cstdio>

int FileBuffer::atomicWriteWithBackup() {
    try {
        // Step 1: Create a temporary file
        std::string tempFilename = fileName + ".tmp";
        {
            std::ofstream tempFile(tempFilename, std::ios::out | std::ios::trunc | std::ios::binary);
            if (!tempFile) {
                T5LOG(T5ERROR) << "Error: Unable to open temporary file for writing";
                return 1;
            }
            //tempFile.write(data, data.size());
            if (!tempFile) {
                T5LOG(T5ERROR) << "Error: Write operation to temporary file failed";
                return 2;
            }

            tempFile.write(reinterpret_cast<const char*>(data.data()), data.size());
            tempFile.flush();
        }

        // Step 2: Rename original file to backup (if it exists)
        std::string backupFilename = fileName + ".old";
        if (std::filesystem::exists(fileName)) {
            std::filesystem::rename(fileName, backupFilename);
        }

        // Step 3: Rename the temporary file to the original filename
        std::filesystem::rename(tempFilename, fileName);

        // Step 4: Delete the backup file
        if (std::filesystem::exists(backupFilename)) {
            FilesystemHelper::DeleteFile(backupFilename, true);
            //std::filesystem::remove(backupFilename);
        }

        return 0;
    } catch (const std::exception& e) {
        T5LOG(T5ERROR) << "Error: " << e.what() << "";
        return 3;
    }
}

bool handleMissingTMI(const std::string& tmdFilename, const std::string& tmiFilename) {
    if (!std::filesystem::exists(tmiFilename)) {
        std::cout << "Recreating missing TMI file from TMD: " << tmdFilename << "\n";
        // Logic to recreate TMI from TMD should go here.
        // Placeholder: just creating an empty TMI file.
        std::ofstream tmiFile(tmiFilename, std::ios::out | std::ios::trunc | std::ios::binary);
        if (!tmiFile) {
            std::cerr << "Error: Unable to recreate TMI file\n";
            return false;
        }
        tmiFile << "Placeholder for regenerated TMI data\n";
        tmiFile.flush();
        return true;
    }
    return true;
}

/////////////////////////////////////

std::string FilesystemHelper::parseDirectory(const std::string path){
    std::size_t found = path.rfind('/');
    if (found!=std::string::npos)
        return path.substr(0,found);
    return path;
}
std::string FilesystemHelper::parseFilename(const std::string path){
    std::size_t found = path.rfind('/');

    if (found!=std::string::npos)
        return path.substr(found+1);
    return path;
}

std::string FilesystemHelper::RemovePathAndExtention(const std::string path){
    std::string filename = parseFilename(path);
    std::size_t found = filename.rfind('.');
    if (found!=std::string::npos)
        return filename.substr(0,found);
    return filename;
}


/*! \brief read a binary file into a Byte vector
\param pszFile fully qualified name of file being read
\param vFileData vector receiving the file data
it is up to the caller to free this area using free()
\param strError string receiving any error message text
\returns 0 is sucessfull or a return code
*/
int FilesystemHelper::LoadFileIntoByteVector(const std::string& strFile, std::vector<unsigned char>  &vFileData )
{
  int iRC = 0;

  // open file
  HFILE hFile =fopen(strFile.c_str(), "r") ;//CreateFile( pszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile == NULL )
  {
    iRC = GetLastError();
    T5LOG(T5ERROR) <<"   Error: open of file "<< strFile << " failed with rc=" << iRC << "";
    return( iRC );
  }

  // get the size of file
  DWORD dwFileSize = FilesystemHelper::GetFileSize(hFile);//GetFileSize( hFile, &dwFileSizeHigh );

  // adjust size of Byte vector
  vFileData.resize( dwFileSize );

  // read file into byte vector
  if ( !fread(  &vFileData[0],dwFileSize,1, hFile ) )
  {
    fclose( hFile );
    hFile = nullptr;
    iRC = GetLastError();
    T5LOG(T5ERROR) << "   Error: reading of "<< dwFileSize << " bytes from file "<< strFile <<" failed with rc=" << iRC << "";
    return( iRC );
  }

  // close file
  fclose( hFile );
  hFile = nullptr;

  return( iRC );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     checkFileName           Check syntax of long names       |
//+----------------------------------------------------------------------------+
//|Function call:     checkFileName   ( const std::string& filename  )         |
//+----------------------------------------------------------------------------+
//|Description:       Checks the length and the characters of the given long   |
//|                   name (invalid characters are backslash, question mark and|
//|                   asterisk).                                               |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszName    pointer to name being checked        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   name is valid                                     |
//|                   FALSE  name is invalid                                   |
//+----------------------------------------------------------------------------+
bool FilesystemHelper::checkFileName( const std::string& filename )
{
   BOOL    fOK  = TRUE;                // function return code
   PPROPSYSTEM pPropSys= NULL;

   // check length of name
   if ( filename.empty() || filename.size() >= MAX_LONGFILESPEC )
   {
     fOK = FALSE;                    // name is too long
   } /* endif */

   // check for invalid characters in the given name
   if ( fOK )
   {
     pPropSys = GetSystemPropPtr();
     char* pszName = (PSZ) &filename[0];
     while ( pPropSys && fOK && (*pszName != EOS) )
     {     
       if ( strchr("\\?*<>:|/", *pszName ) ) 
       {
         fOK = FALSE;                  // invalid character in name
       }
       else
       {
         pszName++;                    // continue with next character
       } /* endif */
     } /* endwhile */
   } /* endif */

   return ( fOK );
} /* end of FilesystemHelper::checkFilename */

std::string FilesystemHelper::FixPath(std::string& path){
    std::string ret;
    //
    if(path.find('/') == std::string::npos){
        char folderPath[255];
        Properties::GetInstance()->get_value(KEY_OTM_DIR, folderPath,255);
        Properties::GetInstance()->get_value_or_default(KEY_DEFAULT_DIR, folderPath, 255, folderPath);
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
    //T5LOG( T5WARNING) <<"TEMPORARY HARDCODED useBuffer= true in FilesystemHelper::CreateFile, fName = ", cpath);
    bool useBuffer = false ;//true; //false;
    if(    (strcasestr(cpath, ".TMI") )
        || (strcasestr(cpath, ".TMD") )
        //|| (strcasestr(cpath, ".MEM") )
    ){
        if(VLOG_IS_ON(1)){
            T5LOG( T5INFO) << "filesystem_open_file::Openning data file(with ext. .TMI, .TMD, .MEM => forcing to use filebuffers, fName = " << 
                        cpath << ", useFilebuffer before = " << useBuffer;
        }
        useBuffer = true;
    }
    return OpenFile(path, mode, useBuffer);
}

int FilesystemHelper::CloneFile(std::string srcPath, std::string dstPath){
    std::ifstream ifs(srcPath, std::ios::in | std::ios::binary);   
    std::ofstream ofs(dstPath, std::ios::out | std::ios::binary);
    if(!ofs)
    {
        T5LOG(T5ERROR) <<  "::can't open src file \'" <<  srcPath << "\' ";
        return FILEHELPER_ERROR_CANT_OPEN_DIR;
    } 
    if(!ifs)
    {
        T5LOG(T5ERROR) <<  "::can't open trg file \'" << dstPath << "\'  to move " << srcPath;
        return FILEHELPER_ERROR_CANT_OPEN_DIR;;
    } 
    ofs << ifs.rdbuf();
    return 0;
}

int FilesystemHelper::RenameFile(std::string oldPath, std::string newPath){
    std::lock_guard<std::mutex> l{fsLock};// lock fs
    std::string fixedOldPath = FixPath(oldPath);
    std::string fixedNewPath = FixPath(newPath);
    errno = 0;
    bool fOk = true;
    //Copy file instead
    if(FileExists(newPath.c_str())){
        T5LOG(T5ERROR) <<  ":: trg file \'" << newPath << "\' already exists!";
        return FILE_EXISTS;
    }
    if(FileExists(oldPath.c_str()) == false){
        T5LOG(T5ERROR) << ":: src file \'" << oldPath << "\' is missing!";
        return FILE_NOT_EXISTS;
    }
    // Attempt to rename the file
    if (std::rename(oldPath.c_str(), newPath.c_str()) == 0) {
        T5LOG(T5DEBUG) << "File \'"<<oldPath <<"\' renamed successfully to \'" << newPath<< "\'";
    }    
    
    if(!fOk){
       T5LOG(T5ERROR) << "cannot rename "<< fixedOldPath << " to " << fixedNewPath <<  ", error = " <<  strerror(errno);
       return errno;
    }else{
        if(VLOG_IS_ON(1)){
            T5LOG( T5INFO) << "file renamed from " << fixedOldPath << " to " <<  fixedNewPath ;
        }
    }
    return 0;
}

int FilesystemHelper::MoveFile(std::string oldPath, std::string newPath){
    std::lock_guard<std::mutex> l{fsLock};// lock fs
    std::string fixedOldPath = FixPath(oldPath);
    std::string fixedNewPath = FixPath(newPath);
    errno = 0;
    bool fOk = true;
    //fOk = rename(fixedOldPath.c_str(), fixedNewPath.c_str()) != -1;
    //Copy file instead
    if(FileExists(newPath.c_str())){
        T5LOG(T5ERROR) <<  ":: trg file \'" << newPath << "\' already exists!";
        return FILE_EXISTS;
    }
    if(FileExists(oldPath.c_str()) == false){
        T5LOG(T5ERROR) << ":: src file \'" << oldPath << "\' is missing!";
        return FILE_NOT_EXISTS;
    }
    CloneFile(oldPath, newPath);
    //delete prev file
    DeleteFile(oldPath);
    //remove(oldPath);
    
    if(!fOk){
       T5LOG(T5ERROR) << "MoveFile:: cannot move "<< fixedOldPath << " to " << fixedNewPath <<  ", error = " <<  strerror(errno);
       return errno;
    }else{
        if(VLOG_IS_ON(1)){
            T5LOG( T5INFO) << "MoveFile:: file moved from " << fixedOldPath << " to " <<  fixedNewPath ;
        }
    }
    return 0;
}

FILE* FilesystemHelper::OpenFile(const std::string& path, const std::string& mode, bool useBuffer){
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);
    long fileSize = 0;
    FileBuffer* pFb = NULL;
    if(VLOG_IS_ON(1)){
        T5LOG( T5DEBUG) << "FilesystemHelper::OpenFile, path ="<< path<< ", mode = "<< mode<< ", useBuffer = "<<useBuffer;
    }
    FILE *ptr = fopen(fixedPath.c_str(), mode.c_str());
    if(!ptr){
        T5LOG(T5ERROR) << "FilesystemHelper::OpenFile:: can't open file "<<path;
    }else if(useBuffer){
        if(!FindFiles(fixedPath).empty()){
            fileSize = GetFileSize(ptr);//GetFileSize(fixedPath);
        }

        if(getFileBufferInstance()->find(fixedPath) != getFileBufferInstance()->end()){
            if(VLOG_IS_ON(1)){
                T5LOG( T5WARNING) << "OpenFile::Filebuffer wasn't created, it's already exists for " << fixedPath ;
            }
            pFb = &(*getFileBufferInstance())[fixedPath];
            pFb->offset = 0;
        }else{
            pFb = &(*getFileBufferInstance())[fixedPath];
            if( fileSize > 0 ){
                pFb->data.resize(fileSize);
                //memset(&pFb->data[0], 0, pFb->data.size());
                
                if(VLOG_IS_ON(1)){
                    T5LOG( T5INFO) << "OpenFile:: file size >0  -> Filebuffer resized to filesize(" << fileSize << "), fname = " << fixedPath;
                }
                int readed = fread(&pFb->data[0], fileSize, 1, ptr);
                if(VLOG_IS_ON(1)){
                    T5LOG( T5INFO) << "OpenFile:: file size >0  -> Filebuffer reading to buffer, size = " << fileSize <<
                            "), fname = " << fixedPath << "; readed = " << readed;
                }
            }else{
                if(VLOG_IS_ON(1)){
                    T5LOG( T5INFO) << "OpenFile:: file size <=0  -> Filebuffer resized to default value, fname = " << fixedPath ;
                }
                pFb->data.resize(16384);
                //memset(&pFb->data[0], 0, pFb->data.size());
            }
            if(VLOG_IS_ON(1)){
                T5LOG( T5INFO) << "OpenFile::Filebuffer created for file " << fixedPath << " with size = " << pFb->data.size();
            }
            //pFb->status = FileBufferStatus::OPEN;
        }

        pFb->offset = 0;
    }

    if(VLOG_IS_ON(1)){
        T5LOG( T5DEBUG) << "FilesystemHelper::OpenFile():: path = " << fixedPath << "; mode = " << mode << "; ptr = " << (long int)ptr <<
             "size = " << fileSize;
    }
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
        if(r == -1){
            T5LOG(T5ERROR) << "can't process file \'"<<proclnk <<"\'"; 
            return "";
        }
        filename[r] = '\0';

        return filename;
    }else{
        T5LOG(T5ERROR) <<"FilesystemHelper::GetFileName:: file ptr is null";
        return "";
    }
}

DECLARE_bool(forbiddeletefiles);
int FilesystemHelper::DeleteFile(const std::string& path, bool suppressLogs){
    //std::lock_guard<std::mutex> l{FilesystemHelper::fsLock};// lock fs
    if(FLAGS_forbiddeletefiles){
        T5LOG( T5WARNING) << ":: file deletion is forbidden, service tried to delete this file: "<<path;
        return FILEHELPER_NO_ERROR;
    }
    std::string fixedPath = path;

    fixedPath = "\'" + FixPath(fixedPath) + "\'";

    if(int errCode = remove(path.c_str())){
        if(suppressLogs){
            T5LOG(T5DEBUG) << "FilesystemHelper::DeleteFile("<<fixedPath<< ") ERROR res = "<<errCode;
        }
        else{
            T5LOG(T5ERROR) << "FilesystemHelper::DeleteFile("<<fixedPath<< ") ERROR res = "<<errCode;
        }
        return errCode;
    }else{
        if(VLOG_IS_ON(1)){
            T5LOG( T5DEBUG) << "FilesystemHelper::DeleteFile("<<fixedPath<< ") res = FILEHELPER_NO_ERROR";
        }
        return FILEHELPER_NO_ERROR;
    }
}


bool FilesystemHelper::FileExists(const std::string& path){
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);
    struct stat buffer;   
    bool exists = (stat (fixedPath.c_str(), &buffer) == 0); 
    if(VLOG_IS_ON(1)){
        T5LOG( T5INFO) << ":: file \'" << fixedPath << "\' returned " << exists;
    }
    return exists;
}

bool FilesystemHelper::FilebufferExists(const std::string& path){
    auto pfb = getFileBufferInstance();
    bool res = pfb->find(path) != pfb->end();
    return res;
}

int FilesystemHelper::ReadFileToFileBufferAndKeepInRam(const std::string& path){
   if(FilebufferExists(path)){
        return FILEHELPER_WARNING_FILEBUFFER_EXISTS;
   }
    #ifdef  LOG_FILE_WRITE
     T5LOG(T5TRANSACTION) <<"called ReadFileToFileBufferAndKeepInRam " << path <<" from disk;";// << " , stacktrace:" << GET_STACKTRACE_EXPL;
    #endif
   auto file = OpenFile(path, "r", true);
   if(file == nullptr){
     return FILEHELPER_ERROR_NO_FILES_FOUND;
   }
   fclose(file);
   
   file = nullptr;
   return FILEHELPER_NO_ERROR;
}

int FilesystemHelper::RemoveDirWithFiles(const std::string& path){
    std::lock_guard<std::mutex> l{FilesystemHelper::fsLock};// lock fs
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);
    //if(int errCode = remove(path.c_str())){
    std::string command = "rm -r ";
    command += '\'' + fixedPath + '\'';
    
    if(int errCode = system(command.c_str()) ){
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) && VLOG_IS_ON(1)){
            T5LOG(T5ERROR) << "FilesystemHelper::RemoveDirWithFiles(" <<fixedPath << ") ERROR res = " << errCode;
        }
        return errCode;
    }else{
        if(VLOG_IS_ON(1)){
            T5LOG( T5DEBUG) << "FilesystemHelper::RemoveDirWithFiles(" << fixedPath << ") res = FILEHELPER_NO_ERROR";
        }
        return FILEHELPER_NO_ERROR;
    }
}

int FileBuffer::ReadFromFile(){        
    #ifdef  LOG_FILE_WRITE
     T5LOG(T5TRANSACTION) <<"called read file " << fileName <<" from disk;";// << " , stacktrace:" << GET_STACKTRACE_EXPL;
    #endif

    if(fileName.empty()){
        return FilesystemHelper::FILEHELPER_ERROR_WRONG_FILENAME;
    }
    if(FilesystemHelper::FileExists(fileName) == false){
        return FilesystemHelper::FILEHELPER_ERROR_NO_FILES_FOUND;
    }
    originalFileSize = FilesystemHelper::GetFileSize(fileName);
    int readed = 0;
    if( originalFileSize > 0  && originalFileSize != -1){
        data.resize(originalFileSize);
        
        if(VLOG_IS_ON(1)){
            T5LOG( T5INFO) << "OpenFile:: file size >0  -> Filebuffer resized to filesize(" << originalFileSize << "), fname = " << fileName;
        }
        file = fopen(fileName.c_str(), "rb");
        if(file == nullptr){
            T5LOG(T5ERROR) << "Can't open the file under the path=" << fileName;
        }else
        {
            readed = fread(&data[0], originalFileSize, 1, file);
            fclose(file);
            file = nullptr;
        } 
        if(VLOG_IS_ON(1)){
            T5LOG( T5INFO) << "OpenFile:: file size >0  -> Filebuffer reading to buffer, size = " << originalFileSize <<
                    "), fname = " << fileName << "; readed = " << readed;
        }
    }else{
        if(VLOG_IS_ON(1)){
            T5LOG( T5INFO) << "OpenFile:: file size <=0  -> Filebuffer resized to default value, fname = " << fileName ;
        }
        data.resize(16384); 
    }
    return 0;
}

int FileBuffer::SetOffset(size_t newOffset, int fileAnchor){
    if(fileAnchor != FILE_BEGIN){
        T5LOG(T5FATAL) <<"fileAnchor != FILE_BEGIN, is not implemented!";
        throw;
    }
    if(newOffset > data.size()){
        T5LOG(T5ERROR) << "tried to set offset to " << newOffset <<" , but its bigger than data size =" <<data.size();
        return -1;
    }
    if(newOffset < 0 ){
        T5LOG(T5ERROR) << "newOffset is negative "<<newOffset;
        return -2;
    }
    offset = newOffset;
    return 0;
}

bool FileBuffer::wasModified() const{
    return status & FileBufferStatus::MODIFIED;
}

int FileBuffer::Flush(bool forced){
    #ifdef  LOG_FILE_WRITE
     T5LOG(T5TRANSACTION) <<"called flush  filebuffer " << fileName <<" to disk;";// << " , stacktrace:" << GET_STACKTRACE_EXPL;
    #endif
    
    bool fileWasOpened = file != nullptr;
    int rc = 0;

    size_t writenBytes = 0;
    if(!FilesystemHelper::FileExists(fileName) && !forced){
        T5LOG(T5WARNING) << "File is not exists on disk so flushing would be skipped, fName = " << fileName;
    }else if(forced || wasModified()){
        if(VLOG_IS_ON(1)){
            T5LOG( T5INFO) << "Flush:: writing files from buffer";
        }  

        int oldSize = 0;
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){ 
            oldSize = FilesystemHelper::GetFileSize(fileName);
        }

        #ifdef TO_BE_REMOVED
        PUCHAR bufStart = &data[0];
        int size = data.size();
        //if(fileWasOpened && file) fclose(file);
        if(!fileWasOpened) file = fopen(fileName.c_str(),"w+b");
       
        if(file==nullptr){
            T5LOG(T5ERROR) <<"Cant open the file " << fileName;
            return FilesystemHelper::FILEHELPER_FILE_PTR_IS_NULL;
        }

        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
            oldSize = FilesystemHelper::GetFileSize(file);
        }
        
        fseek(file, 0, SEEK_SET);
        //if(!fileWasOpened) 
        {
            fclose(file);
            file = nullptr;
        }
        #endif
        //writenBytes = fwrite(bufStart, size, 1, file) * size;
        rc = atomicWriteWithBackup();
        if ( rc ){
            T5LOG(T5ERROR) <<"::Flush():: ERROR_WRITE_FAULT";
            return ERROR_WRITE_FAULT;
        }
    
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
            T5LOG( T5DEBUG) "::Flush("  << (long int) file << ") buff = " << "void" <<
                ", buffsize = " << data.size() << ", path = " << fileName << ", file size = " <<
                FilesystemHelper::GetFileSize(file) << ", oldSize = " << oldSize;
        }

        
    }else{
        if(VLOG_IS_ON(1)){
            T5LOG( T5INFO) <<"Flush:: buffer not modified, so no need to overwrite file, fName = " << fileName;
        }
    }
    return 0;
}

int FileBuffer::Read(void* buff, size_t buffSize){
    if(data.size()< offset + buffSize){
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)){
            T5LOG(T5ERROR) << "ReadBuffer:: Trying to read not existing bytes from buffer, fName = " << fileName;
        }
        return FilesystemHelper::FILEHELPER_WARNING_FILE_IS_SMALLER_THAN_REQUESTED;
    }    
    if(VLOG_IS_ON(1) && T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)){
        T5LOG( T5DEVELOP) << ":: fName = " << fileName << "; buff size = " << buffSize << "; data.size = " << data.size() << "; offset = " << offset << ";";
    }
    
    memcpy(buff, &(data[offset]), buffSize);
    offset+=buffSize;
    return 0;
}

int FileBuffer::Read(void* buff, size_t buffSize, size_t startingPos){
    offset = startingPos;
    return Read(buff, buffSize);
}



bool FileBuffer::isTMDFilebuffer(){
    return EncodingHelper::endsWithIgnoreCase(fileName, ".tmd" );
}

DECLARE_int64(allowedtmdsize);

int FileBuffer::Write(const void* buff, size_t buffSize, size_t startingPosition){
    status |= MODIFIED;
    if(startingPosition + buffSize > data.size()){
        if(VLOG_IS_ON(1)){
            T5LOG( T5DEBUG) << "::Resizing filebuffer "<<fileName<<" from "<<data.size()<<
                " to " <<offset+buffSize;
        }
        
        if(isTMDFilebuffer()){
            size_t allowedSize = (size_t)FLAGS_allowedtmdsize*1000000;
            if(offset + buffSize > allowedSize){
                T5LOG(T5INFO) << "filebuffer vector for " << fileName
                    << " resizing in unsuccessfull, because it's bigger than allowed size " << allowedSize <<" bytes" ;
                //return FilesystemHelper::FILEHELPER_ERROR_DATA_SIZE_BIGGER_THAN_ALLOWED_FILEBUFFER_SIZE;
                return TMD_SIZE_IS_BIGGER_THAN_ALLOWED;
            }
        }
        
        data.resize(offset + buffSize);
        if( offset + buffSize > data.size() ){
            T5LOG(T5ERROR) << "filebuffer vector resizing in unsuccessfull for " << fileName ;
            return FilesystemHelper::FILEHELPER_ERROR_DATA_SIZE_BIGGER_THAN_RECORD_SIZE;
        }
    }

    void* pStPos = &(data[startingPosition]);
    memcpy(pStPos, buff, buffSize);
    offset = startingPosition + buffSize;
    return 0 ;
}


int FilesystemHelper::WriteToBuffer(FILE *& ptr, const void* buff, const int buffSize, const int startingPosition){
    std::string fName = GetFileName(ptr);

    if(getFileBufferInstance()->find(fName)!= getFileBufferInstance()->end()){
        FileBuffer* pFb = &(*getFileBufferInstance())[fName];    

        pFb->Write(buff, buffSize, startingPosition);
    }else{
        T5LOG(T5ERROR) << "FilesystemHelper::WriteToBuffer:: can't find buffer for file " << fName;
        return FilesystemHelper::FILEHELPER_WARNING_BUFFER_FOR_FILE_NOT_OPENED;
    }
    if(VLOG_IS_ON(1)){
        T5LOG( T5DEBUG) << "FilesystemHelper::WriteToBuffer:: success, " << buffSize <<" bytes written to buffer for " << fName;
    }
    return 0;
}

int FilesystemHelper::ReadBuffer(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPos){
    std::string fName = GetFileName(ptr);
    #ifdef  LOG_FILE_WRITE
    // T5LOG(T5TRANSACTION) <<"called read file " << fName <<" from disk;";// << " , stacktrace:" << GET_STACKTRACE_EXPL;
    #endif
    int offset = startingPos;
    FileBuffer* pFb = NULL;

    if(getFileBufferInstance()->find(fName) == getFileBufferInstance()->end()){
        if(VLOG_IS_ON(1)){
            T5LOG( T5DEVELOP) << "ReadBuffer:: file not found in buffers, fName = " << fName;
        }
        return FILEHELPER_WARNING_BUFFER_FOR_FILE_NOT_OPENED;
    }
    pFb = &(*getFileBufferInstance())[fName];
    pFb->Read(buff, buffSize, startingPos);
    bytesRead = buffSize;
    if(VLOG_IS_ON(1)){
        T5LOG( T5DEBUG) <<  "ReadBuffer::" << buffSize <<" bytes read from buffer to " << fName << " starting from " << offset;
    }
    return 0;
}

int FilesystemHelper::FlushBufferIntoFile(const std::string& fName){ 
    auto pFBs = getFileBufferInstance();
    if(pFBs->find(fName)!= pFBs->end()){
        WriteBuffToFile(fName);
        (*pFBs)[fName].status &= ~MODIFIED;// reset modified flag
    }else{
        if(VLOG_IS_ON(1)){
            T5LOG( T5INFO) <<"FilesystemHelper::FlushBufferIntoFile:: filebuffer not found, fName = " << fName;
        }
    }
    return 0;
}

int FilesystemHelper::WriteBuffToFile(std::string fName){
    //#ifdef  LOG_FILE_WRITE
    // T5LOG(T5TRANSACTION) <<"called save file " << fName <<" to disk;";// << " , stacktrace:" << GET_STACKTRACE_EXPL;
    //#endif
    FileBuffer* pFb = NULL;
    auto pFBs = getFileBufferInstance();
    if(pFBs->find(fName)!= pFBs->end()){
        pFb = &(*pFBs)[fName];
        return pFb->Flush();
        
    }else{
        T5LOG(T5ERROR) <<"WriteBuffToFile:: buffer not found, fName = " << fName;
    }    
    return 0;
}


std::vector<UCHAR>* FilesystemHelper::GetFilebufferData(std::string name){
    auto pfb = getFileBufferInstance();
    name = FixPath(name);
    if(pfb->find(name) == pfb->end())
        return nullptr;

    return &((*pfb)[name].data);    
}

int FilesystemHelper::CreateFilebuffer(std::string name){
    auto pfb = getFileBufferInstance();
    if(pfb->find(name) != pfb->end())
        return FILEHELPER_WARNING_FILEBUFFER_EXISTS;
    (*pfb)[name].offset=0;
    
    return FILEHELPER_NO_ERROR;
}

int FilesystemHelper::FlushAllBuffers(std::string * modifiedFiles){
    auto pFileBuffers = getFileBufferInstance();
    if(!pFileBuffers){
        return -1;
    }

    for(auto file : *pFileBuffers){
        if(WriteBuffToFile(file.first) == FILEHELPER_NO_ERROR){
            if(modifiedFiles){
                *modifiedFiles += file.first + "; ";
            }
        }
    }
    return 0;

}

int FilesystemHelper::CloseFile(FILE*& ptr){
    if(VLOG_IS_ON(1)){
        T5LOG( T5DEBUG) << "called FilesystemHelper::CloseFile()";
    }
    if(ptr){
        std::string fName = GetFileName(ptr);
        if(VLOG_IS_ON(1)){
            T5LOG( T5DEBUG) << "FilesystemHelper::CloseFile(" << (long int) ptr << "), fName = " << fName;
        }
        fclose(ptr);
        ptr = nullptr;
        FlushBufferIntoFile(fName);
        CloseFileBuffer(fName);
    }
    ptr = NULL;
    return __last_error_code = FILEHELPER_NO_ERROR;
}

int FilesystemHelper::CloseFileBuffer(const std::string& path){
    auto fbs = getFileBufferInstance();
    if(fbs->find(path) == fbs->end()){
        return FILEHELPER_WARNING_BUFFER_FOR_FILE_NOT_OPENED;
    }
    fbs->erase(path);
    
    return FILEHELPER_NO_ERROR;
}

int curSelFile = -1;
FILE* FilesystemHelper::FindFirstFile(const std::string& name){
    LOG_UNIMPLEMENTED_FUNCTION;
    return NULL;
}

FILE* FilesystemHelper::FindNextFile(){
    LOG_UNIMPLEMENTED_FUNCTION;
}

std::vector<std::string> FilesystemHelper::GetFilesList(std::string&& directory){
    DIR *dir; struct dirent *diread;
    std::vector<std::string> files;

    if ((dir = opendir(directory.c_str())) != nullptr) {
        while ((diread = readdir(dir)) != nullptr) {
            files.push_back(diread->d_name);
        }
        closedir (dir);
    } else {
        T5LOG(T5ERROR) << ":: can't open dir, path = " << directory;
        return {};
    }
    if(VLOG_IS_ON(1)){    
        T5LOG( T5INFO) <<":: returned " << files.size() << " files,  path = " << directory;
    }
return files;
}

std::vector<std::string> FilesystemHelper::FindFiles(const std::string& name, int* errcode){
    std::vector<std::string> selFiles;
    int rc = 0;
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
            if(exactMatch) {
                if(file.compare(fileName) == 0){
                        selFiles.push_back(file);
                    }
            } else {
                if( file.find(fileName) != std::string::npos ) {
                    selFiles.push_back(file);
                }
            }
        }
        rc = FILEHELPER_NO_ERROR;
        closedir (dir);
    } else {      
        T5LOG(T5ERROR) << "FilesystemHelper::FindFiles:: dir = " << dirPath << "; can't open directory";
        rc = FILEHELPER_ERROR_CANT_OPEN_DIR;
    }

    if(errcode)
    {
        *errcode = rc;
    }

    return selFiles;
}


int FilesystemHelper::WriteToFile(const std::string& path, const char* buff, const int buffsize){
    std::string fixedPath = path;
    fixedPath = FixPath(fixedPath);
    FILE *ptr = OpenFile(fixedPath, "wb");
    int oldSize = 0;
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
        oldSize = GetFileSize(ptr);
    }
    int errCode = WriteToFile(ptr, buff, buffsize);
    CloseFile(ptr);
    
    return __last_error_code = errCode;
}

int FilesystemHelper::WriteToFile(FILE*& ptr, const void* buff, const long unsigned int buffSize, int &iBytesWritten, const int startingPosition){
    return WriteToFileBuff(ptr,buff,buffSize,iBytesWritten, startingPosition);
}


int FilesystemHelper::WriteToFileBuff(FILE*& ptr, const void* buff, const long unsigned int buffSize, int &iBytesWritten, const int startingPosition){
    std::string fName = GetFileName(ptr);

    if(getFileBufferInstance()->find(fName) != getFileBufferInstance()->end()){
        if(VLOG_IS_ON(1)){
            T5LOG( T5DEBUG) <<"Writing " << buffSize << " bytes to filebuffer " << fName <<
            " starting from position " << startingPosition;
        }
        WriteToBuffer(ptr, buff, buffSize, startingPosition);
        iBytesWritten = buffSize;
    }else{
        if(VLOG_IS_ON(1)){
            T5LOG( T5DEBUG) << "File is not opened in filebuffers-> writting to file, fName = " << fName << ", fId = " <<(long)ptr;

            T5LOG( T5DEBUG) <<"Writing "<<buffSize<< " bytes to file "<< fName<<" starting from position "<<startingPosition;
        }
        #ifndef TEMPORARY_HARDCODED
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
            std::string filesInBuf = "FilesystemHelper::WriteToFileBuff:: files in filebuffer:\t";
            for (auto it = getFileBufferInstance()->begin(); it != getFileBufferInstance()->end(); ++it)
            {
                filesInBuf += it->first + '\t';
            }
            if(VLOG_IS_ON(1)){
                T5LOG( T5DEBUG) << filesInBuf;
            }
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
                T5LOG( T5DEBUG) << "FilesystemHelper::SetOffsetInFilebuffer -> changing offset from "
                    <<pFb->offset<< " to " << offset;
                pFb->offset = offset;
            }else{
                T5LOG( T5WARNING) << "FilesystemHelper::SetOffsetInFilebuffer -> can't change offset from "<<
                        pFb->offset << " to " << offset << " because it's bigger than size:" <<fSize;
            }
        }else{
            T5LOG( T5DEBUG) << "FilesystemHelper::SetOffsetInFilebuffer -> filebuffer not found, fName = " << fName;
        }
    }else{
        T5LOG(T5ERROR) << "FilesystemHelper::SetOffsetInFilebuffer -> file pointer is null";
    }
}


int FilesystemHelper::TruncateFileForBytes(HFILE ptr, int numOfBytes){
    T5LOG( T5INFO) << "Try to truncate file " << FilesystemHelper::GetFileName(ptr) << " for " << numOfBytes << " bytes";
    off_t lDistance = numOfBytes;
    int retCode = ftruncate(fileno(ptr), lDistance);
    return retCode;
}

short FilesystemHelper::SetFileCursor(HFILE fp,long LoPart,long& HiPart,short OffSet)
{
    DWORD ret = 0 ;

    T5LOG( T5INFO) << "SetFilePointer for file " << FilesystemHelper::GetFileName((HFILE)fp) <<  "; offset is " <<
             LoPart << ", direction is " <<OffSet;
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
        T5LOG(T5FATAL) << "SetFilePointerEx::WRONG dwMoveMethod";

    if(OffSet != FILE_BEGIN){           
        T5LOG( T5WARNING) << "SetFilePointerEx::FILE_CURRENT/FILE_END not implemented/tested";
    }
    
    int size = FilesystemHelper::GetFileSize(fp);
    if(size < LoPart){
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG))
            T5LOG( T5WARNING) << "SetFilePointer::File is smaller than requested position -> writing, fname = " <<
                FilesystemHelper::GetFileName(fp);
        TruncateFileForBytes(fp, LoPart);
    }

    res = fseek(fp, LoPart, whence);   

    if(res >= 0){
        //fp->_offset = LoPart;
        LARGE_INTEGER li ;
        li.QuadPart = res ; //It will move High & Low Order bits.
        ret = li.LowPart ;

        HiPart = li.HighPart ;

    }else{
        int k = errno ;
        switch(errno){
        case EBADF : 
            T5LOG(T5ERROR) << "fd is not an open file descriptor. fname = ", ""
            //FilesystemHelper::GetFileName(fp).c_str()
            ;
            break ;
        case EFAULT : 
            T5LOG(T5ERROR) <<"Problem with copying results to user space, fname =  ", ""
            //FilesystemHelper::GetFileName(fp).c_str() 
            ;
            break ;
        case EINVAL : 
            T5LOG(T5ERROR) <<"cw whence is invalid, fname = ", ""
            //FilesystemHelper::GetFileName(fp).c_str() 
            ;
            break ;
        default:
            T5LOG(T5ERROR) <<" default, fname = ", "" 
            //FilesystemHelper::GetFileName(fp).c_str() 
            ;
        }

    }
    T5LOG( T5INFO) <<"SetFilePointer success, file = " << FilesystemHelper::GetFileName(fp);
    return ret ;
}


int FilesystemHelper::WriteToFile(const std::string& path, const unsigned char* buff, const int buffsize){
    return WriteToFile(path, (const char*)buff, buffsize);
}


int FilesystemHelper::WriteToFile(FILE*& ptr, const void* buff, const int buffsize){
    int errCode = FILEHELPER_NO_ERROR;
    int writenBytes = buffsize;
    int oldSize = 0;
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
        oldSize = GetFileSize(ptr);
    }
    #ifdef  LOG_FILE_WRITE
     T5LOG(T5TRANSACTION) <<"called save file " << (long int) ptr <<" to disk;";// << " , stacktrace:" << GET_STACKTRACE_EXPL;
    #endif
    if(ptr == NULL){
        T5LOG(T5ERROR) <<"FilesystemHelper::WriteToFile():: FILEHELPER_FILE_PTR_IS_NULL";
        __last_error_code = errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{ 
        writenBytes *= fwrite(buff, buffsize, 1, ptr);
        if ( writenBytes <=0 ){
            T5LOG(T5ERROR) <<"FilesystemHelper::WriteToFile():: ERROR_WRITE_FAULT";
            __last_error_code = errCode = ERROR_WRITE_FAULT;
        }
    }
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
        T5LOG( T5DEBUG) "FilesystemHelper::WriteToFile("  << (long int) ptr << ") buff = " << "void" <<
             ", buffsize = " << buffsize << ", path = " << GetFileName(ptr) << ", file size = " <<
             GetFileSize(ptr) << ", oldSize = " << oldSize;
    }
    //CloseFile(ptr);
    //return __last_error_code = errCode;
    return writenBytes;
}

int FilesystemHelper::ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPos){
    int err = 0, size = 0;
    err = ReadBuffer(ptr, buff, buffSize, bytesRead, startingPos);
    if(err == FILEHELPER_WARNING_BUFFER_FOR_FILE_NOT_OPENED){
        T5LOG( T5INFO) << "File not found in buffers -> reading from disk, fName = "<< GetFileName(ptr);

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
        T5LOG(T5ERROR) <<"FilesystemHelper::ReadFile():: FILEHELPER_FILE_PTR_IS_NULL");
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{
        bytesRead = fread(buff, buffSize, 1, ptr);
        if(!bytesRead){
            T5LOG( T5WARNING) << "FilesystemHelper::ReadFile():: bytes not readed from ", toStr((long int)ptr).c_str());
            errCode = FILEHELPER_ERROR_FILE_NOT_READ;
        }else{
            T5LOG( T5DEBUG) << "FilesystemHelper::ReadFile():: readed ", toStr(bytesRead).c_str(), "; data = ", buff);
        }
    }
    return __last_error_code = errCode;
}
//*/


int FilesystemHelper::ReadFile(FILE*& ptr, void* buff, const int buffSize, int& bytesRead){
    int errCode = FILEHELPER_NO_ERROR;
    if(!ptr){
        T5LOG(T5ERROR) <<"FilesystemHelper::ReadFile():: FILEHELPER_FILE_PTR_IS_NULL";
        errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }else{
        bytesRead = fread(buff, buffSize, 1, ptr);
        if(!bytesRead){
            T5LOG( T5WARNING) << "FilesystemHelper::ReadFile():: bytes not readed from " << (long int)ptr 
                    <<", path = "<< GetFileName(ptr);
            errCode = FILEHELPER_ERROR_FILE_NOT_READ;
        }else{
            bytesRead = buffSize;
            T5LOG( T5DEBUG) << "FilesystemHelper::ReadFile():: readed " << bytesRead;
        }
    }
    return __last_error_code = errCode;
}


int FilesystemHelper::ReadFileBuff(FILE*& ptr, void* buff, const int buffSize, int& bytesRead, const int startingPosition){
    int errCode = FILEHELPER_NO_ERROR;
    int offset = startingPosition;
    FileBuffer* pFb = NULL;
    if(!ptr){
        T5LOG(T5ERROR) << "FilesystemHelper::ReadFile File pointer is null";
        return __last_error_code = errCode = FILEHELPER_FILE_PTR_IS_NULL;
    }
    std::string fName = GetFileName(ptr);
    if(getFileBufferInstance()->find(fName) != getFileBufferInstance()->end()){
        pFb = &(*getFileBufferInstance())[fName];
        if(offset < 0){
            T5LOG( T5DEBUG) << "FilesystemHelper::ReadFileBuff::  offset[" << offset << "] is less than 0 ->  using pFb->offset" << pFb->offset;
            offset = pFb->offset; 
        }
        if(offset + buffSize <= pFb->data.size()){
            memcpy(buff, &(pFb->data[offset]), buffSize);
            bytesRead = buffSize;
            pFb->offset += bytesRead; 
            T5LOG( T5DEBUG) << "FilesystemHelper::ReadFileBuff:: success, fName = " << fName << "; offset = " << offset << "; bytesRead = "<<bytesRead;
        }else{ 
            T5LOG(T5ERROR) << "FilesystemHelper::ReadFileBuff:: requested file position not exists";
            __last_error_code = errCode = FILEHELPER_ERROR_READING_OUT_OF_RANGE;
        }

    }else{
        T5LOG( T5DEBUG) << "FilesystemHelper::ReadFileBuff, file buff not found-> using reading directly from file, fName = " << fName;
        errCode = ReadFile(ptr, buff, buffSize, bytesRead);
    }
    return errCode;
}



#include <sys/stat.h>

long FnGetFileSize(std::string&& filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    int logLevel = rc ==0? T5DEBUG : T5ERROR;
    if(rc == 0){
        T5LOG( T5DEBUG) << "FnGetFileSize:: for "<< filename<< " is "<<stat_buf.st_size;
    }else{
        T5LOG(T5ERROR) << "FnGetFileSize:: for "<< filename<< " is "<<stat_buf.st_size<<" , errno = "<<strerror(errno) ;
    }

    return rc == 0 ? stat_buf.st_size : -1;
}

long FdGetFileSize(int fd)
{
    struct stat stat_buf;
    int rc = fstat(fd, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

size_t FilesystemHelper::GetFileSize(const std::string& path){
    auto size = FnGetFileSize(path.c_str());
    return size;
}


size_t FilesystemHelper::GetFileSize(FILE*& ptr){
    if(!ptr){
        __last_error_code = FILEHELPER_FILE_PTR_IS_NULL;
        return -1;
    }
    int pos = ftell(ptr);
    fseek(ptr, 0L, SEEK_END);
    int size = ftell(ptr);
    fseek(ptr, 0L, pos);
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)){
        std::string fName = GetFileName(ptr);
        T5LOG( T5DEBUG) << "FilesystemHelper::GetFileSize("<<(long int)ptr<< ") = "<<size<<", fName = "<<fName;
    }
    return size;
}

size_t FilesystemHelper::GetFilebufferSize(const std::string& path){
    auto fbs = getFileBufferInstance();
    if(fbs->find(path) == fbs->end()){
        T5LOG( T5INFO) <<":: Requested filebuffer not found, path = "<< path;
        return 0;
    }
    auto size = (*fbs)[path].data.size();
    T5LOG( T5INFO) <<":: path = "<< path<< "; size = "<<size;
    return size;
}

size_t FilesystemHelper::GetTotalFilebuffersSize() {
    auto fbs = getFileBufferInstance();
    size_t total = 0;
    for (auto it = fbs->cbegin(); it != fbs->cend(); it++)
    {
        //total += it->second.data.size();
        total += it->second.data.capacity();
    }
    T5LOG( T5INFO) <<":: total size = "<<total;
    return total;
}

int FilesystemHelper::GetLastError(){
    return __last_error_code;
}

int FilesystemHelper::ResetLastError(){
    T5LOG( T5INFO) << "FilesystemHelper::ResetLastError()";
    __last_error_code = FILEHELPER_NO_ERROR;
    return 0;
}


std::string FilesystemHelper::GetOtmDir(){
    return _otmDir;
}
std::string FilesystemHelper::GetHomeDir(){
    return _homeDir;
}

DECLARE_string(log_dir);

#define HOME_ENV "HOME"
int FilesystemHelper::init(){
    char* _home_dir = getenv(HOME_ENV);
    struct passwd *pswd = nullptr;
    if (!_home_dir || !strlen(_home_dir)){         
        pswd = getpwuid(getuid());        
        if (!pswd){
            T5LOG(T5ERROR) << "::ERROR_FILE_CANT_GET_USER_PSWD";
            return FILEHELPER_ERROR_FILE_CANT_GET_USER_PSWD;
        }
    }
    if (!strlen(_home_dir)){
        T5LOG(T5ERROR) << "::ERROR_FILE_CANT_GET_HOME_DIR";
        return FILEHELPER_ERROR_FILE_CANT_GET_HOME_DIR;
    }
    if(pswd) _home_dir = pswd->pw_dir;
    _homeDir = _home_dir;
    _otmDir = _homeDir + "/.t5memory/";
    _memDir = _otmDir +"MEM/";
    _tableDir = _otmDir +"TABLE/";
    _tempDir = _otmDir +"TEMP/";

    std::string tmFiles = _memDir + DEFAULT_PATTERN_NAME + EXT_OF_MEM ;

    int errcode = 0;
    auto files = FilesystemHelper::FindFiles(tmFiles, &errcode);
    if(errcode == FilesystemHelper::FILEHELPER_ERROR_CANT_OPEN_DIR){
        errcode = FilesystemHelper::CreateDir(_memDir);
        if( errcode != FilesystemHelper::FILEHELPER_NO_ERROR ){
            T5LOG(T5FATAL) << ":: error with filesystem helper, creating memDir, errcode = " << errcode << "; memDir = " << _memDir;
            throw;
        }
    }  

    if(FLAGS_log_dir.empty() || FilesystemHelper::CreateDir(FLAGS_log_dir)){
        std::string defaultLogPath = _otmDir + "LOG/";
        T5LOG(T5WARNING) << "FLAGS_log_dir is empty or failed to create log dir under specified location(path = \"" 
            << FLAGS_log_dir <<"\"), setting up default log path and checking/creating default log dir (defaultPath = \"" <<
            defaultLogPath <<"\").";
        FLAGS_log_dir = defaultLogPath;
        if( FilesystemHelper::CreateDir(FLAGS_log_dir) != FilesystemHelper::FILEHELPER_NO_ERROR ){
            T5LOG(T5ERROR) << ":: error with filesystem helper, when creating logDir, errcode = " << errcode << "; logDir = " << FLAGS_log_dir;
        }
    }   

    return -1;
}

std::string FilesystemHelper::GetMemDir(){
    return _memDir;
}

std::string FilesystemHelper::GetTableDir(){
    return _tableDir;
}

std::string FilesystemHelper::GetTempDir(){
    return _tempDir;
}

std::string FilesystemHelper::BuildTempFileName(){
    int iRC = 0;
    std::string sTempPath = GetTempDir();
    // setup temp file name for TMX file 
        
    if(!DirExists(sTempPath) && CreateDir(sTempPath)){
        T5LOG(T5FATAL) << "Cant create dir for temporary files under path = " << sTempPath;
        throw;
    }
    {
        std::lock_guard<std::mutex> l{FilesystemHelper::fsLock};// lock fs
        int i = 0;
        sTempPath += "OTM";
        std::string checkName = sTempPath;
        while( i < 10000 ){
            checkName = sTempPath + std::to_string(i/100) + std::to_string(i%100/10) + std::to_string(i%10);
            auto files = FindFiles( checkName );
            
            if(files.size() == 0){// we can use this name
                T5LOG( T5INFO) << "::Temp file's Name found :" << checkName ;
                auto file = fopen(checkName.c_str(), "w");
                if(file){
                    fclose(file);
                    return checkName;
                }else{
                    T5LOG(T5ERROR) << "::can't create/open file " << checkName;
                    return ("");   
                }
            }
            i++;
        }

        T5LOG(T5ERROR) << "::TO_DO::All temp names is already used - delete some of them";
    }
    return( "" );
}
    
int FilesystemHelper::CreateDir(const std::string& dir, int rights) {
    std::lock_guard<std::mutex> l{FilesystemHelper::fsLock};// lock fs
    struct stat st;
    int ret = stat(dir.c_str(), &st);
    T5LOG( T5DEVELOP) <<  "FilesystemHelper::CreateDir(" << dir << "; rights = " << rights << ")::stat():: ret = " << ret;
    if (ret){
        ret = mkdir(dir.c_str(), rights);
        if (ret)
        {
            T5LOG( T5ERROR) << "FilesystemHelper::CreateDir(" << dir << "; rights = " << rights << ") wasn't created, ret = " << ret;
        } else {
            T5LOG( T5INFO) << "FilesystemHelper::CreateDir(" << dir << "; rights = " << rights << ") was created, ret = " << ret;
        }
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
        T5LOG( T5DEBUG) << "FilesystemHelper::DirExists(" << path <<") exists";
    }else{
        T5LOG( T5INFO) << "FilesystemHelper::DirExists(" << path << ") not exists";
    }
    return bExists;
}
