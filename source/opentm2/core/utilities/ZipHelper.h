
#ifndef _ZIP_HELPER_H_
#define _ZIP_HELPER_H_

typedef void* ZIP;


class ZipHelper{
public:
    static ZIP* ZipCreate ( const char* fName );
    static int  ZipAdd    ( ZIP * pZip, const char* fName );
    static ZIP* ZipOpen   ( const char* fName );
    static int  ZipClose  ( ZIP* pZip );
};


#endif //_ZIP_HELPER_H_