/* Manage properties on Linux replacing usage of Windows system calls, OS2.INI */

#ifndef _PROPERTY_H_
#define _PROPERTY_H_

int property_init();
int property_add_key(const char *key, const char *value);
int property_set_value(const char *key, const char *value);
char * property_get_value(const char *key);

#endif // _PROPERTY_H_
