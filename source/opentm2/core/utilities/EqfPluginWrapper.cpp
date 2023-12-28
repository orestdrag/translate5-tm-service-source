#include "EqfPluginWrapper.h"
#include "EQF.H"
#include "Property.h"

void EqfPluginWrapper::init(){

}

void EqfPluginWrapper::deinit(){

}



USHORT EqfPluginWrapper::MemImportStart     (PLONG plHandle,  PSZ pszInFile, PMEMEXPIMPINFO pMemInfo, ImportStatusDetails*     pImportData  ){
    return EXTMEMIMPORTSTART(plHandle, pszInFile, pMemInfo, pImportData);
}

USHORT EqfPluginWrapper::MemImportProcess   (LONG plHandle,  PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG pMemHandle, ImportStatusDetails*     pImportData ){
    return EXTMEMIMPORTPROCESS(plHandle, pfnInsertSegment, pMemHandle, pImportData);
}

USHORT EqfPluginWrapper::MemImportEnd       (LONG plHandle ){
    return EXTMEMIMPORTEND(plHandle);
}

