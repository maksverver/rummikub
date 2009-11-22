/* A strtok_r implementation. Useful on Windows. */

#include <stdlib.h>
#include <string.h>

char *strtok_r(char *s, const char *sep, char **p)
{
    if (s == NULL)
    {
        if (*p == NULL) return NULL;
        s = *p + 1;
    }

    /* Skip leading delimiters */
    while (*s != '\0' && strchr(sep, *s) != NULL) ++s;
    if (*s == '\0') return *p = NULL;  /* don't return empty tokens */

    /* Find terminating delimiter */
    *p = s + 1;
    while (**p != '\0' && strchr(sep, **p) == NULL) ++*p;
    if (**p == '\0') *p = NULL; else **p = '\0';
    return s;
}
