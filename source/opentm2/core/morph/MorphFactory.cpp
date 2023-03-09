/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "../pluginmanager/PluginManager.h"
#include "OtmMorphPlugin.h"
#include "OtmMorph.h"
#include "OtmMorphICUPlugin.h"
#include "MorphFactory.h"
#include "LogWrapper.h"

/*! \brief the MorphFactory instance	 */
MorphFactory* MorphFactory::pMorphInstance = NULL;

/*! \brief constructor	 */
MorphFactory::MorphFactory(void)
{
	pvPluginList = NULL;
}

/*! \brief destructor	 */
MorphFactory::~MorphFactory(void)
{
	if (NULL != pvPluginList)
	{
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			delete pvPluginList->at(i);
		}
		delete pvPluginList;
		pvPluginList = NULL;
	}
}

/*! \brief This static method returns a pointer to the MorphFactory object.
	The first call of the method creates the MorphFactory instance.
*/
MorphFactory* MorphFactory::getInstance()
{
	if (NULL == pMorphInstance)
	{
		pMorphInstance = new MorphFactory();
		pMorphInstance->refreshPluginList();

		T5LOG(T5DEVELOP) << "Get instance of MorphFactory!";
	}
	return pMorphInstance;
}

/*! \brief refresh the morphology plug-in list */
void MorphFactory::refreshPluginList()
{
	T5LOG(T5DEVELOP) << "Refresh plugin list";
	if (NULL != pvPluginList)
	{
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			delete pvPluginList->at(i);
		}
		delete pvPluginList;
		T5LOG(T5DEBUG) <<"Delete plugin list";
	}

	// access plugin manager
	PluginManager* pPluginManager = PluginManager::getInstance();

	OtmMorphPlugin * pOtmPlugin = NULL;
	pvPluginList = new std::vector<OtmMorphPlugin *>;
	int i = 0;
	do
	{
		i++;
		pOtmPlugin = (OtmMorphPlugin*) pPluginManager->getPlugin(OtmPlugin::eMorphType, i );
		if ( pOtmPlugin != NULL ) pvPluginList->push_back( pOtmPlugin );
	}  while ( pOtmPlugin != NULL ); /* end */     
}

OtmMorphPlugin* MorphFactory::getPlugin( const char* pszPluginName )
{
	T5LOG(T5DEVELOP) << "MorphFactory::getPlugin(): First, get plug-in " << pszPluginName;

	if(NULL != pszPluginName)
	{
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			if (0 == strcmp(pszPluginName, pvPluginList->at(i)->getName()))
			{
				T5LOG(T5DEVELOP) <<"MorphFactory::getPlugin(): Success, get plug-in "<< pszPluginName<< " at " << i;
				return pvPluginList->at(i);
			}
		}
	}

	if (0 < pvPluginList->size())
	{
		T5LOG(T5DEVELOP) <<"MorphFactory::getPlugin(): Success, get the similar plug-in of " << pszPluginName<< " at 0 ";
		return pvPluginList->at(0);
	}

	T5LOG(T5ERROR) <<"MorphFactory::getPlugin(): Error occured when getting plug-in" << pszPluginName;
	return NULL;
}

/* \brief get a Morph checker object
   \param vName the language name
   \param vPluginName name of the plug-in
   \returns pointer to Morph object 
*/
OtmMorph* MorphFactory::getMorph(const char* pszName, const char* pszPluginName)
{
	//OtmMorphICUPlugin* pOtmMorphPlugin = OtmMorphICUPlugin::GetInstance();
	return OtmMorphICUPlugin::GetInstance().openMorph(pszName);
}

/*! \brief get the language support list of the specific plugin, if the plugin name is null, the result will contain all the languages
  \ that is supported by the MorphFactory class.
  \param vPluginName pointer to the name of the plugin
  \param vLanguageList reference to the vector that contains all the languages supported by the MorphFactory class
  \returns 0 means success, other value means error!
*/
int MorphFactory::getLanguageList(
	const char* pszPluginName, 
	vector<string>& vLanguageList
	)
{
	if (NULL == pszPluginName || (strcmp(pszPluginName, "") == 0))
	{
		T5LOG(T5DEBUG) <<"get all the languages supported by the MorphFactory class!";
		set<string> setLanguageList;
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			OtmMorphPlugin* pOtmMorphPlugin = pvPluginList->at(i);
			vector<string> vSpecficLanguageList;
			if (0 == pOtmMorphPlugin->getLanguageList(vSpecficLanguageList))
			{
				for (int j = 0; j < vSpecficLanguageList.size(); j++)
				{
					setLanguageList.insert(vSpecficLanguageList[j]);
				}
			}
		}

		for (set<string>::iterator tIter = setLanguageList.begin(); tIter != setLanguageList.end(); tIter++)
		{
			vLanguageList.push_back(*tIter);
		}
	}
	else
	{
		OtmMorphPlugin* pOtmMorphPlugin = getPlugin(pszPluginName);
		if (NULL != pOtmMorphPlugin)
		{
			return pOtmMorphPlugin->getLanguageList(vLanguageList);
		}
	}
}

/* \brief check if the given language is supported by any of the available Morph plugins
   \param vName the language name
   \returns TRUE if the language is supported and FALS when no plugin for this language is available
*/
bool MorphFactory::isSupported(const char* pszName )
{
  if((int)pvPluginList->size() == 0){
	T5LOG( T5WARNING) << "MorphFactory::isSupported :: pvPluginList size == 0";
	return true;//temporary
  }

  for (int i = 0; i < (int)pvPluginList->size(); i++)
	{
	  OtmMorphPlugin* pOtmMorphPlugin = pvPluginList->at(i);
  	if ( pOtmMorphPlugin->isSupported( pszName ) )
    {
	  T5LOG(T5DEBUG) <<"MorphFactory::isSupported(): The language "<<pszName<<" is supported";
      return( TRUE );
    }
  }

  T5LOG(T5DEBUG) <<"MorphFactory::isSupported(): The language "<<pszName<<" is not supported";
	return FALSE;
}

/* \brief get Morph info of the specified plug-in.
   \param vPluginName name of the plug-in
   \param vName the language name
   \param vInfo pointer to the Morph info struct that restores Morph info.
   \returns ERROR_PARAMETAR means error, 0 means success.
*/
int MorphFactory::getMorphInfo(const char* pszName, OtmMorphPlugin::PMORPHINFO pInfo, const char* pszPluginName)
{
	T5LOG(T5DEBUG) <<"MorphFactory::getMorphInfo(): First, get morphology info of "<<pszPluginName<<" plug-in.";
	OtmMorphPlugin* pOtmMorphPlugin = getPlugin(pszPluginName);
	if (NULL != pOtmMorphPlugin)
	{
		T5LOG(T5DEBUG) <<"MorphFactory::getMorphInfo(): Success, get morphology info of "<<pszPluginName<<" plug-in.";
		return pOtmMorphPlugin->getMorphInfo(pszName, pInfo);
	}

	T5LOG(T5DEBUG) <<"MorphFactory::getMorphInfo(): Error, get morphology info of "<<pszPluginName<<" plug-in failed.";
	return OtmMorph::ERROR_PARAMETAR;
}

/*! \brief Delete the MorphFactory instance if it exists.
*/
void MorphFactory::close()
{
	if (NULL != pMorphInstance)
	{
		delete pMorphInstance;
		pMorphInstance = NULL;
	}
}

