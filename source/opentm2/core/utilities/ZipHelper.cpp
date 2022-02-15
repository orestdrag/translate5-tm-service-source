#include "ZipHelper.h"
#include "EQF.H"
#include "LogWrapper.h"

int  ZipHelper::ZipAdd    ( ZIP * pZip, const char* fName ){
    if(pZip){
        LogMessage3(ERROR,__func__,"::adding to zip file ", fName);
    }else{
        LogMessage3(ERROR,__func__,":: can't add to zip file ", fName);
    }
    zip_entry_open(pZip, UtlGetFnameFromPath(fName));
    {
        zip_entry_fwrite( pZip, fName);
    }
    zip_entry_close(pZip);
    return 0;
}

ZIP*  ZipHelper::ZipOpen   ( const char* fName , char mode){
    struct zip_t * zip = zip_open(fName, ZIP_DEFAULT_COMPRESSION_LEVEL, mode); 
    if(zip){
        LogMessage3(ERROR,__func__,":: Opened zip file ", fName);
    }else{
        LogMessage3(ERROR,__func__,":: can't open zip file ", fName);
    }
    return zip;
}

int  ZipHelper::ZipClose  ( ZIP* pZip ){
    if(pZip){
        LogMessage2(ERROR,__func__,"::closing zip file ");
    }else{
        LogMessage2(ERROR,__func__,":: can't close zip file , ptr = nullptr");
    }
    zip_close(pZip);
    return 0;
}

int on_extract_entry(const char *filename, void *arg) {
    int n = *(int *)arg;
    LogMessage3(INFO,__func__,"::Extracted: ", filename);//," ("," of ", toStr(n),")\n" );
    return 0;
}

int ZipHelper::ZipExtract( const char* zipPath, const char* destPath){
    
    int arg = 2;
    zip_extract(zipPath, destPath, on_extract_entry, &arg);
    return 0;
}