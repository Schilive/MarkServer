/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/05
 */

/* This file implements the declarations on "httpParse.h".
 */

#include "httpParse.h"

#include <string.h>
#include <stdbool.h>
#include "httpChar.h"
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

/* Returns whether the method format is correct. */
static bool is_valid_method(const char *str)
{
        return is_http_method(str, strlen(str));
}

/* Returns whether the URI format is correct. */
static bool is_valid_uri(const char *str)
{
        return is_http_requestURI(str, strlen(str));
}

/* Parses the HTTP version string, verifying that its format is correct.
 * Returns the major and minor versions. It changes the content.
 *
 * @except ERROR_BAD_REQUEST
 */
static enum error parse_http_version(const char *restrict vers,
        struct http_version *restrict pRes)
{
        const char prefix[] = "HTTP/";
        const size_t prefixLen = sizeof(prefix) - 1;

        if (strncmp(vers, prefix, prefixLen) != 0)
                return ERROR_BAD_REQUEST;

        char *pPeriod = strchr(vers, '.');
        if (pPeriod == NULL)
                return ERROR_BAD_REQUEST;

        char *next;
        unsigned long ulMajor = strtoul(vers + prefixLen, &next, 10);
        if (next != pPeriod || next == vers + prefixLen)
                return ERROR_BAD_REQUEST;

        unsigned long ulMinor = strtoul(pPeriod + 1, &next, 10);
        if (*next != '\r' || next == pPeriod + 1)
                return ERROR_BAD_REQUEST;

        if (ulMajor > UINT_MAX || ulMinor > UINT_MAX)
                return ERROR_REQUEST_TOO_LONG;
        pRes->major = (unsigned int)ulMajor;
        pRes->minor = (unsigned int)ulMinor;

        return ERROR_SUCCESS;
}

/* Parses the Request-Line string, modifying 'reqLn' content.
 *
 * @except ERROR_BAD_REQUEST
 * @except ERROR_REQUEST_INCOMPLETE
 */
static enum error parse_request_line_str(char *restrict reqLn,
        struct http_request_line *restrict pRes)
{
        char *end = strstr(reqLn, "\r\n");
        if (end == NULL)
                return ERROR_REQUEST_INCOMPLETE;

        char *sp1 = strchr(reqLn, ' ');
        if (sp1 == NULL)
                return ERROR_BAD_REQUEST;
        char *sp2 = strchr(sp1 + 1, ' ');
        if (sp2 == NULL)
                return ERROR_BAD_REQUEST;
        *sp1 = 0;
        *sp2 = 0;

        if (!is_valid_method(reqLn))
                return ERROR_BAD_REQUEST;
        if (!is_valid_uri(sp1 + 1))
                return ERROR_BAD_REQUEST;
        struct http_version ver;
        enum error err = parse_http_version(sp2 + 1, &ver);
        if (err)
                return err;

        pRes->method = reqLn;
        pRes->uri = sp1 + 1;
        pRes->http_version = ver;

        return ERROR_SUCCESS;
}

enum error parse_http_request_line(char *restrict reqLn,
        struct http_request_line *restrict pRes)
{
        if (reqLn == NULL || pRes == NULL)
                return ERROR_INVALID_PARAMETER;

        enum error err = parse_request_line_str(reqLn, pRes);
        return err;
}
