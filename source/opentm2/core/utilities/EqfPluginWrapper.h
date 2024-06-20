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
};
#endif
