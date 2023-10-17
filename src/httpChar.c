/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/05
 */

/* This file implements the declarations on "httpChar.h".
 */

#include "httpChar.h"

static bool is_CHAR(char ch)
{
        return ch >= 0;
}

static bool is_CTL(char ch)
{
        return (ch >= 0 && ch <= 31) || ch == 127;
}

static bool is_tspecials(char ch)
{
        return  ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '@'
        ||      ch == ',' || ch == ';' || ch == ':' || ch == '\\' || ch == '\"'
        ||      ch == '/' || ch == '[' || ch == ']' || ch == '?' || ch == '='
        ||      ch == '{' || ch == '}' || ch == ' ' || ch == '\t';
}

/* Checks whether an array starts with 'token'.
 *
 * @param [in]          arr     The array to be checked.
 * @param               arrLen  The length of the array.
 * @param [out, opt]    pLen    The length of the 'token', if any.
 * @return                      Whether the array starts with 'token'.
 */
static bool starts_with_token(const char *restrict arr, size_t arrLen,
                                                        size_t *restrict pLen)
{
        size_t len;
        for (len = 0; len < arrLen; len++) {
                if (    !is_CHAR(arr[len]) || is_CTL(arr[len])
                ||      is_tspecials(arr[len])) {
                        break;
                }
        }

        if (arrLen == 0)
                return false;

        if (pLen)
                *pLen = len;
        return true;
}

bool is_http_method(const char *arr, size_t arrLen)
{
        size_t tokenLen;
        if (!starts_with_token(arr, arrLen, &tokenLen))
                return false;

        return tokenLen == arrLen;
}
