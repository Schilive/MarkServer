/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, github.com/Schilive/MarkServer.
 *
 * This file was originally created 2023/10/28.
 */

/* This file implements the declarations of "error.h".
 */

#include "error.h"

const static char *errorStr_unknown = "Unknown error";
const static char *errorStr_success = "Success";
const static char *errorStr_requestIncomplete = "Request incomplete";
const static char *errorStr_invalidParameter = "Invalid parameter";
const static char *errorStr_requestTooLong = "Request too long";
const static char *errorStr_badRequest = "Bad request";
const static char *errorStr_memoryAllocation = "Memory allocation";

const char *error_string(enum error err)
{
        switch (err) {
                case ERROR_SUCCESS:
                        return errorStr_success;
                case ERROR_REQUEST_INCOMPLETE:
                        return errorStr_requestIncomplete;
                case ERROR_INVALID_PARAMETER:
                        return errorStr_invalidParameter;
                case ERROR_REQUEST_TOO_LONG:
                        return errorStr_requestTooLong;
                case ERROR_BAD_REQUEST:
                        return errorStr_badRequest;
                case ERROR_MEMORY_ALLOCATION:
                        return errorStr_memoryAllocation;
                default:
                        return errorStr_unknown;
        }
}
