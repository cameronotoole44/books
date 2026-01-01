#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "store.h"
#include "models.h"

#define BOOKS_FILE "data/books.tsv"
#define LOGS_FILE "data/logs.tsv"
#define MAX_LINE 512

int load_books(Book books[], int max_books) {
    FILE *fp = fopen(BOOKS_FILE, "r");
    if (!fp) {
        return 0;
    }

    char line[MAX_LINE];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < max_books) {
        Book b;
        char *token;

        token = strtok(line, "\t");
        if (!token) continue;
        b.id = atoi(token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        strncpy(b.title, token, TITLE_LEN - 1);
        b.title[TITLE_LEN - 1] = '\0';

        token = strtok(NULL, "\t");
        if (!token) continue;
        b.total_pages = atoi(token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        b.current_page = atoi(token);

        token = strtok(NULL, "\t\n");
        if (!token) continue;
        strncpy(b.status, token, STATUS_LEN - 1);
        b.status[STATUS_LEN - 1] = '\0';

        books[count++] = b;
    }

    fclose(fp);
    return count;
}

int save_books(Book books[], int count) {
    FILE *fp = fopen(BOOKS_FILE, "w");
    if (!fp) {
        perror("Failed to open books file for writing");
        return -1;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d\t%s\t%d\t%d\t%s\n",
                books[i].id,
                books[i].title,
                books[i].total_pages,
                books[i].current_page,
                books[i].status);
    }

    fclose(fp);
    return 0;
}

int append_log(const LogEntry *entry) {
    FILE *fp = fopen(LOGS_FILE, "a");
    if (!fp) {
        perror("Failed to open logs file");
        return -1;
    }

    fprintf(fp, "%s\t%d\t%d\t%s\n",
            entry->date,
            entry->book_id,
            entry->pages_read,
            entry->note[0] ? entry->note : "");

    fclose(fp);
    return 0;
}

int load_logs(LogEntry logs[], int max_logs) {
    FILE *fp = fopen(LOGS_FILE, "r");
    if (!fp) return 0;

    char line[MAX_LINE];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < max_logs) {
        LogEntry e;
        char *token;

        token = strtok(line, "\t");
        if (!token) continue;
        strncpy(e.date, token, 10);
        e.date[10] = '\0';

        token = strtok(NULL, "\t");
        if (!token) continue;
        e.book_id = atoi(token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        e.pages_read = atoi(token);

        token = strtok(NULL, "\t\n");
        if (token) {
            strncpy(e.note, token, NOTE_LEN - 1);
            e.note[NOTE_LEN - 1] = '\0';
        } else {
            e.note[0] = '\0';
        }

        logs[count++] = e;
    }

    fclose(fp);
    return count;
}