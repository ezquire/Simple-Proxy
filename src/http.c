#include "http.h"

#include <stdio.h>
#include <string.h>

struct http_request_line
parse_http_request_line(char *buf, size_t len)
{
    static char const * const crlf = "\r\n";
    static size_t const crlflen = 2;
    static char const * const ws = " \t\r\v\f";
    static size_t const wslen = 5;

    char *p = buf, *end = buf + len;
    struct http_request_line line = { .end = end };

    // Consume leading CRLFs.
    // https://tools.ietf.org/html/rfc7230#section-3.5
    while (p != end && memchr(crlf, *p, crlflen) != NULL)
        ++p;
    len -= p - buf;

    if (p == end) {
        fputs("warning: invalid request line (empty)\n", stderr);
        return line;
    }

    // Method
    line.method.p = p;
    while (p != end && memchr(ws, *p, wslen) == NULL)
        ++p;
    line.method.len = p - line.method.p;

    // Consume whitespace.
    while (p != end && memchr(ws, *p, wslen) != NULL)
        ++p;
    len -= p - line.method.p;

    if (p == end) {
        fputs("warning: invalid request line (after method)\n", stderr);
        fprintf(stderr, "METHOD: %.*s\n", (int)line.method.len, line.method.p);
        return line;
    }

    // Request target
    line.request_target.p = p;
    while (p != end && memchr(ws, *p, wslen) == NULL)
        ++p;
    line.request_target.len = p - line.request_target.p;

    // Consume whitespace.
    while (p != end && memchr(ws, *p, wslen) != NULL)
        ++p;
    len -= p - line.request_target.p;

    if (p == end) {
        fputs("warning: invalid request line (after request target)\n", stderr);
        fprintf(stderr, "METHOD: %.*s\n", (int)line.method.len, line.method.p);
        fprintf(stderr, "REQUEST TARGET: %.*s\n", (int)line.request_target.len, line.request_target.p);
        return line;
    }

    // HTTP version
    line.http_version.p = p;
    while (p != end && memchr(ws, *p, wslen) == NULL)
        ++p;
    line.http_version.len = p - line.http_version.p;

    // Consume whitespace.
    while (p != end && memchr(ws, *p, wslen) != NULL)
        ++p;
    len -= p - line.http_version.p;

    if (p == end) {
        fputs("warning: invalid request line (after http version)\n", stderr);
        fprintf(stderr, "METHOD: %.*s\n", (int)line.method.len, line.method.p);
        fprintf(stderr, "REQUEST TARGET: %.*s\n", (int)line.request_target.len, line.request_target.p);
        fprintf(stderr, "HTTP VERSION: %.*s\n", (int)line.http_version.len, line.http_version.p);
        return line;
    }

    // LF (already consumed CR)
    if (*p != '\n') {
        fputs("warning: invalid request line (missing LF)\n", stderr);
        fprintf(stderr, "METHOD: %.*s\n", (int)line.method.len, line.method.p);
        fprintf(stderr, "REQUEST TARGET: %.*s\n", (int)line.request_target.len, line.request_target.p);
        fprintf(stderr, "HTTP VERSION: %.*s\n", (int)line.http_version.len, line.http_version.p);
        fprintf(stderr, "got instead of LF: %.*s\n", (int)len, p);
        return line;
    }

    line.valid = true;
    line.end = p + 1;

    return line;
}

void
debug_http_request_line(struct http_request_line reqline)
{
    if (reqline.valid)
        printf("valid HTTP request line:\n"
               "\tMETHOD: %.*s\n"
               "\tREQUEST TARGET: %.*s\n"
               "\tHTTP VERSION: %.*s\n",
               (int)reqline.method.len, reqline.method.p,
               (int)reqline.request_target.len, reqline.request_target.p,
               (int)reqline.http_version.len, reqline.http_version.p);
    else
        puts("not a valid HTTP request line");
}

struct http_status_line
parse_http_status_line(char *buf, size_t len)
{
    static char const * const crlf = "\r\n";
    static size_t const crlflen = 2;
    static char const * const ws = " \t\r\v\f";
    static size_t const wslen = 5;

    char *p = buf, *end = buf + len, *statend = memchr(crlf, *p, crlflen);
    statend += 2;
    struct http_status_line line = { .end = statend };


    /************************ NOT SURE IF THIS IS NEEDED *********************/
    // Consume leading CRLFs.
    // https://tools.ietf.org/html/rfc7230#section-3.5
    while (p != end && memchr(crlf, *p, crlflen) != NULL)
        ++p;
    len -= p - buf;

    if (p == end) {
        fputs("warning: invalid response line (empty)\n", stderr);
        return line;
    }
    /********************************** END *********************************/

    // HTTP version
    line.http_version.p = p;
    while (p != end && memchr(ws, *p, wslen) == NULL)
        ++p;
    line.http_version.len = p - line.http_version.p;

    // Consume whitespace.
    while (p != end && memchr(ws, *p, wslen) != NULL)
        ++p;
    len -= p - line.http_version.p;

    if (p == end) {
        fputs("warning: invalid status line (after http version)\n", stderr);
        fprintf(stderr, "HTTP VERSION: %.*s\n", (int)line.http_version.len, line.http_version.p);
        return line;
    }

    // Status code
    line.status_code.p = p;
    while (p != end && memchr(ws, *p, wslen) == NULL)
        ++p;
    line.status_code.len = p - line.status_code.p;

    // Consume whitespace.
    while (p != end && memchr(ws, *p, wslen) != NULL)
        ++p;
    len -= p - line.status_code.p;

    if (p == end) {
        fputs("warning: invalid status line (after status code)\n", stderr);
        fprintf(stderr, "HTTP VERSION: %.*s\n", (int)line.http_version.len, line.http_version.p);
        fprintf(stderr, "STATUS CODE: %.*s\n", (int)line.status_code.len, line.status_code.p);
        return line;
    }

    // Reason phrase
    line.reason_phrase.p = p;
    while (p != end && memchr(ws, *p, wslen) == NULL)
        ++p;
    line.reason_phrase.len = p - line.reason_phrase.p;

    // Consume whitespace.
    while (p != end && memchr(ws, *p, wslen) != NULL)
        ++p;
    len -= p - line.reason_phrase.p;

    if (p == end) {
        fputs("warning: invalid status line (after reason phrase)\n", stderr);
        fprintf(stderr, "HTTP VERSION: %.*s\n", (int)line.http_version.len, line.http_version.p);
        fprintf(stderr, "STATUS CODE: %.*s\n", (int)line.status_code.len, line.status_code.p);
        fprintf(stderr, "REASON PHRASE: %.*s\n", (int)line.reason_phrase.len, line.reason_phrase.p);
        return line;
    }

    /************************ NOT SURE IF THIS IS NEEDED *********************/
    // LF (already consumed CR)
    if (*p != '\n') {
        fputs("warning: invalid response line (missing LF)\n", stderr);
        fprintf(stderr, "HTTP VERSION: %.*s\n", (int)line.http_version.len, line.http_version.p);
        fprintf(stderr, "STATUS CODE: %.*s\n", (int)line.status_code.len, line.status_code.p);
        fprintf(stderr, "REASON PHRASE: %.*s\n", (int)line.reason_phrase.len, line.reason_phrase.p);
        fprintf(stderr, "got instead of LF: %.*s\n", (int)len, p);
        return line;
    }
    /********************************** END *********************************/

    line.valid = true;
    line.end = p + 1;

    return line;
}

void
debug_http_status_line(struct http_status_line statline)
{
    if (statline.valid)
        printf("valid HTTP status line:\n"
               "\tHTTP VERSION: %.*s\n"
               "\tSTATUS CODE: %.*s\n"
               "\tREASON PHRASE: %.*s\n",
               (int)statline.http_version.len, statline.http_version.p,
               (int)statline.status_code.len, statline.status_code.p,
               (int)statline.reason_phrase.len, statline.reason_phrase.p);
    else
        puts("not a valid HTTP status line");
}

struct http_header_field
parse_http_header_field(char *buf, size_t len)
{
    static char const * const ws = " \t\r\v\f";
    static size_t const wslen = 5;
    static char const * const nws = "\r\v\f";
    static size_t const nwslen = 3;

    char *p = buf, *end = buf + len;
    struct http_header_field head = { .end = end };

    if (p == end) {
        fputs("warning: invalid header field (empty)\n", stderr);
        return head;
    }

    // Field name
    head.field_name.p = p;
    p = memchr(p, ':', len);
    if(p == NULL) {
      fputs("warning: invalid header field\n", stderr);
      head.valid = false;
      return head;
    }
    head.field_name.len = p - head.field_name.p;

    if (p != NULL) {
        ++p; // :

        // Eat white space.
        while (p < end && memchr(ws, *p, wslen) != NULL)
            ++p;
        len -= p - head.field_name.p;
    }

    if (p == end || p == NULL) {
        fputs("warning: invalid header field (after field name)\n", stderr);
        fprintf(stderr, "FIELD NAME: %.*s\n", (int)head.field_name.len, head.field_name.p);
        return head;
    }

    // Field value
    head.field_value.p = p;
    while (p != end && memchr(nws, *p, nwslen) == NULL)
        ++p;
    head.field_value.len = p - head.field_value.p;

    // Consume whitespace.
    while (p != end && memchr(ws, *p, wslen) != NULL)
        ++p;
    len -= p - head.field_value.p;

    if (p == end) {
        fputs("warning: invalid header field (after field value)\n", stderr);
        fprintf(stderr, "FIELD NAME: %.*s\n", (int)head.field_name.len, head.field_name.p);
        fprintf(stderr, "FIELD VALUE: %.*s\n", (int)head.field_value.len, head.field_value.p);
        return head;
    }

    if (*p != '\n') {
        fputs("warning: invalid header field (missing LF)\n", stderr);
        fprintf(stderr, "FIELD NAME: %.*s\n", (int)head.field_name.len, head.field_name.p);
        fprintf(stderr, "FIELD VALUE: %.*s\n", (int)head.field_value.len, head.field_value.p);
        fprintf(stderr, "got instead of LF: %.*s\n", (int)len, p);
        return head;
    }

    head.valid = true;
    head.end = p + 1;

    return head;
}

void
debug_http_header_field(struct http_header_field reqhead)
{
    if (reqhead.valid)
        printf("valid HTTP header field:\n"
               "\tFIELD NAME: %.*s\n"
               "\tFIELD VALUE: %.*s\n",
               (int)reqhead.field_name.len, reqhead.field_name.p,
               (int)reqhead.field_value.len, reqhead.field_value.p);
    else
        puts("not a valid HTTP header field");
}
