/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/30
 */

/* This file implements the declarations of "httpResponse.h".
 */

#include "httpResponse.h"

#include "httpChar.h"
#include "util.h"
#include <string.h>

static bool are_headers_valid(const struct http_header *hdrs)
{
        while (hdrs) {
                if (    !is_http_fieldName(hdrs->value, strlen(hdrs->value))
                ||      !is_http_fieldContent(hdrs->name, strlen(hdrs->name))) {
                        return false;
                }

                hdrs = hdrs->next;
        }

        return true;
}

/* Allocates a string containing the HTTP-Version.
 *
 * @except ERROR_INTERNAL
 * @except ERROR_MEMORY_ALLOCATION
 */
static enum error create_version(char **pRes, const struct http_version *pHttpV)
{
        char buf[1024];
        int chs = snprintf(buf, 1024, "%u.%u", pHttpV->major, pHttpV->minor);
        if (chs < 0 || chs == 1023)
                return ERROR_INTERNAL;

        char *vers = malloc(chs + 1);
        if (vers == NULL)
                return ERROR_MEMORY_ALLOCATION;
        memcpy(vers, buf, chs + 1);

        *pRes = vers;
        return ERROR_SUCCESS;
}

/* Allocates memory to create a string containing a Status-Line.
 *
 * @except ERROR_MEMORY_ALLOCATION
 */
static enum error create_statusLine(char **pRes,
        const struct http_version *pHttpV,
        const char *statusCode,
        const char *reasonPhrase)
{
        char *ln;
        enum error err = create_version(&ln, pHttpV);
        if (err != ERROR_SUCCESS)
                return err;
        err = append_character(&ln, ' ');
        if (err != ERROR_SUCCESS)
                goto error_allocated;
        err = append_string(&ln, statusCode);
        if (err != ERROR_SUCCESS)
                goto error_allocated;
        err = append_character(&ln, ' ');
        if (err != ERROR_SUCCESS)
                goto error_allocated;
        err = append_string(&ln, reasonPhrase);
        if (err != ERROR_SUCCESS)
                goto error_allocated;
        err = append_string(&ln, "\r\n");
        if (err != ERROR_SUCCESS)
                goto error_allocated;

        *pRes = ln;
        return ERROR_SUCCESS;

        error_allocated:
        free(ln);
        return err;
}

/* Appends header, ending it, with CRLF. If an error is returned, the content
 * of the response is undefined.
 *
 * @except ERROR_MEMORY_ALLOCATION
 * @except ERROR_INTERNAL
 */
static enum error append_header(char **pRes, const struct http_header *pHdr)
{
        /* Name:Value\r\n */
        size_t hdrLen = strlen(pHdr->name) + 1 + strlen(pHdr->value) + 2;
        char *hdrStr = realloc(*pRes, hdrLen + 1);
        if (hdrStr == NULL)
                return ERROR_MEMORY_ALLOCATION;
        *pRes = hdrStr;

        int chs = sprintf(hdrStr, "%s:%s\r\n", pHdr->name, pHdr->value);
        if (chs == 0 || chs != hdrLen)
                return ERROR_INTERNAL;
        return ERROR_SUCCESS;
}

/* Appends the headers to the string, ending the list, with CRLF. If an error
 * is returned, the content of the response is undefined.
 */
static enum error append_headers(char **pRes, const struct http_header *hdrs)
{
        while (hdrs) {
                enum error err = append_header(pRes, hdrs);
                if (err != ERROR_SUCCESS)
                        return err;

                hdrs = hdrs->next;
        }

        enum error err = append_string(pRes, "\r\n");
        return err;
}

enum error create_http_response_raw(const struct http_version *pHttpV,
                                    const char *statusCode,
                                    const char *reasonPhrase,
                                    const struct http_header *hdrs,
                                    const char *body,
                                    char **pRes)
{
        if (pHttpV == NULL || reasonPhrase == NULL || pRes == NULL)
                return ERROR_INVALID_PARAMETER;

        if (!is_http_statusCode(statusCode, strlen(statusCode)))
                return ERROR_INVALID_PARAMETER;
        if (!is_http_reasonPhrase(reasonPhrase, strlen(reasonPhrase)))
                return ERROR_INVALID_PARAMETER;
        if (!are_headers_valid(hdrs))
                return ERROR_INVALID_PARAMETER;

        char *resp;
        enum error err = create_statusLine(&resp, pHttpV, statusCode,
                reasonPhrase);
        if (err != ERROR_SUCCESS)
                return err;
        err = append_headers(&resp, hdrs);
        if (err != ERROR_SUCCESS)
                goto error_allocated;
        err = append_string(&resp, body);
        if (err != ERROR_SUCCESS)
                goto error_allocated;

        *pRes = resp;
        return ERROR_SUCCESS;

        error_allocated:
        free(resp);
        return err;
}
