/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/05
 */

/* This file declares functions to parse HTTP requests, but not responses.
 */

#ifndef MARKSERVER_HTTPPARSE_H
#define MARKSERVER_HTTPPARSE_H

#include <stdio.h>
#include "error.h"

struct http_version {
    unsigned int major;
    unsigned int minor;
};

/* The parsed Request-Line. */
struct http_request_line {
    char *method;
    char *uri;
    struct http_version http_version;
};

struct http_header {
    char *name;
    char *value;
    struct http_header *next; /* If NULL, there is no next header. */
};

struct http_request {
    struct http_request_line request_line;
    struct http_header *headers; /* If NULL, there are no headers. */
};

/* Parses the Request-Line in a string, checking the logic. It mutates the
 * string and allocates.
 *
 * @param [in]  fName   The string of the Request-Line. Its content is
 *                      undefined after the function call.
 * @param [out] pRes    The parsed Request-Line. Only changed if no
 *                      error is returned.
 * @return              The error value.
 * @except              ERROR_INVALID_PARAMETER
 * @except              ERROR_REQUEST_INCOMPLETE
 * @except              ERROR_REQUEST_TOO_LONG
 * @except              ERROR_BAD_REQUEST
 * @except              ERROR_MEMORY_ALLOCATION
 */
enum error parse_http_request(char *restrict req,
        struct http_request *restrict pRes);

/* Deallocates the allocated stuff in the HTTP Request analysis.
 *
 * @param [in]  pReq    The valid analysis. Nothing is deallocated if an error
 *                      is returned.
 * @return              The error value.
 * @except              ERROR_INVALID_PARAMETER
 */
enum error destroy_http_request(struct http_request *pReq);

#endif /* MARKSERVER_HTTPPARSE_H */
