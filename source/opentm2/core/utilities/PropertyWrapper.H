#ifndef _PROPERTY_WRAPPER_H__
#define _PROPERTY_WRAPPER_H__

extern "C" {
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
