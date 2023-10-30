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

/* Reallocates string to append character.
 *
 * @except ERROR_MEMORY_ALLOCATION
 */
static enum error append_character(char **pStr, char ch)
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

/* Reallocates string to append array.
 *
 * @except ERROR_MEMORY_ALLOCATION
 */
static enum error append_array(char **pStr, const char *arr, size_t arrLen)
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

static bool is_valid_header(const char *restrict name,
                            const char *restrict val)
{
        return is_http_fieldName(name, strlen(name)) &&
               is_http_fieldContent(val, strlen(val));
}

/* Parses the HTTP version string, verifying that its format is correct.
 * Returns the major and minor versions. It changes the content.
 *
 * @except ERROR_BAD_REQUEST
 * @except ERROR_REQUEST_TOO_LONG
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

        if (ulMajor >= UINT_MAX || ulMinor >= UINT_MAX)
                return ERROR_REQUEST_TOO_LONG;
        pRes->major = (unsigned int)ulMajor;
        pRes->minor = (unsigned int)ulMinor;

        return ERROR_SUCCESS;
}

/* Parses the Request-Line string, modifying 'reqLn' content.
 *
 * @param [in]  reqLn   String starting with Request-Line.
 * @param [out] pRes    The parsed Request-Line. Only changed if successful.
 * @param [out] next    The character immediately after the Request-Line. Only
 *                      changed if successful.
 *
 * @except ERROR_BAD_REQUEST
 * @except ERROR_REQUEST_INCOMPLETE
 * @except ERROR_REQUEST_TOO_LONG
 */
static enum error parse_request_line(char *restrict reqLn,
        struct http_request_line *restrict pRes, char **next)
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
        *next = end + 2;

        return ERROR_SUCCESS;
}

/* Parses the 'field-value' of a header to extract only the 'field-content'.
 *
 * @param [out] str     The string starting with 'field-value'. Not changed.
 * @param [out] pRes    The parsed 'field-value'. Only changed if successful.
 * @param [out] pNext   The character immediately after the end of the header,
 *                      that is, after CRLF. Only changed if successful.
 * @except              ERROR_REQUEST_INCOMPLETE
 * @except              ERROR_MEMORY_ALLOCATION
 */
static enum error parse_header_value(char *restrict str, char **restrict pRes,
        char **restrict pNext)
{
        enum error err;

        char *val = NULL;
        size_t offset = 0;
        while (1) {
                if (str[offset] == 0) {
                        err = ERROR_REQUEST_INCOMPLETE;
                        goto error;
                }

                size_t LWSLen;
                if (starts_with_http_LWS(str + offset,
                                         strlen(str + offset),
                                         &LWSLen)) {
                        offset += LWSLen;
                        continue;
                } if (str[offset] == '\r' && str[offset + 1] == '\n') {
                        break;
                }

                err = append_character(&val, str[offset]);
                if (err != ERROR_SUCCESS)
                        goto error;
                offset++;
        }

        *pRes = val;
        *pNext = str + offset + 2;
        return ERROR_SUCCESS;

        error:
        free(val);
        return err;
}

/* Parses a header, ending in CRLF. It modifies the string.
 *
 * @param [in]  str     The string starting with the 'field-name'.
 * @param [out] pRes    The parsed header. Only changed if successful.
 * @param [out] pNext   The character immediately after the end of the header,
 *                      that is, after CRLF. Only changed if successful.
 *
 * @except              ERROR_REQUEST_INCOMPLETE
 * @except              ERROR_MEMORY_ALLOCATION
 * @except              ERROR_BAD_REQUEST
 */
static enum error parse_header(char *restrict str,
        struct http_header **restrict pRes, char **restrict pNext)
{
        char *endLn = strstr(str, "\r\n");
        if (endLn == NULL)
                return ERROR_BAD_REQUEST;
        char *pColon = strchr(str, ':');
        if (pColon == NULL || pColon > endLn)
                return ERROR_BAD_REQUEST;
        char *pSP = strchr(str, ' ');
        char *pHT = strchr(str, '\t');
        if ((pSP != NULL && pSP < pColon ) || (pHT != NULL && pHT < pColon))
                return ERROR_BAD_REQUEST;

        *pColon = 0;

        char *val;
        char *next;
        enum error err = parse_header_value(pColon + 1, &val, &next);
        if (err != ERROR_SUCCESS)
                return err;



        struct http_header *pHdr = malloc(sizeof(struct http_header));
        if (pHdr == NULL) {
                free(val);
                return ERROR_MEMORY_ALLOCATION;
        }
        pHdr->name = str;
        pHdr->value = val;
        pHdr->next = NULL;

        *pRes = pHdr;
        *pNext = next;
        return ERROR_SUCCESS;
}

/* Appends a header struct to the end of the header list. */
static void append_header(struct http_header **restrict pHead,
        struct http_header *restrict new)
{
        struct http_header **pNext = pHead;
        while (*pNext)
                pNext = &((*pNext)->next);
        *pNext = new;
}

/* Deallocates all allocated stuff in the header list. */
static void destroy_headers(struct http_header *head)
{
        while (head) {
                struct http_header *next = head->next;

                free(head->value);
                free(head);

                head = next;
        }
}

/* Parses the header list, ending in CRLF. Mutates the string.
 *
 * @param [in]  str     The string starting with the header list.
 * @param [out] pRes    The parsed header. Only changed if successful.
 * @except              ERROR_REQUEST_INCOMPLETE
 * @except              ERROR_MEMORY_ALLOCATION
 * @except              ERROR_BAD_REQUEST
 */
static enum error parse_headers(char *restrict str,
        struct http_header **restrict pRes)
{
        enum error err;
        struct http_header *hdrs = NULL;

        size_t offset = 0;
        while (1) {
                if (str[offset] == 0) {
                        err = ERROR_REQUEST_INCOMPLETE;
                        goto error;
                } if (str[offset] == '\r' && str[offset + 1] == '\n')
                        break;

                char *nextHdr;
                struct http_header *pHdr;
                err = parse_header(str + offset,  &pHdr, &nextHdr);
                if (err != ERROR_SUCCESS)
                        goto error;
                append_header(&hdrs, pHdr);

                offset = nextHdr - str;
        }

        *pRes = hdrs;
        return ERROR_SUCCESS;

        error:
        destroy_headers(hdrs);
        return err;
}

enum error parse_http_request(char *restrict req,
        struct http_request *restrict pRes)
{
        if (req == NULL || pRes == NULL)
                return ERROR_INVALID_PARAMETER;

        char *hdrsStr;
        struct http_request_line reqLn;
        enum error err = parse_request_line(req, &reqLn, &hdrsStr);
        if (err != ERROR_SUCCESS)
                return err;

        struct http_header *hdrs;
        err = parse_headers(hdrsStr, &hdrs);
        if (err != ERROR_SUCCESS)
                return err;

        pRes->request_line = reqLn;
        pRes->headers = hdrs;
        return ERROR_SUCCESS;
}

enum error destroy_http_request(struct http_request *pReq)
{
        if (pReq == NULL)
                return ERROR_INVALID_PARAMETER;

        destroy_headers(pReq->headers);
        return ERROR_SUCCESS;
}
