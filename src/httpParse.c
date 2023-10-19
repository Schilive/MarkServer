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

#define HTTP_VERSION_TOKEN_MAX_LENGTH 63
#define HTTP_REQUEST_LINE_MAX_LENGTH (HTTP_METHOD_MAX_LENGTH + 1 + \
        HTTP_URI_MAX_LENGTH + 1 + HTTP_VERSION_TOKEN_MAX_LENGTH)

struct request_line_tokens {
    char method[HTTP_METHOD_MAX_LENGTH + 1];
    char uri[HTTP_URI_MAX_LENGTH + 1];
    char version[HTTP_VERSION_TOKEN_MAX_LENGTH + 1];
};

/* Returns the next character to be read, without changing the file pointer. */
static int seek_file(FILE *f)
{
        int ch = fgetc(f);

        if (ch != EOF)
                ungetc(ch, f);

        return ch;
}

/* Returns the Request-Line of the start of the file, as a string, whether
 * the character of the fields are correct or not. The string is assumed to be
 * of the size 'HTTP_REQUEST_LINE_MAX_LENGTH + 1' at minimal.
 *
 * @except      ERROR_REQUEST_INCOMPLETE
 * @except      ERROR_INTERNAL
 * @except      ERROR_REQUEST_TOO_LONG
 */
static enum error read_request_line(FILE *restrict f, char *restrict reqLineStr)
{
        char reqLineTxt[HTTP_REQUEST_LINE_MAX_LENGTH];
        size_t reqLineLen = 0;

        while (1) {
                int ch = fgetc(f);

                if (ch == EOF)
                        return  ferror(f)
                                ? ERROR_REQUEST_INCOMPLETE
                                : ERROR_INTERNAL;
                if (reqLineLen == HTTP_REQUEST_LINE_MAX_LENGTH)
                        return ERROR_REQUEST_TOO_LONG;
                if (ch == '\r' && seek_file(f) == '\n')
                        break;

                reqLineTxt[reqLineLen] = (char)ch;
                reqLineLen++;
        }

        memcpy(reqLineStr, reqLineTxt, reqLineLen);
        reqLineStr[reqLineLen] = 0;

        return ERROR_SUCCESS;
}

/* Tokenizes the Request-Line string.
 *
 * @except ERROR_BAD_REQUEST
 */
static enum error tokenize_request_line_str(const char *restrict reqLine,
        struct request_line_tokens *restrict pReqLineToks)
{
        char *sp1 = strchr(reqLine, ' ');
        if (!sp1)
                return ERROR_BAD_REQUEST;
        char *sp2 = strchr(sp1 + 1, ' ');
        if (!sp2)
                return ERROR_BAD_REQUEST;

        size_t methodLen = sp1 - reqLine;
        size_t uriLen = sp2 - (sp1 + 1);
        size_t versLen = strlen(sp2 + 1);

        memcpy(pReqLineToks->method, reqLine, methodLen);
        memcpy(pReqLineToks->uri, sp1 + 1, uriLen);
        memcpy(pReqLineToks->version, sp2 + 1, versLen);

        pReqLineToks->method[methodLen] = 0;
        pReqLineToks->uri[uriLen] = 0;
        pReqLineToks->version[versLen] = 0;

        return ERROR_SUCCESS;
}

/* Returns whether the method format is correct. */
static bool method_is_valid(const char *str)
{
        return is_http_method(str, strlen(str));
}

/* Returns whether the URI format is correct. */
static bool uri_is_valid(const char *str)
{
        return is_http_requestURI(str, strlen(str));
}

/* Parses the HTTP version string, verifying that its format is correct.
 * Returns the major and minor versions.
 *
 * @except ERROR_BAD_REQUEST
 */
static enum error parse_http_version(const char *vers,
                          unsigned long *restrict pMajor,
                          unsigned long *restrict pMinor)
{
        char prefix[] = "HTTP/";
        size_t prefixLen = sizeof(prefix) - 1;

        if (strncmp(vers, prefix, 5) != 0)
                return ERROR_BAD_REQUEST;

        char *pPeriod = strchr(vers, '.');
        if (!pPeriod)
                return ERROR_BAD_REQUEST;

        char *next;
        errno = 0;
        unsigned long major = strtoul(vers + prefixLen, &next, 10);
        if (errno)
                return ERROR_BAD_REQUEST;

        if (next != pPeriod || next == vers)
                return ERROR_BAD_REQUEST;

        unsigned long minor = strtoul(pPeriod + 1, &next, 10);
        if (errno)
                return ERROR_BAD_REQUEST;

        if (*next != 0 || next == pPeriod + 1)
                return ERROR_BAD_REQUEST;

        *pMajor = major;
        *pMinor = minor;
        return ERROR_SUCCESS;
}

/* Parse tokens into the struct 'request_line'
 *
 * @except ERROR_BAD_REQUEST
 */
static enum error parse_request_line_tokens(
        const struct request_line_tokens *restrict pReqLineToks,
        struct http_request_line *restrict pReqLine)
{
        if (!method_is_valid(pReqLineToks->method))
                return ERROR_BAD_REQUEST;
        if (!uri_is_valid(pReqLineToks->uri))
                return ERROR_BAD_REQUEST;

        unsigned long major, minor;
        enum error err =
                parse_http_version(pReqLineToks->version, &major, &minor);
        if (err)
                return err;

        strcpy(pReqLine->method, pReqLineToks->method);
        strcpy(pReqLine->uri, pReqLineToks->uri);
        pReqLine->major_version = major;
        pReqLine->minor_version = minor;

        return ERROR_SUCCESS;
}

enum error parse_http_request_line(const char *restrict fName,
        struct http_request_line *restrict pRes)
{
        if (!fName || !pRes)
                return ERROR_INVALID_PARAMETER;

        FILE *f = fopen(fName, "rb");
        if (!f)
                return ERROR_INVALID_PARAMETER;

        char reqLineStr[HTTP_REQUEST_LINE_MAX_LENGTH + 1];
        enum error err = read_request_line(f, reqLineStr);
        if (err)
                return err;

        struct request_line_tokens reqLineToks;
        err = tokenize_request_line_str(reqLineStr, &reqLineToks);
        if (err)
                return err;

        err = parse_request_line_tokens(&reqLineToks, pRes);
        if (err)
                return err;

        return ERROR_SUCCESS;
}
