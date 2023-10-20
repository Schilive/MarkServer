/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/05
 */

#ifndef MARKSERVER_ERROR_H
#define MARKSERVER_ERROR_H

enum error {
    ERROR_SUCCESS,
    ERROR_REQUEST_INCOMPLETE,
    ERROR_INVALID_PARAMETER,
    ERROR_REQUEST_TOO_LONG,     /* Something of the HTTP request was considered
                                 * too long. */
    ERROR_INTERNAL,
    ERROR_BAD_REQUEST           /* Request is malformed by the standard. */
};

#endif /* MARKSERVER_ERROR_H */
