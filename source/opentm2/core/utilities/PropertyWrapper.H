#ifndef _PROPERTY_WRAPPER_H__
#define _PROPERTY_WRAPPER_H__

extern "C" {

    enum PROPERTY_ERRORS {
        PROPERTY_NO_ERRORS = 0,

        PROPERTY_ERROR_FILE_CANT_OPEN,
        PROPERTY_ERROR_FILE_STRINGPROPERTIES_NOT_FOUND,
        PROPERTY_ERROR_FILE_INTPROPERTIES_NOT_FOUND,
        PROPERTY_ERROR_FILE_CANT_CREATE_OTM_DIRECTORY,
        PROPERTY_ERROR_FILE_CANT_CREATE_FILE,
        PROPERTY_ERROR_FILE_CANT_GET_HOME_DIR,
        PROPERTY_ERROR_FILE_CANT_GET_USER_PSWD,
        PROPERTY_ERROR_FILE_KEY_NOT_FOUND,

        PROPERTY_ERROR_STR_KEY_NOT_EXISTS,
        PROPERTY_ERROR_STR_KEY_ALREADY_EXISTS,
        
        PROPERTY_ERROR_INT_KEY_NOT_EXISTS,
        PROPERTY_ERROR_INT_KEY_ALREADY_EXISTS
    };

    int properties_init();
    void properties_deinit();
    
    int properties_add_str(const char *key, const char *value);
    int properties_add_int(const char *key, const int   value);
    
    int properties_set_str(const char *key, const char *value);
    int properties_set_int(const char *key, const int   value);

    bool properties_exist (const char *key);
    int properties_get_str(const char *key, char* buff, int buffSize);
    int properties_get_int(const char *key, int& value);
    

    char * properties_get_otm_dir();
}

#endif // _PROPERTY_WRAPPER_H__
