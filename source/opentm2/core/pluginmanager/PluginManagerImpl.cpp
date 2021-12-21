/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include <set>
#include <algorithm> 
#include <string>
#include <dlfcn.h>
#include <gnu/lib-names.h>
#include "PluginManager.h"
#include "PluginManagerImpl.h"
#include "OtmPlugin.h"
#include "PropertyWrapper.H"
#include "../utilities/LogWrapper.h"

using namespace std;

#define PLUGIN_PATH_MAX_DEPTH          2

#ifndef CPPTEST
extern "C"
{
#endif
	#include "EQF.H"
#ifndef CPPTEST
}
#endif

/*! \typedef PLUGINPROC
	Prototype for the registerPlugins() function
*/
typedef unsigned short (/*__cdecl*/ *PLUGINPROC) ();   // Modify for P402974

// Add for P403115 start
BOOL DllNameCheck(const char * strName);
// Add end

PluginManagerImpl::PluginManagerImpl()
{
	pluginSet = new PLUGINSET;
	bRegisterAllowed = false;
  iCurrentlyLoadedPluginDLL = -1;
  m_iNextCommandID = ID_TWBM_AAB_TOOLPLUGINS;
}

PluginManagerImpl::~PluginManagerImpl()
{
	delete pluginSet;
}

PluginManager::eRegRc PluginManagerImpl::registerPlugin(OtmPlugin* plugin)
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;
	if (bRegisterAllowed)
	{
		if (plugin != 0)
		{
			std::string name(plugin->getName());
			if (!name.empty())
      {
#ifdef TEMPORARY_COMMENTED_LOGS
        this->Log.writef( "   Registering plugin %s", name.c_str() );
#endif //TEMPORARY_COMMENTED

        // insert plugin into list of active plugins
				PLUGINSET::iterator it;
				for (it = pluginSet->begin(); it != pluginSet->end() && eRc == PluginManager::eSuccess; it++)
				{
					std::string tmpName((*it)->getName());
					if (tmpName == name)
					{
						eRc = PluginManager::eAlreadyRegistered;

#ifdef TEMPORARY_COMMENTED_LOGS
                        this->Log.writef( "Error:   The plugin %s is already registered.", name.c_str() ); // Add for 403115
#endif // TEMPORARY_COMMENTED
					}
				}

        // insert plugin into plugin list of currently loaded plugin DLL
				if (eRc == PluginManager::eSuccess)
				{
					pluginSet->insert(plugin);

          // update list of plugins registered for active plugin DLL
          if ( iCurrentlyLoadedPluginDLL != -1 )
          {
            vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].vPluginList.push_back(plugin);
          } /* end */             
				}

        // notify all plugin listeners
				if (eRc == PluginManager::eSuccess)
				{
          NotifyListeners( PluginListener::eStarted, plugin->getName(), plugin->getType(), false );
				}
			}
			else
			{
				eRc = PluginManager::eInvalidName;
			}
		}
		else
		{
			eRc = PluginManager::eInvalidParm;
		}
	}
	else
	{
		eRc = PluginManager::eInvalidRequest;
	}

#ifdef TEMPORARY_COMMENTED_LOGS
    this->Log.writef( "   Registering plugin %d", eRc );
#endif //TEMPORARY_COMMENTED

	return eRc;
}

PluginManager::eRegRc PluginManagerImpl::deregisterPlugin(OtmPlugin* plugin)
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;

	if (plugin != 0)
	{
    // remove plugin from our list of plugins
    for ( PLUGINSET::iterator it = pluginSet->begin(); it != pluginSet->end(); it++)
		{
			if ((*it) == plugin)
			{
        pluginSet->erase( it );
        break; 
			}
		}

    // remove plugin from plugin list of loadded plugin DLLs
    int iPluginEntry = findPluginEntry( plugin );
    if ( iPluginEntry != -1 )
    {
      for ( std::vector<OtmPlugin *>::iterator it = vLoadedPluginDLLs[iPluginEntry].vPluginList.begin(); it != vLoadedPluginDLLs[iPluginEntry].vPluginList.end(); it++)
		  {
			  if ((*it) == plugin)
			  {
          vLoadedPluginDLLs[iPluginEntry].vPluginList.erase( it );
          break; 
			  }
		  }
    }
	}
	else
	{
		eRc = PluginManager::eInvalidParm;
	}
	return eRc;
}

int PluginManagerImpl::getPluginCount()
{
	return pluginSet->size();
}

OtmPlugin* PluginManagerImpl::getPlugin(const char* name)
{
	OtmPlugin* plugin = 0;
	if (name && *name != '\0')
	{
		PLUGINSET::iterator it;
		for (it = pluginSet->begin(); it != pluginSet->end() && plugin == 0; it++)
		{
			if ( strcmp( (*it)->getName(), name ) == 0 )
			{
				plugin = *it;
			}
		}
	}
	return plugin;
}

OtmPlugin* PluginManagerImpl::getPlugin(int iIndex)
{
	OtmPlugin* plugin = 0;
	if (iIndex < getPluginCount())
	{
		PLUGINSET::iterator it;
		for (it = pluginSet->begin(); iIndex > 0; iIndex--, it++);
		plugin = *it;
	}
	return plugin;
}

OtmPlugin* PluginManagerImpl::getPlugin(OtmPlugin::ePluginType type, int n)
{
	OtmPlugin* plugin = 0;
	if (type != OtmPlugin::eUndefinedType)
	{
		PLUGINSET::iterator it;
		for (it = pluginSet->begin(); it != pluginSet->end() && plugin == 0; it++)
		{
    	OtmPlugin* pTest = *it;
			if (pTest->getType() == type)
			{
				n--;
				if (n <= 0)
				{
					plugin = pTest;
				}
			}
		}
	}
	return plugin;
}

USHORT PluginManagerImpl::loadPluginDlls(const char* pszPluginDir)
{
  
  USHORT usRC = PluginManager::eSuccess;         // function return code add for P402974
  //return usRC;

	std::string strFileSpec(pszPluginDir);
	BOOL fMoreFiles = TRUE;
	HANDLE hDir = HDIR_CREATE;
	WIN32_FIND_DATA ffb;

  BOOL fLogHasBeenOpened = FALSE;
  
LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 75 // check the depths of the cycle  if (IsDepthOvered( pszPluginDir )) // util");
#ifdef TEMPORARY_COMMENTED
LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 76");
#ifdef TEMPORARY_COMMENTED
#ifdef _DEBUG
  if ( !this->Log.isOpen() )
  {
    this->Log.open( "PluginManager" );
      fLogHasBeenOpened = TRUE;
  } /* end */     
  this->Log.writef( "Loading plugin DLLs from directory %s...", pszPluginDir );
#endif
#endif //TEMPORARY_COMMENTED

  // check the depths of the cycle
  if (IsDepthOvered( pszPluginDir )) // util
  {
LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 77");
#ifdef TEMPORARY_COMMENTED
      this->Log.writef( "...skipping directory, max. nesting level reached" );
      if ( fLogHasBeenOpened ) this->Log.close();
#endif
      return usRC;
  }
#endif

	// allow calling the registerPlugin()-method
	bRegisterAllowed = true;
  
  strFileSpec += "libEqfMemoryPlugin.so";

    // Modify end
    USHORT usSubRC = loadPluginDll(strFileSpec.c_str());  // add return value for P402974
    // Add for P402974 start
    if (PluginManager::ePluginExpired == usSubRC)
    {
    //TEMPORARY_COMMENTED
      // expired is the highest authority
    //              this->Log.writef("Error:   DLL %s is expired.", strDll.c_str());
      usRC = usSubRC;
    }
    else if (usSubRC && (PluginManager::ePluginExpired != usRC) && (PluginManager::eAlreadyRegistered != usSubRC))
    {
      // else is other error and skip already registered error
      usRC = usSubRC;
    }

#ifdef TEMPORARY_COMMENTED_LOGS
 this->Log.writef( "   running FindFirst for %s...", strFileSpec.c_str() );
#endif //TEMPORARY_COMMENTED
  
LogMessage2(ERROR,__func__, ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 23 hDir = FindFirstFile( strFileSpec.c_str(), &ffb );");
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
    hDir = FindFirstFile( strFileSpec.c_str(), &ffb );

  if ( hDir != INVALID_HANDLE_VALUE )
  {
    fMoreFiles = TRUE;
	  while ( fMoreFiles )
	  {
		  std::string strDll(pszPluginDir);
		  strDll += '\\';
		  strDll += ffb.cFileName;
          // Modify for P403115 start
          if (DllNameCheck(strDll.c_str()))
          {
//TEMPORARY_COMMENTED
//              this->Log.writef( "   found DLL %s", strDll.c_str() );
          }
          else
          {
//TEMPORARY_COMMENTED
//              this->Log.writef("Error:   DLL %s is not valid, skip", strDll.c_str());
              //fMoreFiles= FindNextFile(hDir, &ffb);
              continue;
          }
          // Modify end
		  USHORT usSubRC = loadPluginDll(strDll.c_str());  // add return value for P402974
          // Add for P402974 start
          if (PluginManager::ePluginExpired == usSubRC)
          {
//TEMPORARY_COMMENTED
              // expired is the highest authority
//              this->Log.writef("Error:   DLL %s is expired.", strDll.c_str());
              usRC = usSubRC;
          }
          else if (usSubRC && (PluginManager::ePluginExpired != usRC) && (PluginManager::eAlreadyRegistered != usSubRC))
          {
              // else is other error and skip already registered error
              usRC = usSubRC;
          }
          // Add end
		  //fMoreFiles= FindNextFile( hDir, &ffb );
	  } /* endwhile */
    //FindClose( hDir );
  }
  else
  {
    int iRC = 0;//GetLastError();
//TEMPORARY_COMMENTED
//    this->Log.writef( "   FindFirstFile failed with rc=%ld", iRC );
  } /* endif */     
#endif

	// now search all subdirectories
	strFileSpec = pszPluginDir;
	strFileSpec += "/*";
	//hDir = FindFirstFile( strFileSpec.c_str(), &ffb );
LogMessage2(ERROR,__func__, ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 24 if ( hDir != INVALID_HANDLE_VALUE )");
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    fMoreFiles = TRUE;

	  while ( fMoreFiles )
	  {
      if ( (ffb.cFileName[0] != '.') && (ffb.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		  {
			  std::string strSubdir(pszPluginDir);
			  strSubdir += '\\';
			  strSubdir += ffb.cFileName;
			  USHORT usSubRC = loadPluginDlls( strSubdir.c_str() ); // Modify for P402974
              // Add for P402974 start
              if (PluginManager::ePluginExpired == usSubRC)
              {
//TEMPORARY_COMMENTED
                  // expired is the highest authority
//                  this->Log.writef("Error:   DLL %s is expired.", strSubdir.c_str());
                  usRC = usSubRC;
              }
              else if (usSubRC && (PluginManager::ePluginExpired != usRC) && (PluginManager::eAlreadyRegistered != usSubRC))
              {
                  // else is other error and skip already registered error
                  usRC = usSubRC;
              }
              // Add end
		  }
		  //fMoreFiles= FindNextFile( hDir, &ffb );
	  }
    //FindClose( hDir );
  } /* endif */     
#endif
//TEMPORARY_COMMENTED
//  this->Log.write( "   ...Done" );

	// disallow calling the registerPlugin()-method
	bRegisterAllowed = false;
	
//TEMPORARY_COMMENTED
//  if ( fLogHasBeenOpened ) this->Log.close();

  return usRC;     // Add for P402974
}

USHORT PluginManagerImpl::loadPluginDll(const char* pszName)
{
    USHORT usRC = PluginManager::eSuccess;         // function return code add for P402974

    //SetErrorMode(  SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX  );
    // Add for P402792 start
    if (!vLoadedPluginDLLs.empty())
    {
        for (size_t iInx = 0; iInx < vLoadedPluginDLLs.size(); iInx++)
        {
            if (!strcasecmp(vLoadedPluginDLLs[iInx].strDll, pszName))
            {
                return usRC;
            }
        }
    }

    #ifdef TO_BE_REMOVED
    
    void *handle = dlopen("libEqfMemoryPlugin.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = x");
#ifdef TEMPORARY_COMMENTED
        return PluginManager::eNotEnoughMemory;
#endif //TEMPORARY_COMMENTED
    }

    dlerror();

    unsigned short (*pFunc)();
    pFunc = (unsigned short (*)())dlsym(handle, "registerPlugins");
    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = x");
#ifdef TEMPORARY_COMMENTED
        this->Log.write( "   Could not resolve address of function \"registerPlugins()\"" );
        dlclose(handle);
        return PluginManager::eNotEnoughMemory;
#endif //TEMPORARY_COMMENTED
    }

  #endif//TO_BE_REMOVED


    // prepare our loaded DLL entry
    //iCurrentlyLoadedPluginDLL = vLoadedPluginDLLs.size(); vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL]
    vLoadedPluginDLLs.push_back( LoadedPluginDLL() );
    LoadedPluginDLL& rNewPluginDll = vLoadedPluginDLLs.back();
    #ifdef TO_BE_REMOVED
    rNewPluginDll.hMod = handle;
    #endif
    
    // Add for P402792 start
    memset(rNewPluginDll.strDll, 0x00,
        sizeof(rNewPluginDll.strDll));
    strcpy(rNewPluginDll.strDll, pszName);
    // Add end

    // call plugin registration entry point
    //usRC = pFunc();     // Add return value for P402974
    usRC = registerPlugins();

    if (usRC) {
        // if register dll error, just remove the dll from the group
LogMessage5(ERROR,__func__, ":: Error: register plugin ", pszName, " failed ", intToA(usRC));

        // FOR P403268 begin
        // if it's already in pluginSet, also erase it
        // vLoadedPluginDLLs.pop_back();
        //LoadedPluginDLL lpdll = vLoadedPluginDLLs.back();
        //OtmPlugin *pToDel = lpdll.vPluginList.back();

        //if(pToDel != NULL)
        //  pluginSet->erase(pToDel);

        //vLoadedPluginDLLs.pop_back();
        // FOR P403268 end

      #ifdef TO_BE_REMOVED
        dlclose(handle);
        #endif
    }

    // reset active plugin DLL entry
    iCurrentlyLoadedPluginDLL = -1;

LogMessage2(ERROR,__func__, ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 25 HMODULE hMod = LoadLibrary(pszName);");
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
    // Add end
	//HMODULE hMod = LoadLibrary(pszName);
    //SetErrorMode(0);
    this->Log.writef( "   Loading plugin %s", pszName );

	if (hMod != 0)
	{
    //this->Log.writef( "   Plugin %s successfully loaded", pszName );
		PLUGINPROC pFunc = (PLUGINPROC) GetProcAddress(hMod, "registerPlugins");
		if (pFunc != 0)
		{
      //this->Log.write( "   Address of function \"registerPlugins()\" successfully resolved" );

      // prepare our loaded DLL entry
      vLoadedPluginDLLs.push_back( LoadedPluginDLL() );
      iCurrentlyLoadedPluginDLL = vLoadedPluginDLLs.size() - 1;
      vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].hMod = hMod;
      // Add for P402792 start
      memset(vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].strDll, 0x00, sizeof(vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].strDll));
      strcpy(vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].strDll, pszName);
      // Add end

      // call plugin registration entry point
      usRC = pFunc();     // Add return value for P402974
      // Add for P403115 start
      if (usRC)
      {
          // if register dll error, just remove the dll from the group
          this->Log.writef( "Error: register plugin %s failed %d.", pszName, usRC);
         
		  // FOR P403268 beigin
		  // if it's already in pluginSet, also erase it
		  // vLoadedPluginDLLs.pop_back();
		  LoadedPluginDLL lpdll = vLoadedPluginDLLs.back();
		  OtmPlugin *pToDel = lpdll.vPluginList.back();
		  if(pToDel != NULL)
			  pluginSet->erase(pToDel);
		  vLoadedPluginDLLs.pop_back();
		  // FOR P403268 end

          FreeLibrary(hMod);
      }
      // Add end

      // reset active plugin DLL entry
      iCurrentlyLoadedPluginDLL = -1;
		}
		else
		{
      this->Log.write( "   Could not resolve address of function \"registerPlugins()\"" );
			FreeLibrary(hMod);
		}
	}
	else
	{
    this->Log.writef( "   Plugin DLL %s failed to load", pszName );
	}
#endif

    return usRC;     // Add for P402974
}

int PluginManagerImpl::findPluginEntry( OtmPlugin *pPlugin )
{
  for ( int i = 0; i < (int)vLoadedPluginDLLs.size(); i++ )
  {
    for( std::vector<OtmPlugin*>::iterator itPlugin = vLoadedPluginDLLs[i].vPluginList.begin(); itPlugin != vLoadedPluginDLLs[i].vPluginList.end(); ++itPlugin )
    {
      if ( *itPlugin == pPlugin )
      {
        return( i );
      }
    }
  }
  return( -1 );
}


//This Method notifies all listeners
bool PluginManagerImpl::NotifyListeners( PluginListener::eNotifcationType eNotifcation, const char *pszPluginName, OtmPlugin::ePluginType eType, bool fForce )
{
  for( std::vector<PluginListener *>::iterator itListener = m_vPluginListener.begin(); itListener != m_vPluginListener.end(); ++itListener )
  {
      (*itListener)->Notify( eNotifcation, pszPluginName, eType, fForce );
  }
	return (m_vPluginListener.size() > 0);
}

static char* strupr(char *str) 
{ 
    char *tmp = str; 
    while (*tmp) { 
        *tmp = toupper((unsigned char)*tmp); 
        ++tmp; 
    } 
    return str; 
}

BOOL PluginManagerImpl::IsDepthOvered(const char * strPath)
{
    BOOL bOvered = FALSE;
    CHAR szPluginPath[ MAX_PATH ];
    UtlQueryString( QST_PLUGINPATH, szPluginPath, sizeof( szPluginPath ));
    UtlMakeEQFPath( szPluginPath, NULC, PLUGIN_PATH, NULL );

    string strCheckPath(strPath);
    if (!strcasecmp(strCheckPath.c_str(), szPluginPath))
    {
        return bOvered;
    }

    // convert to upper case
    strupr(szPluginPath);
    std::transform(strCheckPath.begin(), strCheckPath.end(), strCheckPath.begin(), [](unsigned char c) -> unsigned char { return std::toupper(c); });

    string::size_type nPos = strCheckPath.find(szPluginPath);
    if (string::npos == nPos)
    {
        return bOvered;
    }

    int nLevel = 2;
    nPos += strlen(szPluginPath) + 1; // plus 1 mean there is no backslash after PLUGINS directory
    while ((nPos < strCheckPath.length()) && (string::npos != (nPos = strCheckPath.find("\\", nPos))))
    {
        nLevel++;
        nPos++;
    }

    if (nLevel > PLUGIN_PATH_MAX_DEPTH)
    {
        bOvered = TRUE;
    }

    return bOvered;
}

