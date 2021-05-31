#include "property.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define HOME_ENV "HOME"
#define OTMMEMORYSERVICE "OtmMemoryService"

static char property_file[PATH_MAX];

static char * get_home_dir() {
    char *home_dir = getenv(HOME_ENV);
    if (home_dir)
        return home_dir;

    home_dir = getpwuid(getuid())->pw_dir;
    return home_dir;
}

static int create_otm_dir(char *otm_dir) {
    const char *home_dir = get_home_dir();
    if (!home_dir)
        return -1;

    snprintf(otm_dir, PATH_MAX, "%s/.%s", home_dir, OTMMEMORYSERVICE);

    struct stat st;
    int ret = stat(otm_dir, &st);
    if (!ret)
        return 0;

    return mkdir(otm_dir, 0700);
}

static int write2file(FILE *fp, const char *key, const char *value) {
    fprintf(fp, "%s=%s\n", key, value);
}

static FILE * open_properties(char *buf) {
    FILE *fp = fopen(property_file, "a+");
    if (!fp)
        return NULL;

    fscanf(fp, "%s", buf);
    return fp;
}

int property_add_key(const char *key, const char *value) {
    char *buf;
    FILE *fp = open_properties(buf);
    if (!fp)
        return -1;

    int rv = 0;
    char *ret;
    if (buf && (ret = strstr(buf, key)))
        rv = -1;
    else
        write2file(fp, key, value);

    free(buf);
    fclose(fp);

    return rv;
}

static char * remove_substr(char *str, const char *sub) {
    char *p, *q, *r;

    q = r = strstr(str, sub);
    if (!q)
        return NULL;

    int len = strlen(sub);
    while ((r = strstr(p = r + len, sub)) != NULL) {
        while (p < r)
            *q++ = *p++;
    }

    while ((*q++ = *p++) != '\0')
        continue;

    return str;
}

int property_set_value(const char *key, const char *value) {
    char *buf;
    FILE *fp = open_properties(buf);
    if (!fp)
        return -1;

    int rv = 0;
    char *ret = strstr(buf, key);

    if (ret) {
        //remove_substr(ret, );
        write2file(fp, key, value);
    }
    else {
        rv = -1;
    }

    free(buf);
    fclose(fp);

    return rv;
}

char * property_get_value(const char *key) {
    return NULL;
}

int property_init() {
    char otm_dir[PATH_MAX];
    int ret = create_otm_dir(otm_dir);
    if (ret)
        return -1;

    snprintf(property_file, PATH_MAX, "%s/%s", otm_dir, "Properties");

    return 0;
}
