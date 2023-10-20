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

/* Buffers the Request-Line of the start of the file, as a string, whether
 * the content of the fields are correct or not. That is, until CRLF.
 *
 * @param       f       The file to be read.
 * @param       reqLine The buffer.
 * @param       maxLen  The maximum length of the buffer. If the buffer is too
 *                      small, then ERROR_REQUEST_TOO_LONG is returned.
 * @except              ERROR_REQUEST_INCOMPLETE
 * @except              ERROR_INTERNAL
 * @except              ERROR_REQUEST_TOO_LONG
 */
static enum error read_request_line(FILE *restrict f, char *restrict reqLine)
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

        memcpy(reqLine, reqLineTxt, reqLineLen);
        reqLine[reqLineLen] = 0;

        return ERROR_SUCCESS;
}

/* Tokenizes the Request-Line string.
 *
 * @except ERROR_BAD_REQUEST
 */
static enum error tokenize_request_line_str(const char *restrict reqLine,
        struct request_line_tokens *restrict pRes)
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

        memcpy(pRes->method, reqLine, methodLen);
        memcpy(pRes->uri, sp1 + 1, uriLen);
        memcpy(pRes->version, sp2 + 1, versLen);

        pRes->method[methodLen] = 0;
        pRes->uri[uriLen] = 0;
        pRes->version[versLen] = 0;

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

/* Parse tokens into the struct 'request_line'
 *
 * @except ERROR_BAD_REQUEST
 */
static enum error parse_request_line_tokens(
        const struct request_line_tokens *restrict pReqLineToks,
        struct http_request_line *restrict pRes)
{
        if (!is_valid_method(pReqLineToks->method))
                return ERROR_BAD_REQUEST;
        if (!is_valid_uri(pReqLineToks->uri))
                return ERROR_BAD_REQUEST;

        unsigned long major, minor;
        enum error err =
                parse_http_version(pReqLineToks->version, &major, &minor);
        if (err)
                return err;

        strcpy(pRes->method, pReqLineToks->method);
        strcpy(pRes->uri, pReqLineToks->uri);
        pRes->http_version.major = major;
        pRes->http_version.minor = minor;

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
