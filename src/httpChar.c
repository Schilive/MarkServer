/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/05
 */

/* This file implements the declarations on "httpChar.h".
 */

#include "httpChar.h"

static bool is_CHAR(char ch)
{
        return ch >= 0;
}

static bool is_CTL(char ch)
{
        return (ch >= 0 && ch <= 31) || ch == 127;
}

static bool is_tspecial(char ch)
{
        return  ch == '(' || ch == ')' || ch == '<' || ch == '>'  || ch == '@'
        ||      ch == ',' || ch == ';' || ch == ':' || ch == '\\' || ch == '\"'
        ||      ch == '/' || ch == '[' || ch == ']' || ch == '?'  || ch == '='
        ||      ch == '{' || ch == '}' || ch == ' ' || ch == '\t';
}

bool is_http_token_char(char ch)
{
        return is_CHAR(ch) && !is_CTL(ch) && !is_tspecial(ch);
}

bool is_http_DIGIT(char ch)
{
        return  ch == '0' || ch == '1' || ch == '2'
        ||      ch == '3' || ch == '4' || ch == '5'
        ||      ch == '6' || ch == '7' || ch == '8'
        ||      ch == '9';
}
