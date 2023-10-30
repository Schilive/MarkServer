/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/30
 */

/* This file implements the functions on "util.c".
 */

#include "util.h"

#include "error.h"
#include <string.h>

enum error append_character(char **pStr, char ch)
{
        size_t strLen = *pStr ? strlen(*pStr) : 0;

        char *newStr = realloc(*pStr, strLen + 2);
        if (newStr == NULL)
                return ERROR_MEMORY_ALLOCATION;
        *pStr = newStr;

        newStr[strLen] = ch;
        newStr[strLen + 1] = 0;

        return ERROR_SUCCESS;
}

enum error append_array(char **pStr, const char *arr, size_t arrLen)
{
        size_t strLen = *pStr ? strlen(*pStr) : 0;

        char *newStr = realloc(*pStr, strLen + arrLen + 1);
        if (newStr == NULL)
                return ERROR_MEMORY_ALLOCATION;
        *pStr = newStr;

        memcpy(newStr + strLen, arr, arrLen);
        newStr[strLen + arrLen] = 0;
        return ERROR_SUCCESS;
}

enum error append_string(char **pStr1, const char *str2)
{
        size_t str1Len = *pStr1 ? strlen(*pStr1) : 0;
        size_t str2Len = str2 ? strlen(str2) : 0;

        char *newStr = realloc(*pStr1, str1Len + str2Len + 1);
        if (newStr == NULL)
                return ERROR_MEMORY_ALLOCATION;
        *pStr1 = newStr;

        memcpy(newStr + str1Len, str2, str2Len);
        newStr[str1Len + str2Len] = 0;

        return ERROR_SUCCESS;
}
