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
	//#ifdef TEMPORARY_COMMENTED
	usRC = thePluginManager->loadPluginDlls(szPluginPath);   // add for P402974
	//#endif

	InitMarkupPluginMapper(); // tagtable
	InitDocumentPluginMapper(); // document

    return usRC;
}
