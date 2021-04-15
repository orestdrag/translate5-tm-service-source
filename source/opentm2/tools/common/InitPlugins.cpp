//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
extern "C" {
#include "EQF.H"
}
#include "../../core/pluginmanager/OtmPlugin.h"
#include "../../core/pluginmanager/PluginManager.h"
#include "InitPlugins.h"
#include "OTMFUNC.H"

// initialize the Plugin-Manager and load the available plugins
USHORT InitializePlugins( char *szPluginPath )
{
    USHORT usRC = NO_ERROR;         // function return code (add for P402974)
	PluginManager* thePluginManager = PluginManager::getInstance();
	usRC = thePluginManager->loadPluginDlls(szPluginPath);   // add for P402974
#ifdef TEMPORARY_COMMENTED
	InitTMPluginWrapper(); // seems like an empty method
	InitDictPluginWrapper(); // dictionary is not needed
#endif
	InitMarkupPluginMapper(); // tagtable
	InitDocumentPluginMapper(); // document

    return usRC;
}
