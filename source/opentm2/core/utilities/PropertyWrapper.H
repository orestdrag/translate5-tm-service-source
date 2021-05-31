#ifndef _PROPERTY_WRAPPER_H__
#define _PROPERTY_WRAPPER_H__

extern "C" {
    int properties_init();
    void properties_deinit();
    int properties_add_key(const char *key, const char *value);
    int properties_set_value(const char *key, const char *value);
}

#endif // _PROPERTY_WRAPPER_H__
