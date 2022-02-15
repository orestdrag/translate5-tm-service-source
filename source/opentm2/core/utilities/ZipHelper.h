
#ifndef _ZIP_HELPER_H_
#define _ZIP_HELPER_H_

#include "zip/zip.h"

typedef zip_t ZIP;


class ZipHelper{
public:
    static ZIP* ZipCreate ( const char* fName );
    static int  ZipAdd    ( ZIP * pZip, const char* fName );
    static ZIP* ZipOpen   ( const char* fName , char mode );
    static int  ZipClose  ( ZIP* pZip );

    static int ZipExtract( ZIP* pZip );
    static int ZipAppend( const char* zipName, const char* fName);
};


#endif //_ZIP_HELPER_H_