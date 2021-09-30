#include "win_types.h"

#include "EQFMEMIE.H"

class EqfPluginWrapper{
public:
    static void init();
    static void deinit();

    static USHORT MemExportStart     ( char* pszOutFile, PMEMEXPIMPINFO pMemInfo );
    static USHORT MemExportProcess   ( PMEMEXPIMPSEG pSeg);
    static USHORT MemExportEnd       ( );
    static USHORT MemImportStart     ( char*  pszInFile, PMEMEXPIMPINFO pMemInfo );
    static USHORT MemImportProcess   ( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, PLONG lProgress );
    static USHORT MemImportEnd       ( );
};
