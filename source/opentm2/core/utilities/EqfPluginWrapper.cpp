#include "EqfPluginWrapper.h"
#include "EQF.H"
#include "PropertyWrapper.H"



void EqfPluginWrapper::init(){
     #ifdef TEMPORARY_COMMENTED
     //usRc = DosLoadModule( NULL, 0, "OTMTMXIE", &(pExportIDA->hmodMemExtExport) );
    char libPath[MAX_EQF_PATH];
    properties_get_str(KEY_PLUGIN_DIR, libPath, MAX_EQF_PATH);
    strcat(libPath, "libEqfMemoryPlugin.so");
    usRc = DosLoadModule( NULL, 0, libPath, &(pExportIDA->hmodMemExtExport));

    #endif

}

void EqfPluginWrapper::deinit(){
    #ifdef TEMPORARY_COMMENTED

    #endif


}



USHORT EqfPluginWrapper::MemExportStart     ( char*  pszOutFile, PMEMEXPIMPINFO pMemInfo ){

}

USHORT EqfPluginWrapper::MemExportProcess   ( PMEMEXPIMPSEG pSeg){

}

USHORT EqfPluginWrapper::MemExportEnd       (  ){

}

USHORT EqfPluginWrapper::MemImportStart     (  PSZ pszInFile, PMEMEXPIMPINFO pMemInfo ){

}

USHORT EqfPluginWrapper::MemImportProcess   (  PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, PLONG lProgress ){

}

USHORT EqfPluginWrapper::MemImportEnd       ( ){

}




#ifdef API_functions

typedef USHORT (/*__cdecl*/ *PFN_EXTMEMEXPORTSTART)( PLONG plHandle, PSZ pszOutFile, PMEMEXPIMPINFO pMemInfo );
typedef USHORT (/*__cdecl*/ *PFN_EXTMEMEXPORTPROCESS)( LONG lHandle, PMEMEXPIMPSEG pSeg);
typedef USHORT (/*__cdecl*/ *PFN_EXTMEMEXPORTEND)( LONG lHandle );
typedef USHORT (/*__cdecl*/ *PFN_EXTMEMIMPORTSTART)( PLONG plHandle, PSZ pszInFile, PMEMEXPIMPINFO pMemInfo );
typedef USHORT (/*__cdecl*/ *PFN_EXTMEMIMPORTPROCESS)( LONG lHandle, PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, PLONG lProgress );
typedef USHORT (/*__cdecl*/ *PFN_EXTMEMIMPORTEND)(LONG lHandle );
#endif
