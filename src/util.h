/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/30
 */

/* This file declares some useful functions.
 */

#ifndef MARKSERVER_UTIL_H
#define MARKSERVER_UTIL_H

#include <stdlib.h>

/* Reallocates string to append character.
 *
 * @except ERROR_MEMORY_ALLOCATION
 */
enum error append_character(char **pStr, char ch);

/* Reallocates string to append array.
 *
 * @except ERROR_MEMORY_ALLOCATION
 */
enum error append_array(char **pStr, const char *arr, size_t arrLen);

/* Reallocates string to append string.
 *
 * @except ERROR_MEMORY_ALLOCATION
 */
enum error append_string(char **pStr1, const char *str2);

#endif /* MARKSERVER_UTIL_H */
