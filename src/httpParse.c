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

#define HTTP_REQUEST_LINE_MAX_LENGTH 2047

/* Returns the next character to be read, without changing the file pointer. */
static int seek_file(FILE *f)
{
        int ch = fgetc(f);

        if (ch != EOF)
                ungetc(ch, f);

        return ch;
}

/* Reads the Request-Line of the start of the file, into a string, whether
 * the content of the fields are correct or not. That is, until CRLF.
 *
 * @param               f       The file to be read.
 * @param  [out]        pRes    The allocated string.
 * @except                      ERROR_REQUEST_INCOMPLETE
 * @except                      ERROR_INTERNAL
 * @except                      ERROR_REQUEST_TOO_LONG
 */
static enum error read_request_line(FILE *restrict f, char **restrict pRes)
{
        size_t reqLnLen = 0;
        char *reqLn = malloc(reqLnLen + 1);
        if (reqLn == NULL)
                return ERROR_INTERNAL;

        while (1) {
                int ch = fgetc(f);

                if (ch == EOF) {
                        free(reqLn);
                        return ferror(f)
                               ? ERROR_REQUEST_INCOMPLETE
                               : ERROR_INTERNAL;
                } if (reqLnLen == HTTP_REQUEST_LINE_MAX_LENGTH) {
                        free(reqLn);
                        return ERROR_REQUEST_TOO_LONG;
                } if (ch == '\r' && seek_file(f) == '\n')
                        break;

                char *tmp = realloc(reqLn, reqLnLen + 2);
                if (tmp == NULL)
                        return ERROR_INTERNAL;
                reqLn = tmp;

                reqLn[reqLnLen] = (char)ch;
                reqLnLen++;
        }
        reqLn[reqLnLen] = 0;

        *pRes = reqLn;
        return ERROR_SUCCESS;
}

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
 * Returns the major and minor versions.
 *
 * @except ERROR_BAD_REQUEST
 */
static enum error parse_http_version(const char *restrict vers,
                                     unsigned int *restrict pMajor,
                                     unsigned int *restrict pMinor)
{
        const char prefix[] = "HTTP/";
        const size_t prefixLen = sizeof(prefix) - 1;

        if (strncmp(vers, prefix, 5) != 0)
                return ERROR_BAD_REQUEST;

        char *pPeriod = strchr(vers, '.');
        if (pPeriod == NULL)
                return ERROR_BAD_REQUEST;

        char *next;
        errno = 0;
        unsigned long ulMajor = strtoul(vers + prefixLen, &next, 10);
        if (errno)
                return ERROR_BAD_REQUEST;

        if (next != pPeriod || next == vers)
                return ERROR_BAD_REQUEST;

        unsigned long ulMinor = strtoul(pPeriod + 1, &next, 10);
        if (errno)
                return ERROR_BAD_REQUEST;

        if (*next != 0 || next == pPeriod + 1)
                return ERROR_BAD_REQUEST;

        if (ulMajor >= ULONG_MAX || ulMinor >= ULONG_MAX)
                return ERROR_BAD_REQUEST;

        *pMajor = (unsigned int)ulMajor;
        *pMinor = (unsigned int)ulMinor;
        return ERROR_SUCCESS;
}

/* Pares the Request-Line string into 're', modifying 'reqLine' pointing to its
 * content.
 *
 * @except ERROR_BAD_REQUEST
 */
static enum error parse_request_line_str(char *restrict reqLine,
        struct http_request_line *restrict pRes)
{
        char *sp1 = strchr(reqLine, ' ');
        if (sp1 == NULL)
                return ERROR_BAD_REQUEST;
        char *sp2 = strchr(sp1 + 1, ' ');
        if (sp2 == NULL)
                return ERROR_BAD_REQUEST;
        *sp1 = 0;
        *sp2 = 0;

        if (!is_valid_method(reqLine))
                return ERROR_BAD_REQUEST;
        if (!is_valid_uri(sp1 + 1))
                return ERROR_BAD_REQUEST;

        unsigned int major, minor;
        enum error err = parse_http_version(sp2 + 1, &major, &minor);
        if (err)
                return err;

        pRes->method = reqLine;
        pRes->uri = sp1 + 1;
        pRes->http_version.major = major;
        pRes->http_version.minor = minor;

        return ERROR_SUCCESS;
}

enum error parse_http_request_line(const char *restrict fName,
        struct http_request_line *restrict pRes)
{
        if (fName == NULL || pRes == NULL)
                return ERROR_INVALID_PARAMETER;

        FILE *f = fopen(fName, "rb");
        if (!f)
                return ERROR_INTERNAL;

        char *lnStr;
        enum error err = read_request_line(f, &lnStr);
        fclose(f);
        if (err)
                return err;

        err = parse_request_line_str(lnStr, pRes);
        if (err) {
                free(lnStr);
                return err;
        }

        return ERROR_SUCCESS;
}

enum error destroy_http_request_line(struct http_request_line *pReqLine):
{
        if (pReqLine == NULL)
                return ERROR_INVALID_PARAMETER;

        free(pReqLine->method);
        return ERROR_SUCCESS;
}
