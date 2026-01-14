#include "local_dirent.h"

#if 0
DIR *opendir(const char *name)
{
    printf("dirnaem: %s\n", name);
    return NULL;
}


struct dirent *readdir(DIR *d)
{
    struct dirent *result = NULL;
    return result;
}


int closedir(DIR *d)
{
    int result = 0;
    return result;
}

int remove(const char *name)
{
    printf("remove: %s\n", name);
}

int rename(const char *old_filename, const char *new_filename)
{
    printf("oldname:%s, newname:%s\n", old_filename, new_filename);
    return 0;
}
#endif
