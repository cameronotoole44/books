#ifndef DATES_H
#define DATES_H

int today_ymd(char out[11]);
int parse_ymd(const char *s, int *y, int *m, int *d);
int add_days_ymd(const char *ymd, int days, char out[11]);
int days_between_ymd(const char *a, const char *b);

#endif
