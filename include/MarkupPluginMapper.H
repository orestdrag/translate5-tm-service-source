//+----------------------------------------------------------------------------+
//|  MarkupPluginMapper.H - Markup table plugin mapper functions               |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#ifndef _MARKUPPLUGINMAPPER_H_
#define _MARKUPPLUGINMAPPER_H_

#include "vector"
#include "../source/opentm2/core/pluginmanager/PluginManager.h"
#include "../source/opentm2/core/pluginmanager/OtmMarkup.h"
#include "../source/opentm2/core/pluginmanager/OtmMarkupPlugin.h"

// find a markup table object by markup table name

 OtmMarkup *GetMarkupObject( char *pszMarkup, char *pszPlugin );

// find a markup table plugin object by plugin name

 OtmMarkupPlugin *GetMarkupPluginObject( char *pszPlugin );

// get the markup table plugin providing a specific markup table 

OtmMarkupPlugin *GetMarkupPlugin( char *pszMarkupName );

#endif
