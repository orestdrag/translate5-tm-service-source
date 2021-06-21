
#include "SystemFunctionsWrapper.h"
#include <map>

//TODO replace with proper registry implementation
std::map<std::string, std::string> stringRegistry;
std::map<std::string, int> intRegistry;

bool SystemFunctionsWrapper::GetStringFromRegistry( std::string sAppl, std::string sKey, std::string& sResult){

    #ifdef _WIN32
    HKEY hKey = NULL;
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
    {
        HKEY hSubKey = NULL;
        if ( RegOpenKeyEx( hKey, pszAppl, 0, KEY_ALL_ACCESS, &hSubKey ) == ERROR_SUCCESS )
        {
        DWORD dwType = REG_SZ;
        DWORD iSize = iBufSize;
        int iSuccess = RegQueryValueEx( hSubKey, pszKey, 0, &dwType, (LPBYTE)pszBuffer, &iSize );
        fOK = (iSuccess == ERROR_SUCCESS);
        RegCloseKey(hSubKey);
        } /* endif */        
        RegCloseKey( hKey );
    } /* endif */
    return fOK;
    #else //linux
    std::string name = sAppl + sKey;

    if(stringRegistry.find(name) != stringRegistry.end()){
        sResult = stringRegistry[name];
        return true;
    }
    return false;

    #endif     
}

bool SystemFunctionsWrapper::GetIntFromRegistry(std::string sAppl, std::string sKey, int& iResult){
    #ifdef _WIN32
    HKEY hKey = NULL;
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
    {
        HKEY hSubKey = NULL;
        if ( RegOpenKeyEx( hKey, pszAppl, 0, KEY_ALL_ACCESS, &hSubKey ) == ERROR_SUCCESS )
        {
        DWORD dwType = REG_SZ;
        char szBuffer[20];
        DWORD iSize = sizeof(szBuffer);
        int iSuccess = RegQueryValueEx( hSubKey, pszKey, 0, &dwType, (LPBYTE)szBuffer, &iSize );
        if ( iSuccess == ERROR_SUCCESS )
        {
            if ( dwType == REG_DWORD )
            {
            fOK = TRUE;
            iResult = *((int *)szBuffer);
            } 
            else if ( dwType == REG_SZ )
            {
            fOK = TRUE;
            iResult = atol( szBuffer );
            } /* endif */           
        } /* endif */         
        RegCloseKey(hSubKey);
        } /* endif */        
        RegCloseKey( hKey );
    } /* endif */     

    #else
    std::string name = sAppl + sKey;

    if(intRegistry.find(name) != intRegistry.end()){
        iResult = intRegistry[name];
        return true;
    }
    return false;
    #endif
}

bool SystemFunctionsWrapper::WriteStringToRegistry(std::string sAppl, std::string sKey, std::string sValue){
    #ifdef _WIN32
    HKEY hKey = NULL;
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
    {
        HKEY hSubKey = NULL;
        int iSuccess = RegOpenKeyEx( hKey, pszAppl, 0, KEY_ALL_ACCESS, &hSubKey );
        if ( iSuccess != ERROR_SUCCESS )
        {
            DWORD dwDisp = 0;
            iSuccess = RegCreateKeyEx( hKey, pszAppl, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisp );
        } /* endif */       

        if( iSuccess == ERROR_SUCCESS )
        {
            RegSetValueEx( hSubKey, pszKey, 0, REG_SZ, (LPBYTE)pszValue , strlen(pszValue )+ 1);
            RegCloseKey(hSubKey);
            fOK = TRUE;
        } /* endif */        

        RegCloseKey( hKey );
    } /* endif */

    #else //linux

    std::string name = sAppl + sKey;

    if(stringRegistry.find(name) != stringRegistry.end()){// we already have that key
        stringRegistry[name] = sValue;
        return true;
    }else{
        stringRegistry[name] = sValue;
        return true;
    }

    #endif
}

bool SystemFunctionsWrapper::WriteIntToRegistry( std::string sAppl, std::string sKey, int iValue ){
    #ifdef _WIN32
    
    HKEY hKey = NULL;
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
    {
        HKEY hSubKey = NULL;
        int iSuccess = RegOpenKeyEx( hKey, pszAppl, 0, KEY_ALL_ACCESS, &hSubKey );
        if ( iSuccess != ERROR_SUCCESS )
        {
            DWORD dwDisp = 0;
            iSuccess = RegCreateKeyEx( hKey, pszAppl, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisp );
        } /* endif */       

        if( iSuccess == ERROR_SUCCESS )
        {
            RegSetValueEx( hSubKey, pszKey, 0, REG_DWORD, (LPBYTE)&iValue , sizeof(DWORD) );
            RegCloseKey(hSubKey);
            fOK = TRUE;
        } /* endif */        

        RegCloseKey( hKey );
    } /* endif */
    #else
    std::string name = sAppl + sKey;

    if(intRegistry.find(name) != intRegistry.end()){// we already have that key
        intRegistry[name] = iValue;
        return true;
    }else{
        intRegistry[name] = iValue;
        return true;
    }
    #endif 
}

