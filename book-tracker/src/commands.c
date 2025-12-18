#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "store.h"
#include "models.h"

#define MAX_BOOKS 1000

static int next_book_id(Book books[], int count) {
    int max_id = 0;
    for (int i = 0; i < count; i++) {
        if (books[i].id > max_id) max_id = books[i].id;
    }
    return max_id + 1;
}

int cmd_add(const char *title, int total_pages) {
    if (!title || title[0] == '\0') {
        fprintf(stderr, "Error: title cannot be empty.\n");
        return 1;
    }
    if (total_pages <= 0) {
        fprintf(stderr, "Error: total_pages must be > 0.\n");
        return 1;
    }

    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);

    if (count >= MAX_BOOKS) {
        fprintf(stderr, "Error: book limit reached (%d).\n", MAX_BOOKS);
        return 1;
    }

    Book b;
    b.id = next_book_id(books, count);

    strncpy(b.title, title, TITLE_LEN);
    b.title[TITLE_LEN - 1] = '\0';

    b.total_pages = total_pages;
    b.current_page = 0;

    strncpy(b.status, STATUS_UNREAD, STATUS_LEN);
    b.status[STATUS_LEN - 1] = '\0';

    books[count++] = b;

    if (save_books(books, count) != 0) {
        fprintf(stderr, "Error: failed to save books.\n");
        return 1;
    }

    printf("Added: [%d] %s (%d pages) - %s\n", b.id, b.title, b.total_pages, b.status);
    return 0;
}

int cmd_list(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);

    if (count == 0) {
        puts("No books yet. Add one with: booktracker add \"Title\" <pages>");
        return 0;
    }

    for (int i = 0; i < count; i++) {
        const Book *b = &books[i];
        printf("[%d] %s — %d / %d pages — %s\n",
               b->id, b->title, b->current_page, b->total_pages, b->status);
    }

    return 0;
}
