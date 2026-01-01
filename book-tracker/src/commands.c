#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "commands.h"
#include "store.h"
#include "models.h"
#include "dates.h"
#include "stats.h"

#define MAX_BOOKS 1000

static int next_book_id(Book books[], int count) {
    int max_id = 0;
    for (int i = 0; i < count; i++) {
        if (books[i].id > max_id) max_id = books[i].id;
    }
    return max_id + 1;
}

static int find_book_index_by_id(Book books[], int count, int id) {
    for (int i = 0; i < count; i++) {
        if (books[i].id == id) return i;
    }
    return -1;
}

static int find_current_reading_index(Book books[], int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_READING) == 0) return i;
    }
    return -1;
}

static void seed_rng_once(void) {
    static int seeded = 0;
    if (!seeded) {
        seeded = 1;
        srand((unsigned int)(time(NULL) ^ (uintptr_t)&seeded));
    }
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
        printf("[%d] %s - %d / %d pages - %s\n",
               b->id, b->title, b->current_page, b->total_pages, b->status);
    }

    return 0;
}

int cmd_log(int pages_read) {
    if (pages_read <= 0) {
        fprintf(stderr, "Error: pages_read must be > 0.\n");
        return 1;
    }

    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    
    int cur = find_current_reading_index(books, count);
    if (cur < 0) {
        fprintf(stderr, "No current reading book. Use: booktracker pick\n");
        return 1;
    }

    LogEntry entry;
    if (today_ymd(entry.date) != 0) {
        fprintf(stderr, "Error: failed to get today's date.\n");
        return 1;
    }
    entry.book_id = books[cur].id;
    entry.pages_read = pages_read;
    entry.note[0] = '\0';
    
    if (append_log(&entry) != 0) {
        fprintf(stderr, "Error: failed to save log.\n");
        return 1;
    }

    books[cur].current_page += pages_read;
    if (books[cur].current_page > books[cur].total_pages) {
        books[cur].current_page = books[cur].total_pages;
    }

    if (save_books(books, count) != 0) {
        fprintf(stderr, "Error: failed to save books.\n");
        return 1;
    }

    float pct = 100.0 * books[cur].current_page / books[cur].total_pages;
    printf("✓ Logged %d pages for [%d] %s\n", pages_read, books[cur].id, books[cur].title);
    printf("  Progress: %d/%d pages (%.1f%%)\n", 
           books[cur].current_page, books[cur].total_pages, pct);

    if (books[cur].current_page >= books[cur].total_pages) {
        printf("\n You finished %s! Congrats!\n", books[cur].title);
        strncpy(books[cur].status, STATUS_FINISHED, STATUS_LEN);
        books[cur].status[STATUS_LEN - 1] = '\0';
        save_books(books, count);
    }

    return 0;
}

int cmd_edit_pages(int id, int new_total_pages) {
    if (id <= 0) {
        fprintf(stderr, "Error: id must be > 0.\n");
        return 1;
    }
    if (new_total_pages <= 0) {
        fprintf(stderr, "Error: total_pages must be > 0.\n");
        return 1;
    }

    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    if (count == 0) {
        fprintf(stderr, "No books to edit.\n");
        return 1;
    }

    int idx = find_book_index_by_id(books, count, id);
    if (idx < 0) {
        fprintf(stderr, "Error: no book found with id %d.\n", id);
        return 1;
    }

    books[idx].total_pages = new_total_pages;
    if (books[idx].current_page > books[idx].total_pages) {
        books[idx].current_page = books[idx].total_pages;
    }

    if (save_books(books, count) != 0) {
        fprintf(stderr, "Error: failed to save books.\n");
        return 1;
    }

    printf("Updated: [%d] %s - total_pages=%d\n", books[idx].id, books[idx].title, books[idx].total_pages);
    return 0;
}

int cmd_edit_title(int id, const char *new_title) {
    if (id <= 0) {
        fprintf(stderr, "Error: id must be > 0.\n");
        return 1;
    }
    if (!new_title || new_title[0] == '\0') {
        fprintf(stderr, "Error: new_title cannot be empty.\n");
        return 1;
    }

    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    if (count == 0) {
        fprintf(stderr, "No books to edit.\n");
        return 1;
    }

    int idx = find_book_index_by_id(books, count, id);
    if (idx < 0) {
        fprintf(stderr, "Error: no book found with id %d.\n", id);
        return 1;
    }

    strncpy(books[idx].title, new_title, TITLE_LEN);
    books[idx].title[TITLE_LEN - 1] = '\0';

    if (save_books(books, count) != 0) {
        fprintf(stderr, "Error: failed to save books.\n");
        return 1;
    }

    printf("Updated: [%d] title=\"%s\"\n", books[idx].id, books[idx].title);
    return 0;
}

int cmd_delete(int id) {
    if (id <= 0) {
        fprintf(stderr, "Error: id must be > 0.\n");
        return 1;
    }

    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    if (count == 0) {
        fprintf(stderr, "No books to delete.\n");
        return 1;
    }

    int idx = find_book_index_by_id(books, count, id);
    if (idx < 0) {
        fprintf(stderr, "Error: no book found with id %d.\n", id);
        return 1;
    }

    Book removed = books[idx];

    for (int i = idx; i < count - 1; i++) {
        books[i] = books[i + 1];
    }
    count--;

    if (save_books(books, count) != 0) {
        fprintf(stderr, "Error: failed to save books.\n");
        return 1;
    }

    printf("Deleted: [%d] %s\n", removed.id, removed.title);
    return 0;
}


int cmd_set_current(int id) {
    if (id <= 0) {
        fprintf(stderr, "Error: id must be > 0.\n");
        return 1;
    }

    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    if (count == 0) {
        fprintf(stderr, "No books available.\n");
        return 1;
    }

    int idx = find_book_index_by_id(books, count, id);
    if (idx < 0) {
        fprintf(stderr, "Error: no book found with id %d.\n", id);
        return 1;
    }

    if (strcmp(books[idx].status, STATUS_UNREAD) != 0) {
        fprintf(stderr, "Error: can only set an unread book as current.\n");
        return 1;
    }

    int cur = find_current_reading_index(books, count);
    if (cur >= 0) {
        strncpy(books[cur].status, STATUS_UNREAD, STATUS_LEN);
        books[cur].status[STATUS_LEN - 1] = '\0';
    }

    strncpy(books[idx].status, STATUS_READING, STATUS_LEN);
    books[idx].status[STATUS_LEN - 1] = '\0';

    if (save_books(books, count) != 0) {
        fprintf(stderr, "Error: failed to save books.\n");
        return 1;
    }

    printf("Now reading: [%d] %s\n", books[idx].id, books[idx].title);
    return 0;
}

int cmd_pick(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    if (count == 0) {
        fprintf(stderr, "No books to pick from.\n");
        return 1;
    }

    int cur = find_current_reading_index(books, count);
    if (cur >= 0) {
        printf("Already reading: [%d] %s (%d/%d)\n",
               books[cur].id, books[cur].title, books[cur].current_page, books[cur].total_pages);
        puts("Finish it or use set-current <id> to switch.");
        return 0;
    }

    int unread_idx[MAX_BOOKS];
    int unread_count = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_UNREAD) == 0) {
            unread_idx[unread_count++] = i;
        }
    }

    if (unread_count == 0) {
        puts("No unread books left to pick.");
        return 0;
    }

    seed_rng_once();
    int pick_i = unread_idx[rand() % unread_count];

    strncpy(books[pick_i].status, STATUS_READING, STATUS_LEN);
    books[pick_i].status[STATUS_LEN - 1] = '\0';

    if (save_books(books, count) != 0) {
        fprintf(stderr, "Error: failed to save books.\n");
        return 1;
    }

    printf("Picked: [%d] %s (%d pages)\n", books[pick_i].id, books[pick_i].title, books[pick_i].total_pages);
    return 0;
}


int cmd_projection(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    yearly_projection(books, count);
    return 0;
}

int cmd_finish(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    if (count == 0) {
        fprintf(stderr, "No books available.\n");
        return 1;
    }

    int cur = find_current_reading_index(books, count);
    if (cur < 0) {
        puts("No current reading book. Use: booktracker pick");
        return 0;
    }

    strncpy(books[cur].status, STATUS_FINISHED, STATUS_LEN);
    books[cur].status[STATUS_LEN - 1] = '\0';

    books[cur].current_page = books[cur].total_pages;

    if (save_books(books, count) != 0) {
        fprintf(stderr, "Error: failed to save books.\n");
        return 1;
    }

    printf("Finished: [%d] %s\n", books[cur].id, books[cur].title);
    puts("Add notes");
    return 0;
}
