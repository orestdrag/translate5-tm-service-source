#include "ZipHelper.h"
#include "EQF.H"
#include "LogWrapper.h"

ZIP* ZipHelper::ZipCreate ( const char* fName ){
    struct zip_t *zip = zip_open(fName, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    if(zip){
        LogMessage3(ERROR,__func__,"::created zip file ", fName);
    }else{
        LogMessage3(ERROR,__func__,":: can't open zip file ", fName);
    }

    return zip;
    /*
    {
        zip_entry_open(zip, "foo-1.txt");
        {
            const char *buf = "Some data here...\0";
            zip_entry_write(zip, buf, strlen(buf));
        }
        zip_entry_close(zip);
    }//*/
    //zip_close(zip);
    //return nullptr;
    //return ZipOpen(fName, 'w');
}
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


int  ZipHelper::ZipAppend( const char* zipName, const char* fName){
    struct zip_t *zip = zip_open(zipName, ZIP_DEFAULT_COMPRESSION_LEVEL, 'a');
    {
        zip_entry_open(zip, UtlGetFnameFromPath(fName));
        {
            zip_entry_fwrite( zip, fName);
        }
        zip_entry_close(zip);
    }
    zip_close(zip);
}


ZIP*  ZipHelper::ZipOpen   ( const char* fName , char mode){
    auto zip = zip_open(fName, ZIP_DEFAULT_COMPRESSION_LEVEL, mode); 
    return nullptr;
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
