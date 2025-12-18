#include "stats.h"
#include "dates.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOGS_FILE "data/logs.tsv"
#define MAX_LINE 1024

double avg_pages_per_day(int book_id, int last_n_days) {
    FILE *fp = fopen(LOGS_FILE, "r");
    if (!fp) return 0.0;

    char today[11];
    int have_today = (today_ymd(today) == 0);

    char line[MAX_LINE];
    int days_counted = 0;
    int pages_sum = 0;

    while (fgets(line, sizeof(line), fp)) {
        char *date = strtok(line, "\t");
        char *bid  = strtok(NULL, "\t");
        char *pgs  = strtok(NULL, "\t");

        if (!date || !bid || !pgs) continue;

        int this_id = atoi(bid);
        if (this_id != book_id) continue;

        if (last_n_days > 0 && have_today) {
            int diff = days_between_ymd(date, today);
            if (diff >= last_n_days) continue;
            if (diff < 0) continue;
        }

        int pages = atoi(pgs);
        if (pages < 0) pages = 0;

        pages_sum += pages;
        days_counted += 1;
    }

    fclose(fp);

    if (days_counted == 0) return 0.0;
    return (double)pages_sum / (double)days_counted;
}
