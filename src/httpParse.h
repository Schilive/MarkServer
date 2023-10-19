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

#define HTTP_METHOD_MAX_LENGTH 255
#define HTTP_URI_MAX_LENGTH 1023

/* The parsed Request-Line. */
struct http_request_line {
    char method[HTTP_METHOD_MAX_LENGTH + 1];
    char uri[HTTP_URI_MAX_LENGTH];
    unsigned long major_version;
    unsigned long minor_version;
};

/* Parses the Request-Line in a string, checking the logic.
 *
 * @param [in]  fName   The filename of the file starting with the
 *                      Request-Line.
 * @param [out] pRes    The parsed Request-Line. Only changed if no
 *                      error is returned.
 * @return              The error value.
 * @except              ERROR_INVALID_PARAMETER.
 * @except              ERROR_REQUEST_INCOMPLETE
 * @except              ERROR_INTERNAL
 * @except              ERROR_REQUEST_TOO_LONG
 * @except              ERROR_BAD_REQUEST
 */
enum error parse_http_request_line(     const char *restrict fName,
        struct http_request_line *restrict pRes);

#endif /* MARKSERVER_HTTPPARSE_H */
