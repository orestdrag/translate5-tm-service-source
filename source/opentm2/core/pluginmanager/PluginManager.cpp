/*! \file
	\par Copyright Notice:
	Copyright (C) 1990-2015 International Business Machines Corporation and others. All rights reserved.
*/

#include "PluginManager.h"
#include "PluginManagerImpl.h"
#include "tm.h"
#include "../utilities/LogWrapper.h"
/*! \mainpage PluginManager and plugins for OpenTM2
	\par Copyright Notice:
	Copyright (C) 1990-2012, International Business Machines Corporation and others. All rights reserved.

	<HR>
	\section witpm What is the PluginManager, what are plugins?
	The PluginManager of OpenTM2 allows to add and control plugins in OpenTM2
	and ensures that the plugins supply the appropriate interface.\n
	A plugin is a software module adding new or enhancing existing functionality
	of the product.\n\n
	Plugins can be used for:
	- Translation Memory systems
	- Markups for the support of specific file formats
	- Dictionary systems
	- Spell checkers
	- Morphologic support
	- Analysis/Segmentation modules
	- Word count and reporting modules
	- ...
	
	\n
	Plugins have to be provided as one or more DLLs, which must be copied to the
	plugin-directory of OpenTM2. They will be loaded at startup.\n
	Refer to the class description of OtmPlugin for further information
	about how to implement plugins.
	
	\n
	The PluginManager itself consists of two parts:
	- an interface (the class PluginManager)
	- an implementation (the class PluginManagerImpl)
	For plugin-development, the interface class is the most important part.
	\n
	To ensure independancy of a specific platform and compiler, the PluginManager
	uses the C++ standard libraries incl. the Standard Template Library (STL).
*/

void StrcatName(const char * strName, const char * strSymbol, char * strOutput); // Add for P403138

/** Initialize the static instance variable */
PluginManager* PluginManager::instance = 0;

PluginManager* PluginManager::getInstance()
{
	if (instance == 0)
	{
		instance = new PluginManager();
		instance->pImpl = new PluginManagerImpl();
	}
	return instance;
}

PluginManager::eRegRc PluginManager::registerPlugin(OtmPlugin* plugin)
{
	return instance->pImpl->registerPlugin(plugin);
}

PluginManager::eRegRc PluginManager::deregisterPlugin(OtmPlugin* plugin)
{
	return instance->pImpl->deregisterPlugin(plugin);
}

int PluginManager::getPluginCount()
{
	return instance->pImpl->getPluginCount();
}

OtmPlugin* PluginManager::getPlugin(const char* name)
{
	return instance->pImpl->getPlugin(name);
}

OtmPlugin* PluginManager::getPlugin(int iIndex)
{
	return instance->pImpl->getPlugin(iIndex);
}

OtmPlugin* PluginManager::getPlugin(OtmPlugin::ePluginType type, int n)
{
	return instance->pImpl->getPlugin(type, n);
}

USHORT PluginManager::loadPluginDlls(const char* pszPluginDir)
{
	return instance->pImpl->loadPluginDlls(pszPluginDir);
}


// Add for P403138 start
int PluginManager::ValidationCheck(char * strParam)
{
    int nRC = NO_ERROR;
    return nRC;
}

void StrcatName(const char * strName, const char * strSymbol, char * strOutput)
{
    if ((NULL != strOutput) && (strlen(strOutput) != 0))
    {
        strcat(strOutput, strSymbol);
        strcat(strOutput, " ");
        strcat(strOutput, strName);
    }
    else
    {
        strcpy(strOutput, strName);
    }
}
// Add end

/*
void PluginManager::destroy()
{
	if (instance != 0 && instance->pImpl != 0)
		delete instance->pImpl;
	if (instance != 0)
		delete instance;
	instance = 0;
}
*/
