
#ifndef _ZIP_HELPER_H_
#define _ZIP_HELPER_H_

#include "zip/zip.h"

typedef zip_t ZIP;


class ZipHelper{
public:
    static int  ZipAdd    ( ZIP * pZip, const char* fName );
    static ZIP* ZipOpen   ( const char* fName , char mode );
    static int  ZipClose  ( ZIP* pZip );
    static int  ZipExtract( const char* zipPath, const char* destPath );     
};


#endif //_ZIP_HELPER_H_