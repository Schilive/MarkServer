/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, see github.com/Schilive/MarkServer.
 *
 * This file was originally created in 2023/10/05
 */

/* This file implements the declarations on "httpChar.h".
 */

/* Some functions contain a comment informing the structure of an object, as
 * "token = 1*<any CHAR except CTLs or tspecials>". They use the BNF
 * (Backus-Naur Normal Form) notation. */

#include "httpChar.h"
#include <string.h>

static bool is_CHAR(char ch)
{
        return ch >= 0;
}

static bool is_CTL(char ch)
{
        return (ch >= 0 && ch <= 31) || ch == 127;
}

static bool is_tspecials(char ch)
{
        return  ch == '(' || ch == ')' || ch == '<' || ch == '>'  || ch == '@'
                ||      ch == ',' || ch == ';' || ch == ':' || ch == '\\' || ch == '\"'
                ||      ch == '/' || ch == '[' || ch == ']' || ch == '?'  || ch == '='
                ||      ch == '{' || ch == '}' || ch == ' ' || ch == '\t';
}

static bool is_DIGIT(char ch)
{
        return  ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4'
        ||      ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9';
}

static bool is_UPALPHA(char ch)
{
        return  ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'E'
        ||      ch == 'F' || ch == 'G' || ch == 'H' || ch == 'I' || ch == 'J'
        ||      ch == 'K' || ch == 'L' || ch == 'M' || ch == 'N' || ch == 'O'
        ||      ch == 'P' || ch == 'Q' || ch == 'R' || ch == 'S' || ch == 'T'
        ||      ch == 'U' || ch == 'V' || ch == 'W' || ch == 'X' || ch == 'Y'
        ||      ch == 'Z';
}

static bool is_LOALPHA(char ch)
{
        return  ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' || ch == 'e'
        ||      ch == 'f' || ch == 'g' || ch == 'h' || ch == 'i' || ch == 'j'
        ||      ch == 'k' || ch == 'l' || ch == 'm' || ch == 'n' || ch == 'o'
        ||      ch == 'p' || ch == 'q' || ch == 'r' || ch == 's' || ch == 't'
        ||      ch == 'u' || ch == 'v' || ch == 'w' || ch == 'x' || ch == 'y'
        ||      ch == 'z';
}

static bool is_safe(char ch)
{
        return ch == '$' || ch == '-' || ch == '_' || ch == '.';
}

static bool is_extra(char ch)
{
        return  ch == '!' || ch == '*' || ch == '\'' || ch == '(' || ch == ')'
        ||      ch == ',';
}

static bool is_reserved(char ch)
{
        return  ch == ';' || ch == '/' || ch == '?' || ch == ':' || ch == '@'
        ||      ch == '&' || ch == '=' || ch == '+';
}

static bool is_unsafe(char ch)
{
        return  is_CTL(ch)      || ch == ' ' || ch == '\"' || ch == '#'
        ||      ch == '%'       || ch == '<' || ch == '>';
}

static bool is_ALPHA(char ch)
{
        return is_UPALPHA(ch) || is_LOALPHA(ch);
}

static bool is_national(char ch)
{
        return  !is_ALPHA(ch) && !is_DIGIT(ch)  && !is_reserved(ch)
        &&      !is_extra(ch) && !is_safe(ch)   && !is_unsafe(ch);
}

static bool is_HEX(char ch)
{
        return  is_DIGIT(ch)
        || ch == 'A' || ch == 'a'
        || ch == 'B' || ch == 'b'
        || ch == 'C' || ch == 'c'
        || ch == 'D' || ch == 'd'
        || ch == 'E' || ch == 'e'
        || ch == 'F' || ch == 'f';
}

static bool is_unreserved(char ch)
{
        return  is_ALPHA(ch) || is_DIGIT(ch) || is_safe(ch) || is_extra(ch)
        ||      is_national(ch);
}

/* Checks whether an array starts with 'escape'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'escape', if any.
 * @return                      Returns whether an array starts with 'escape'
 */
static bool starts_with_escape(const char *restrict arr, size_t arrLen,
                               size_t *restrict pLen)
{
        /* escape = "%" HEX HEX */

        if (arrLen < 3)
                return false;
        if (arr[0] != '%' || !is_HEX(arr[1]) || !is_HEX(arr[2]))
                return false;

        if (pLen)
                *pLen = 3;
        return true;
}

/* Checks whether an array starts with 'uchar'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'uchar', if any.
 * @return                      Returns whether an array starts with 'uchar'
 */
static bool starts_with_uchar(const char *restrict arr, size_t arrLen,
                              size_t *restrict pLen)
{
        /* uchar = unreserved | escape */

        size_t len;
        if (starts_with_escape(arr, arrLen, &len)) {
                if (pLen)
                        *pLen = len;
                return true;
        }

        if (arrLen == 0)
                return false;
        if (!is_unreserved(arr[0]))
                return false;

        if (pLen)
                *pLen = 1;
        return true;
}

/* Checks whether an array starts with 'pchar'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'pchar', if any.
 * @return                      Returns whether an array starts with 'pchar'
 */
static bool starts_with_pchar(const char *restrict arr, size_t arrLen,
                              size_t *restrict pLen)
{
        /* pchar = uchar | ":" | "@" | "&" | "=" | "+" */

        if (arrLen == 0)
                return false;

        size_t offset = 0;
        while (1) {
                if (offset == arrLen)
                        break;

                if (    arr[offset] == ':' || arr[offset] == '@'
                ||      arr[offset] == '&' || arr[offset] == '='
                ||      arr[offset] == '+') {
                        offset++;
                        continue;
                }

                size_t len;
                if (!starts_with_uchar(arr + offset, arrLen - offset, &len))
                        break;
                offset += len;
        }

        if (pLen)
                *pLen = offset;
        return true;
}

/* Checks whether an array starts with 'fsegment'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'fsegment', if any.
 * @return                      Returns whether an array starts with 'fsegment'
 */
static bool starts_with_fsegment(const char *restrict arr, size_t arrLen,
                                 size_t *restrict pLen)
{
        /* fsegment = 1*pchar */

        size_t len;
        if (!starts_with_pchar(arr, arrLen, &len))
                return false;

        if (len == 0)
                return false;

        if (pLen)
                *pLen = len;
        return true;
}

/* Checks whether an array starts with 'segment'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'segment', if any.
 * @return                      Returns whether an array starts with 'segment'
 */
static bool starts_with_segment(const char *restrict arr, size_t arrLen,
                                size_t *restrict pLen)
{
        /* segment = *pchar */

        size_t len;
        if (!starts_with_pchar(arr, arrLen, &len)) {
                if (pLen)
                        *pLen = 0;
                return true;
        }

        if (pLen)
                *pLen = len;
        return true;
}

/* Checks whether an array starts with 'path'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'path', if any.
 * @return                      Returns whether an array starts with 'path'
 */
static bool starts_with_path(const char *restrict arr, size_t arrLen,
                             size_t *restrict pLen)
{
        /* path = fsegment *("/" segment) */

        size_t len;
        size_t offset = 0;

        if (!starts_with_fsegment(arr, arrLen, &offset))
                return false;
        len = offset;

        while (1) {
                if (offset == arrLen)
                        break;
                if (arr[offset] != '/')
                        break;
                offset++;

                size_t segLen;
                if (!starts_with_segment(arr + offset, arrLen - offset,
                                         &segLen))
                        break;

                offset += segLen;
                len = offset;
        }

        if (pLen)
                *pLen = len;
        return true;
}

/* Checks whether an array starts with 'param'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'param', if any.
 * @return                      Returns whether an array starts with 'param'
 */
static bool starts_with_param(const char *restrict arr, size_t arrLen,
                              size_t *restrict pLen)
{
        /* param = *( pchar | "/" ) */

        size_t offset = 0;
        while (1) {
                if (offset == arrLen)
                        break;

                if (arr[offset] == '/') {
                        offset++;
                        continue;
                }

                size_t len;
                if (!starts_with_pchar(arr + offset, arrLen - offset, &len))
                        break;
                offset += len;
        }

        if (pLen)
                *pLen = offset;
        return true;
}

/* Checks whether an array starts with 'params'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'params', if any.
 * @return                      Returns whether an array starts with 'params'
 */
static bool starts_with_params(const char *restrict arr, size_t arrLen,
                               size_t *restrict pLen)
{
        /* params = param *( ";" param ) */

        size_t offset, len;

        if (!starts_with_param(arr, arrLen, &offset))
                return false;
        len = offset;

        while (1) {
                if (offset == arrLen)
                        break;
                if (arr[offset] != ';')
                        break;
                offset++;

                size_t paramLen;
                if (!starts_with_param(arr + offset, arrLen - offset,
                                       &paramLen)) {
                        break;
                }

                offset += paramLen;
                len = offset;
        }

        if (pLen)
                *pLen = len;
        return true;
}

/* Checks whether an array starts with 'query'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'query', if any.
 * @return                      Returns whether an array starts with 'query'
 */
static bool starts_with_query(const char *restrict arr, size_t arrLen,
                              size_t *restrict pLen)
{
        /* query = *(uchar | reserved) */

        size_t offset = 0;

        while (1) {
                if (offset == arrLen)
                        break;
                if (is_reserved(arr[offset])) {
                        offset++;
                        continue;
                }

                size_t len;
                if (!starts_with_uchar(arr + offset, arrLen - offset, &len))
                        break;

                offset += len;
        }

        if (pLen)
                *pLen = offset;
        return true;
}

/* Checks whether an array starts with 'rel_path'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'rel_path', if any.
 * @return                      Returns whether an array starts with 'rel_path'
 */
static bool starts_with_relPath(const char *restrict arr, size_t arrLen,
                                size_t *restrict pLen)
{
        /* rel_path = [path] [";" params] ["?" query] */

        size_t offset, len;

        starts_with_path(arr, arrLen, &offset);

        if (offset == arrLen) {
                if (pLen)
                        *pLen = offset;
                return true;
        } if (arr[offset] == ';') {
                offset++;

                if (!starts_with_params(arr + offset, arrLen - offset, &len))
                        return false;
                offset += len;
        }

        if (offset == arrLen) {
                if (pLen)
                        *pLen = offset;
                return true;
        } if (arr[offset] !=  '?') {
                if (pLen)
                        *pLen = offset;
                return true;
        }
        offset++;

        if (!starts_with_query(arr + offset, arrLen - offset, &len))
                return false;
        offset += len;

        if (pLen)
                *pLen = offset;
        return true;
}

/* Checks whether an array starts with 'abs_path'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'abs_path', if any.
 * @return                      Returns whether an array starts with 'abs_path'
 */
static bool starts_with_absPath(const char *restrict arr, size_t arrLen,
                                size_t *restrict pLen)
{
        /* abs_path = "/" rel_path */

        if (arrLen == 0)
                return false;
        if (arr[0] != '/')
                return false;

        size_t relPathLen;
        if (!starts_with_relPath(arr + 1, arrLen - 1, &relPathLen))
                return false;

        if (pLen)
                *pLen = relPathLen + 1;
        return true;
}

/* Checks whether an array starts with 'scheme'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'scheme', if any.
 * @return                      Returns whether an array starts with 'scheme'
 */
static bool starts_with_scheme(const char *restrict arr, size_t arrLen,
                               size_t *restrict pLen)
{
        /* scheme = 1*( ALPHA | DIGIT | "+" | "-" | "." ) */

        size_t offset = 0;
        while (1) {
                if (offset == arrLen)
                        break;

                if (    !is_ALPHA(arr[offset])  && !is_DIGIT(arr[offset])
                &&      arr[offset] != '+'      && arr[offset] != '-'
                &&      arr[offset] != '.') {
                        break;
                }

                offset++;
        }

        if (offset == 0)
                return false;

        if (pLen)
                *pLen = offset;
        return true;
}

/* Checks whether an array starts with 'absoluteURI'.
 *
 * @param  [in]       arr       The array being checked.
 * @param             arrLen    The length of the array.
 * @param  [out, opt] pLen      The length of the 'absoluteURI', if any.
 * @return                      Returns whether an array starts with
 *                              'absoluteURI'
 */
static bool starts_with_absoluteURI(const char *restrict arr, size_t arrLen,
                                    size_t *restrict pLen)
{
        /* absoluteURI = scheme ":" *(uchar | reserved) */

        size_t offset;
        if (!starts_with_scheme(arr, arrLen, &offset))
                return false;
        if (offset == arrLen)
                return false;
        if (arr[offset] != ':')
                return false;
        offset++;

        while (1) {
                if (offset == arrLen)
                        break;

                if (is_reserved(arr[offset])) {
                        offset++;
                        continue;
                }

                size_t ucharLen;
                if (!starts_with_uchar(arr + offset, arrLen - offset,
                                       &ucharLen)) {
                        break;
                }

                offset += ucharLen;
        }

        if (pLen)
                *pLen = offset;
        return true;
}

/* Checks whether an array starts with 'token'.
 *
 * @param [in]          arr     The array to be checked.
 * @param               arrLen  The length of the array.
 * @param [out, opt]    pLen    The length of the 'token', if any.
 * @return                      Whether the array starts with 'token'.
 */
static bool starts_with_token(const char *restrict arr, size_t arrLen,
                                                        size_t *restrict pLen)
{
        /* token = 1*<any CHAR except CTLs or tspecials> */

        size_t len;
        for (len = 0; len < arrLen; len++) {
                if (    !is_CHAR(arr[len]) || is_CTL(arr[len])
                ||      is_tspecials(arr[len])) {
                        break;
                }
        }

        if (arrLen == 0)
                return false;

        if (pLen)
                *pLen = len;
        return true;
}

/* Checks whether an array starts with 'qdtext'.
 *
 * @param [in]          arr     The array to be checked.
 * @param               arrLen  The length of the array.
 * @param [out, opt]    pLen    The length of the 'qdtext', if any.
 * @return                      Whether the array starts with 'qdtext'.
 */
static bool starts_with_qdtext(const char *restrict arr, size_t arrLen,
                               size_t *restrict pLen)
{
        /* qdtext = <any CHAR except <"> and CTLS, but including LWS> */

        size_t len = 0;
        while (1) {
                if (len == arrLen)
                        break;
                if (arr[len] == '\"')
                        break;

                size_t LWSLen;
                if (starts_with_http_LWS(arr + len, arrLen - len, &LWSLen)) {
                        len += LWSLen;
                        continue;
                } else if (is_CTL(arr[len])) {
                        break;
                }
        }

        if (len == 0)
                return false;

        *pLen = len;
        return true;
}

/* Checks whether an array starts with 'TEXT'.
 *
 * @param [in]          arr     The array to be checked.
 * @param               arrLen  The length of the array.
 * @param [out, opt]    pLen    The length of the 'TEXT', if any.
 * @return                      Whether the array starts with 'TEXT'.
 */
static bool starts_with_TEXT(const char *restrict arr, size_t arrLen,
                             size_t *restrict pLen)
{
        /* TEXT = <any OCTET except CTLs, but including LWS> */

        size_t len = 0;
        while (1) {
                size_t LWSLen;
                if (starts_with_http_LWS(arr + len, arrLen - len, &LWSLen)) {
                        len += LWSLen;
                        continue;
                } else if (is_CTL(arr[len])) {
                        break;
                }

                len++;
        }

        *pLen = len;
        return true;
}

bool is_http_method(const char *arr, size_t arrLen)
{
        /* Method = token */

        size_t tokenLen;
        if (!starts_with_token(arr, arrLen, &tokenLen))
                return false;

        return tokenLen == arrLen;
}

bool is_http_requestURI(const char *arr, size_t arrLen)
{
        /* Request-URI = absoluteURI | abs_path */

        size_t len;
        if (starts_with_absoluteURI(arr, arrLen, &len)) {
                if (len == arrLen)
                        return true;
        }

        if (!starts_with_absPath(arr, arrLen, &len))
                return false;


        return len == arrLen;
}

bool starts_with_http_LWS(const char *restrict arr, size_t arrLen,
                          size_t *restrict pLen)
{
        /* LWS = [CRLF] 1*(SP | HT) */

        if (arrLen == 0)
                return false;

        size_t len = 0;
        if (arrLen < 2) {}
        else if (arr[0] == '\r' && arr[1] == '\n')
                len += 2;

        if (arr[len] != ' ' && arr[len] != '\t')
                return false;
        len++;

        while (arr[len] == ' ' || arr[len] == '\t')
                len++;

        *pLen = len;
        return true;
}

bool is_http_fieldContent(const char *restrict arr, size_t arrLen)
{
        /* field-content = <the OCTETs making up the field-value
                        and consisting of either *TEXT or combinations
                        of token, tspecials, and quoted-string>

         This implies that if field-content is not *TEXT, then it is "tokens",
         "tspecials", or "qdtext", since <"> is "tespecial".
        */

        size_t len;
        if (!starts_with_TEXT(arr, arrLen, &len)) {}
        else if (len == arrLen)
                return true;

        size_t offset = 0;
        while (1) {
                if (offset == arrLen)
                        return true;

                if (starts_with_token(arr + offset, arrLen - offset, &len)) {
                        offset += len;
                        continue;
                } if (is_tspecials(arr[offset])) {
                        offset++;
                        continue;
                } if (starts_with_qdtext(arr + offset, arrLen - offset, &len)) {
                        offset += len;
                        continue;
                }

                return false;
        }
}

bool is_http_fieldName(const char *restrict arr, size_t arrLen)
{
        /* field-name = token */

        size_t len;
        if (!starts_with_token(arr, arrLen, &len))
                return false;
        return len == arrLen;
}
