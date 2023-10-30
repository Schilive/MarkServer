/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/30
 */

/* This file declares the functions to create HTTP responses.
 */

#ifndef MARKSERVER_HTTPRESPONSE_H
#define MARKSERVER_HTTPRESPONSE_H

#include "httpParse.h"


/* Create and HTTP Response as string. To free the memory, use free().
 *
 * @param [in]          pHttpV           The http version of the response.
 * @param [in]          statusCode      The 3-digit status code of the response.
 * @param [in]          reasonPhrase    The reason phrase of the status code.
 * @param [in, opt]     hdrs            The headers of the response.
 * @param [in]          body            The Entity-Body of the response.
 * @param [out]         pRes            The created response.
 * @return                              The error value.
 * @except                              ERROR_INVALIID_PARAMETER
 */
enum error create_http_response_raw(const struct http_version *pHttpV,
                                    const char *statusCode,
                                    const char *reasonPhrase,
                                    const struct http_header *hdrs,
                                    const char *body,
                                    char **pRes);

#endif /* MARKSERVER_HTTPRESPONSE_H */
