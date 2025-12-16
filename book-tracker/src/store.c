#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "models.h"

#define BOOKS_FILE "data/books.tsv"
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
        strncpy(b.title, token, TITLE_LEN);
        b.title[TITLE_LEN - 1] = '\0';

        token = strtok(NULL, "\t");
        if (!token) continue;
        b.total_pages = atoi(token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        b.current_page = atoi(token);

        token = strtok(NULL, "\t\n");
        if (!token) continue;
        strncpy(b.status, token, STATUS_LEN);
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
            books[i].status
        );
    }

    fclose(fp);
    return 0;
}
