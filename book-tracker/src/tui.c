#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tui.h"
#include "commands.h"
#include "store.h"
#include "models.h"
#include "dates.h"

#define MAX_BOOKS 1000

static void wait_for_key_curses(void) {
    mvprintw(LINES - 2, 2, "Press any key to continue...");
    refresh();
    getch();
}

static void show_message_curses(const char *title, const char *message) {
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "%s", title);
    mvhline(2, 1, ACS_HLINE, COLS - 2);
    mvprintw(4, 2, "%s", message);
    wait_for_key_curses();
}

static void show_books_curses(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);

    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "BookTrack - All Books");
    mvhline(2, 1, ACS_HLINE, COLS - 2);

    if (count == 0) {
        mvprintw(4, 2, "No books yet. Add one with the 'add' command.");
        wait_for_key_curses();
        return;
    }

    int start_row = 4;
    int max_display = LINES - 6;

    for (int i = 0; i < count && i < max_display; i++) {
        Book *b = &books[i];
        float pct = (b->total_pages > 0) ? (100.0 * b->current_page / b->total_pages) : 0;
        
        mvprintw(start_row + i, 2, "[%d] %s", b->id, b->title);
        mvprintw(start_row + i, 40, "%d/%d pages (%.0f%%) - %s",
                 b->current_page, b->total_pages, pct, b->status);
    }

    if (count > max_display) {
        mvprintw(LINES - 3, 2, "... and %d more books (screen too small)", count - max_display);
    }

    wait_for_key_curses();
}

static void log_pages_curses(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    
    int cur = -1;
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_READING) == 0) {
            cur = i;
            break;
        }
    }

    if (cur < 0) {
        show_message_curses("Log Pages", "No current reading book. Use 'Pick random unread' first.");
        return;
    }

    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "Log Pages Read");
    mvhline(2, 1, ACS_HLINE, COLS - 2);
    mvprintw(4, 2, "Currently reading: [%d] %s", books[cur].id, books[cur].title);
    mvprintw(5, 2, "Current page: %d / %d", books[cur].current_page, books[cur].total_pages);
    
    echo();
    curs_set(1);
    mvprintw(7, 2, "What page are you on now? ");
    refresh();
    
    char buf[64] = {0};
    getnstr(buf, 63);
    int ending_page = atoi(buf);
    
    noecho();
    curs_set(0);

    if (ending_page <= 0 || ending_page > books[cur].total_pages) {
        show_message_curses("Error", "Invalid page number!");
        return;
    }

    if (ending_page <= books[cur].current_page) {
        show_message_curses("Error", "Ending page must be greater than current page!");
        return;
    }

    int pages_read = ending_page - books[cur].current_page;
    
    LogEntry entry;
    if (today_ymd(entry.date) != 0) {
        show_message_curses("Error", "Failed to get today's date.");
        return;
    }
    entry.book_id = books[cur].id;
    entry.pages_read = pages_read;
    entry.note[0] = '\0';
    
    if (append_log(&entry) != 0) {
        show_message_curses("Error", "Failed to save log.");
        return;
    }

    books[cur].current_page = ending_page;
    if (save_books(books, count) != 0) {
        show_message_curses("Error", "Failed to save books.");
        return;
    }

    float pct = 100.0 * books[cur].current_page / books[cur].total_pages;
    char msg[256];
    snprintf(msg, 256, "Logged %d pages for [%d] %s\nProgress: %d/%d pages (%.1f%%)",
             pages_read, books[cur].id, books[cur].title,
             books[cur].current_page, books[cur].total_pages, pct);

    if (books[cur].current_page >= books[cur].total_pages) {
        strncpy(books[cur].status, STATUS_FINISHED, STATUS_LEN);
        books[cur].status[STATUS_LEN - 1] = '\0';
        save_books(books, count);
        strncat(msg, "\n\nYou finished this book! Congrats!", 256 - strlen(msg) - 1);
    }

    show_message_curses("Success", msg);
}

static void pick_random_curses(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    
    if (count == 0) {
        show_message_curses("Error", "No books to pick from.");
        return;
    }

    int cur = -1;
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_READING) == 0) {
            cur = i;
            break;
        }
    }

    if (cur >= 0) {
        char msg[256];
        snprintf(msg, 256, "Already reading: [%d] %s (%d/%d)\nFinish it or use 'Add another book' to switch.",
                 books[cur].id, books[cur].title, books[cur].current_page, books[cur].total_pages);
        show_message_curses("Already Reading", msg);
        return;
    }

    int unread_idx[MAX_BOOKS];
    int unread_count = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_UNREAD) == 0) {
            unread_idx[unread_count++] = i;
        }
    }

    if (unread_count == 0) {
        show_message_curses("No Unread Books", "No unread books left to pick.");
        return;
    }

    srand((unsigned int)time(NULL));
    int pick_i = unread_idx[rand() % unread_count];

    strncpy(books[pick_i].status, STATUS_READING, STATUS_LEN);
    books[pick_i].status[STATUS_LEN - 1] = '\0';

    if (save_books(books, count) != 0) {
        show_message_curses("Error", "Failed to save books.");
        return;
    }

    char msg[256];
    snprintf(msg, 256, "Picked: [%d] %s (%d pages)\nHappy reading!",
             books[pick_i].id, books[pick_i].title, books[pick_i].total_pages);
    show_message_curses("Book Picked", msg);
}

static void finish_current_curses(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    
    if (count == 0) {
        show_message_curses("Error", "No books available.");
        return;
    }

    int cur = -1;
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_READING) == 0) {
            cur = i;
            break;
        }
    }

    if (cur < 0) {
        show_message_curses("No Current Book", "No current reading book. Use 'Pick random unread' first.");
        return;
    }

    strncpy(books[cur].status, STATUS_FINISHED, STATUS_LEN);
    books[cur].status[STATUS_LEN - 1] = '\0';
    books[cur].current_page = books[cur].total_pages;

    if (save_books(books, count) != 0) {
        show_message_curses("Error", "Failed to save books.");
        return;
    }

    char msg[256];
    snprintf(msg, 256, "Finished: [%d] %s\nCongratulations on finishing this book!",
             books[cur].id, books[cur].title);
    show_message_curses("Book Finished", msg);
}

static void set_current_curses(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    
    if (count == 0) {
        show_message_curses("Error", "No books available.");
        return;
    }

    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "Add Another Book");
    mvhline(2, 1, ACS_HLINE, COLS - 2);
    mvprintw(4, 2, "Enter the book ID you want to start reading:");
    mvprintw(5, 2, "> ");
    
    echo();
    curs_set(1);
    refresh();
    
    char buf[64] = {0};
    getnstr(buf, 63);
    int id = atoi(buf);
    
    noecho();
    curs_set(0);

    if (id <= 0) {
        show_message_curses("Error", "Invalid book ID.");
        return;
    }

    int idx = -1;
    for (int i = 0; i < count; i++) {
        if (books[i].id == id) {
            idx = i;
            break;
        }
    }

    if (idx < 0) {
        show_message_curses("Error", "Book not found with that ID.");
        return;
    }

    if (strcmp(books[idx].status, STATUS_UNREAD) != 0) {
        show_message_curses("Error", "Can only set an unread book as current.");
        return;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_READING) == 0) {
            strncpy(books[i].status, STATUS_UNREAD, STATUS_LEN);
            books[i].status[STATUS_LEN - 1] = '\0';
        }
    }

    strncpy(books[idx].status, STATUS_READING, STATUS_LEN);
    books[idx].status[STATUS_LEN - 1] = '\0';

    if (save_books(books, count) != 0) {
        show_message_curses("Error", "Failed to save books.");
        return;
    }

    char msg[256];
    snprintf(msg, 256, "Now reading: [%d] %s", books[idx].id, books[idx].title);
    show_message_curses("Success", msg);
}

int cmd_tui(void) {
    const char *items[] = {
        "Books",
        "Log pages",
        "Pick book",
        "Add (multiple) book(s)",
        "Finish book",
        "Quit"
    };
    const int n_items = (int)(sizeof(items) / sizeof(items[0]));
    int choice = 0;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    while (1) {
        clear();
        box(stdscr, 0, 0);

        mvprintw(1, 2, "BookTrack 1.0");
        mvprintw(2, 2, "Arrows: move   Enter: select   q: quit");
        mvhline(3, 1, ACS_HLINE, COLS - 2);

        for (int i = 0; i < n_items; i++) {
            int row = 5 + i;
            if (i == choice) attron(A_REVERSE);
            mvprintw(row, 4, "%s", items[i]);
            if (i == choice) attroff(A_REVERSE);
        }

        refresh();

        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;

        if (ch == KEY_UP) {
            choice = (choice - 1 + n_items) % n_items;
            continue;
        }
        if (ch == KEY_DOWN) {
            choice = (choice + 1) % n_items;
            continue;
        }

        if (ch == '\n' || ch == KEY_ENTER) {
            if (choice == 5) {  // Quit
                break;
            } else if (choice == 0) {  // List books
                show_books_curses();
            } else if (choice == 1) {  // Log pages
                log_pages_curses();
            } else if (choice == 2) {  // Pick random
                pick_random_curses();
            } else if (choice == 3) {  // Add another book
                set_current_curses();
            } else if (choice == 4) {  // Finish current
                finish_current_curses();
            }
        }
    }

    endwin();
    return 0;
}