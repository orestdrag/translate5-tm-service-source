
#include "EQF.H"
#include "LogWrapper.h"

int  FilesystemHelper::ZipAdd    ( ZIP * pZip, const std::string&  fName ){
    if(pZip){
        T5LOG( T5INFO) << "::adding to zip file " << fName;
    }else{
        T5LOG(T5ERROR) << ":: can't add to zip file " << fName;
    }
    zip_entry_open(pZip, UtlGetFnameFromPath(fName.c_str()));
    {
        zip_entry_fwrite( pZip, fName.c_str());
    }
    zip_entry_close(pZip);
    return 0;
}

ZIP*  FilesystemHelper::ZipOpen   ( const std::string& fName , char mode){
    struct zip_t * zip = zip_open(fName.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, mode); 
    if(zip){
        T5LOG( T5INFO) << ":: Opened zip file " << fName;
    }else{
        T5LOG(T5ERROR)  << ":: can't open zip file " << fName;
    }
    return zip;
}

int  FilesystemHelper::ZipClose  ( ZIP* pZip ){
    if(pZip){
        T5LOG( T5INFO) << "::closing zip file ";
    }else{
        T5LOG(T5ERROR) << ":: can't close zip file , ptr = nullptr";
    }
    zip_close(pZip);
    return 0;
}

int on_extract_entry(const char *filename, void *arg) {
    int n = *(int *)arg;
    T5LOG( T5INFO) << "::Extracted: " << filename;//," ("," of ", toStr(n),")\n" );
    return 0;
}

int FilesystemHelper::ZipExtract( const std::string&  zipPath, const std::string&  destPath){
    
    int arg = 2;
    zip_extract(zipPath.c_str(), destPath.c_str(), on_extract_entry, &arg);
    return 0;
}