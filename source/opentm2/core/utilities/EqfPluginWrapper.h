#ifndef _EQF_PLUGIN_WRAPPER_H_
#define _EQF_PLUGIN_WRAPPER_H_

#include "win_types.h"

#include "EQFMEMIE.H"

#include "tm.h"

class EqfPluginWrapper{
public:
    static void init();
    static void deinit();

    static USHORT MemExportStart     (PLONG plHandle, char* pszOutFile, PMEMEXPIMPINFO pMemInfo );
    static USHORT MemExportProcess   (LONG plHandle, PMEMEXPIMPSEG pSeg);
    static USHORT MemExportEnd       (LONG plHandle );

    static USHORT MemImportStart     (PLONG plHandle, char*  pszInFile, PMEMEXPIMPINFO pMemInfo, ImportStatusDetails*     pImportData   );
    static USHORT MemImportProcess   (LONG plHandle, PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, ImportStatusDetails*     pImportData );
    static USHORT MemImportEnd       (LONG plHandle );
};
#endif
