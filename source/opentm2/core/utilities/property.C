#include "property.h"

#include <stdlib.h>
#include <stdio.h>
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

int property_add_key(const char *key, const char *value) {
    FILE *fp = fopen(property_file, "w+");
    if (!fp)
        return -1;

    write2file(fp, key, value);
    fclose(fp);

    return 0;
}

int property_set_value(const char *key, const char *value) {
    return 0;
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
