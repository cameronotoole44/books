#include "dates.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static int make_tm_from_ymd(const char *s, struct tm *out) {
    int y, m, d;
    if (parse_ymd(s, &y, &m, &d) != 0) return 1;

    memset(out, 0, sizeof(*out));
    out->tm_year = y - 1900;
    out->tm_mon  = m - 1;
    out->tm_mday = d;
    out->tm_hour = 12;
    return 0;
}

int today_ymd(char out[11]) {
    if (!out) return 1;
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    if (!lt) return 1;
    snprintf(out, 11, "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
    return 0;
}

int parse_ymd(const char *s, int *y, int *m, int *d) {
    if (!s || strlen(s) < 10) return 1;
    if (s[4] != '-' || s[7] != '-') return 1;

    int yy = 0, mm = 0, dd = 0;
    if (sscanf(s, "%4d-%2d-%2d", &yy, &mm, &dd) != 3) return 1;
    if (mm < 1 || mm > 12) return 1;
    if (dd < 1 || dd > 31) return 1;

    if (y) *y = yy;
    if (m) *m = mm;
    if (d) *d = dd;
    return 0;
}

int add_days_ymd(const char *ymd, int days, char out[11]) {
    if (!ymd || !out) return 1;

    struct tm tmv;
    if (make_tm_from_ymd(ymd, &tmv) != 0) return 1;

    time_t t = mktime(&tmv);
    if (t == (time_t)-1) return 1;

    t += (time_t)days * 24 * 60 * 60;
    struct tm *res = localtime(&t);
    if (!res) return 1;

    snprintf(out, 11, "%04d-%02d-%02d", res->tm_year + 1900, res->tm_mon + 1, res->tm_mday);
    return 0;
}

int days_between_ymd(const char *a, const char *b) {
    struct tm ta, tb;
    if (make_tm_from_ymd(a, &ta) != 0) return 0;
    if (make_tm_from_ymd(b, &tb) != 0) return 0;

    time_t t1 = mktime(&ta);
    time_t t2 = mktime(&tb);
    if (t1 == (time_t)-1 || t2 == (time_t)-1) return 0;

    double diff = difftime(t2, t1);
    return (int)(diff / (24 * 60 * 60));
}
