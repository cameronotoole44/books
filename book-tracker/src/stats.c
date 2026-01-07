#include "stats.h"
#include "dates.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOGS_FILE "data/logs.tsv"
#define MAX_LINE 1024

static int count_status(Book books[], int count, const char *status) {
    int n = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, status) == 0) n++;
    }
    return n;
}

double avg_pages_per_day(int book_id, int last_n_days) {
    FILE *fp = fopen(LOGS_FILE, "r");
    if (!fp) return 0.0;

    char today[11];
    int have_today = (today_ymd(today) == 0);

    char dates[365][11];
    int date_count = 0;
    int pages_sum = 0;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        char line_copy[MAX_LINE];
        strncpy(line_copy, line, MAX_LINE);
        
        char *date = strtok(line_copy, "\t");
        char *bid  = strtok(NULL, "\t");
        char *pgs  = strtok(NULL, "\t");

        if (!date || !bid || !pgs) continue;

        int this_id = atoi(bid);
        if (this_id != book_id) continue;

        if (last_n_days > 0 && have_today) {
            int diff = days_between_ymd(date, today);
            if (diff >= last_n_days || diff < 0) continue;
        }

        int pages = atoi(pgs);
        if (pages < 0) pages = 0;
        pages_sum += pages;

        int found = 0;
        for (int i = 0; i < date_count; i++) {
            if (strcmp(dates[i], date) == 0) {
                found = 1;
                break;
            }
        }
        if (!found && date_count < 365) {
            strncpy(dates[date_count], date, 10);
            dates[date_count][10] = '\0';
            date_count++;
        }
    }

    fclose(fp);

    if (date_count == 0) return 0.0;
    return (double)pages_sum / (double)date_count;
}

int yearly_projection(Book books[], int book_count) {
    FILE *fp = fopen(LOGS_FILE, "r");
    if (!fp) {
        printf("\nNo reading logs found yet. Start logging your reading progress!\n\n");
        return 0;
    }

    char dates[365][11];
    int date_count = 0;
    int total_pages = 0;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        char line_copy[MAX_LINE];
        strncpy(line_copy, line, MAX_LINE);
        
        char *date = strtok(line_copy, "\t");
        char *bid  = strtok(NULL, "\t");
        char *pgs  = strtok(NULL, "\t");

        if (!date || !pgs) continue;

        int pages = atoi(pgs);
        total_pages += pages;

        int found = 0;
        for (int i = 0; i < date_count; i++) {
            if (strcmp(dates[i], date) == 0) {
                found = 1;
                break;
            }
        }
        if (!found && date_count < 365) {
            strncpy(dates[date_count], date, 10);
            dates[date_count][10] = '\0';
            date_count++;
        }
    }
    fclose(fp);

    if (date_count == 0) {
        printf("\nNo reading logs found yet. Start logging your reading progress!\n\n");
        return 0;
    }

    double pages_per_day = (double)total_pages / date_count;
    
    char target_date[11];
    snprintf(target_date, 11, "2026-12-31");
    
    char today[11];
    if (today_ymd(today) != 0) {
        printf("\nError: couldn't get today's date\n\n");
        return 0;
    }
    
    int days_left = days_between_ymd(today, target_date);
    if (days_left <= 0) {
        printf("\nWe're past 2026! Time to set a new goal.\n\n");
        return 0;
    }

    int total_pages_readable = (int)(pages_per_day * days_left);

    int books_finishable = 0;
    int pages_counted = 0;
    int unread_count = count_status(books, book_count, STATUS_UNREAD);

    for (int i = 0; i < book_count; i++) {
        if (strcmp(books[i].status, STATUS_UNREAD) == 0) {
            if (pages_counted + books[i].total_pages <= total_pages_readable) {
                pages_counted += books[i].total_pages;
                books_finishable++;
            }
        }
    }

    printf("\n Projection (2026)\n");
    printf("========================================\n");
    printf("  Reading speed: %.1f pages/day\n", pages_per_day);
    printf("  Days until 2026-12-31: %d days\n", days_left);
    printf("  On pace to read: %d pages\n\n", total_pages_readable);
    
    if (books_finishable == unread_count) {
        printf("  All %d books finishable\n\n", unread_count);
    } else {
        printf("  %d of %d books finishable\n\n", books_finishable, unread_count);
    }

    return books_finishable;
}