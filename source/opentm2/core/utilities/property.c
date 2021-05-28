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

static FILE *fp;
static char *home_dir;
static char *otm_dir;
static char *property_file;

static char * get_home_dir() {
    char *home_dir = getenv(HOME_ENV);
    if (home_dir)
        return home_dir;

    home_dir = getpwuid(getuid())->pw_dir;
    return home_dir;
}

static int create_otm_dir() {
    const char *home_dir = get_home_dir();
    if (!home_dir)
        return -1;

    int res = snprintf(otm_dir, PATH_MAX, "%s/.%s", home_dir, OTMMEMORYSERVICE);

    struct stat st;
    res = stat(otm_dir, &st);
    if (!res)
        return 0;

    res = mkdir(otm_dir, 0700);
    return res;
}

static int open_file() {
    fp = fopen(property_file, "w+");
    if (!fp)
        return -1;

    return 0;
}

static int close_file() {
    int ret = close(fp);
    if (ret == EOF)
        return -1;

    return 0;
}

static int write2file(const char *key, const char *value) {
    fprintf(fp, "%s=%s\n", key, value);
}

int property_add_key(const char *key, const char *value) {
    int ret = open_file();
    if (ret)
        return -1;

    write2file(key, value);
    close_file();

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
    int ret = create_otm_dir();
    if (ret)
        return -1;

    char property_file[PATH_MAX];
    snprintf(property_file, PATH_MAX, "%s/%s", otm_dir, "Properties");

    return 0;
}
