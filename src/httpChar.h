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

/* Returns whether a character is a token character.
 *
 * @param  ch   Character to be checked.
 * @return      Whether the character is a token character.
 */
bool is_http_token_char(char ch);

/* Returns whether a character is a DIGIT.
 *
 * @param  ch   Character to be checked.
 * @return      Whether the character is a DIGIT.
 */
bool is_http_DIGIT(char ch);

#endif /* MARKSERVER_HTTPCHAR_H */
