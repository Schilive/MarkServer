/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/05
 */

/* This file declares the functions to detect the HTTP/1.0 character types,
 * as declared on RFC 1945.
 */

#ifndef MARKSERVER_HTTPCHAR_H
#define MARKSERVER_HTTPCHAR_H

#include <stdbool.h>
#include <stdlib.h>

/* Returns whether an array is an HTTP 'Method'.
 *
 * @param [in]  arr     The array to be checked.
 * @param       arrLen  The length of the array.
 * @return              Whether the array is a 'Method'.
 */
bool is_http_method(const char *arr, size_t arrLen);

/* Returns whether an array is an HTTP 'Request-URI'.
 *
 * @param [in]  arr     The array to be checked.
 * @param       arrLen  The length of the array.
 * @return              Whether the array is a 'Request-URI'.
 */
bool is_http_requestURI(const char *arr, size_t arrLen);

#endif /* MARKSERVER_HTTPCHAR_H */
