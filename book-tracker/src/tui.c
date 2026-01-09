#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tui.h"
#include "commands.h"
#include "store.h"
#include "models.h"
#include "dates.h"
#include "stats.h"

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
    
    char msg_copy[1024];
    strncpy(msg_copy, message, 1023);
    msg_copy[1023] = '\0';
    
    int row = 4;
    char *line = strtok(msg_copy, "\n");
    while (line != NULL && row < LINES - 4) {
        mvprintw(row, 2, "%s", line);
        row++;
        line = strtok(NULL, "\n");
    }
    
    wait_for_key_curses();
}

static int confirm_yes_no_curses(const char *question) {
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "Confirmation");
    mvhline(2, 1, ACS_HLINE, COLS - 2);
    mvprintw(4, 2, "%s", question);
    mvprintw(6, 2, "[Y]es  [N]o");
    refresh();

    while (1) {
        int ch = getch();
        if (ch == 'y' || ch == 'Y') return 1;
        if (ch == 'n' || ch == 'N') return 0;
    }
}

static void edit_book_curses(Book *book) {
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "Edit Book: [%d] %s", book->id, book->title);
    mvhline(2, 1, ACS_HLINE, COLS - 2);
    
    mvprintw(4, 2, "What would you like to edit?");
    mvprintw(6, 4, "[T] Title");
    mvprintw(7, 4, "[P] Total pages");
    mvprintw(8, 4, "[S] Status");
    mvprintw(9, 4, "[Q] Cancel");
    refresh();

    int ch = getch();
    
    if (ch == 't' || ch == 'T') {
        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "Edit Title");
        mvhline(2, 1, ACS_HLINE, COLS - 2);
        mvprintw(4, 2, "Current title: %s", book->title);
        mvprintw(6, 2, "Enter new title:");
        mvprintw(7, 2, "> ");
        
        echo();
        curs_set(1);
        refresh();
        
        char new_title[TITLE_LEN] = {0};
        getnstr(new_title, TITLE_LEN - 1);
        
        noecho();
        curs_set(0);
        
        if (new_title[0] != '\0') {
            strncpy(book->title, new_title, TITLE_LEN - 1);
            book->title[TITLE_LEN - 1] = '\0';
            show_message_curses("Success", "Title updated!");
        }
    } else if (ch == 'p' || ch == 'P') {
        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "Edit Total Pages");
        mvhline(2, 1, ACS_HLINE, COLS - 2);
        mvprintw(4, 2, "Current total pages: %d", book->total_pages);
        mvprintw(6, 2, "Enter new total pages:");
        mvprintw(7, 2, "> ");
        
        echo();
        curs_set(1);
        refresh();
        
        char buf[64] = {0};
        getnstr(buf, 63);
        int new_pages = atoi(buf);
        
        noecho();
        curs_set(0);
        
        if (new_pages > 0) {
            book->total_pages = new_pages;
            if (book->current_page > book->total_pages) {
                book->current_page = book->total_pages;
            }
            show_message_curses("Success", "Total pages updated!");
        } else {
            show_message_curses("Error", "Invalid page number!");
        }
    }else if (ch == 's' || ch == 'S') {
        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "Edit Status");
        mvhline(2, 1, ACS_HLINE, COLS - 2);
        mvprintw(4, 2, "Current status: %s", book->status);
        mvprintw(6, 2, "Select new status:");
        mvprintw(8, 4, "[U] Unread");
        mvprintw(9, 4, "[R] Reading");
        mvprintw(10, 4, "[F] Finished");
        mvprintw(11, 4, "[Q] Cancel");
        refresh();
        
        int status_ch = getch();
        
        if (status_ch == 'u' || status_ch == 'U') {
            strncpy(book->status, STATUS_UNREAD, STATUS_LEN - 1);
            book->status[STATUS_LEN - 1] = '\0';
            show_message_curses("Success", "Status changed to unread");
        } else if (status_ch == 'r' || status_ch == 'R') {
            strncpy(book->status, STATUS_READING, STATUS_LEN - 1);
            book->status[STATUS_LEN - 1] = '\0';
            show_message_curses("Success", "Status changed to reading");
        } else if (status_ch == 'f' || status_ch == 'F') {
            strncpy(book->status, STATUS_FINISHED, STATUS_LEN - 1);
            book->status[STATUS_LEN - 1] = '\0';
            book->current_page = book->total_pages;
            show_message_curses("Success", "Status changed to finished");
        }
    }
}

static void show_books_curses(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);

    if (count == 0) {
        show_message_curses("No Books", "No books yet. Add one with the 'add' command.");
        return;
    }

    int selected = 0;
    int scroll_offset = 0;
    int needs_save = 0;

    while (1) {
        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "BookTrack - All Books");
        mvprintw(2, 2, "Arrows: navigate | Enter/S: start reading | E: edit | D: delete | Q: back");
        mvhline(3, 1, ACS_HLINE, COLS - 2);

        int start_row = 5;
        int max_display = LINES - 8;
        
        if (selected < scroll_offset) {
            scroll_offset = selected;
        }
        if (selected >= scroll_offset + max_display) {
            scroll_offset = selected - max_display + 1;
        }

        for (int i = 0; i < max_display && (scroll_offset + i) < count; i++) {
            int book_idx = scroll_offset + i;
            Book *b = &books[book_idx];
            float pct = (b->total_pages > 0) ? (100.0 * b->current_page / b->total_pages) : 0;
            
            if (book_idx == selected) attron(A_REVERSE);
            
            mvprintw(start_row + i, 2, "[%d] %-35s %4d/%-4d (%3.0f%%) %s",
                     b->id, b->title,
                     b->current_page, b->total_pages, pct, b->status);
            
            if (book_idx == selected) attroff(A_REVERSE);
        }

        if (scroll_offset > 0) {
            mvprintw(4, COLS - 10, "^ More ^");
        }
        if (scroll_offset + max_display < count) {
            mvprintw(LINES - 3, COLS - 10, "v More v");
        }

        refresh();

        int ch = getch();

        if (ch == 'q' || ch == 'Q') {
            if (needs_save) {
                save_books(books, count);
            }
            break;
        }

        if (ch == KEY_UP) {
            if (selected > 0) {
                selected--;
            }
        } else if (ch == KEY_DOWN) {
            if (selected < count - 1) {
                selected++;
            }
        } else if (ch == '\n' || ch == KEY_ENTER || ch == 's' || ch == 'S') {
            if (strcmp(books[selected].status, STATUS_UNREAD) != 0) {
                show_message_curses("Error", "Can only start reading unread books!");
                continue;
            }

            for (int i = 0; i < count; i++) {
                if (strcmp(books[i].status, STATUS_READING) == 0) {
                    strncpy(books[i].status, STATUS_UNREAD, STATUS_LEN);
                    books[i].status[STATUS_LEN - 1] = '\0';
                }
            }

            strncpy(books[selected].status, STATUS_READING, STATUS_LEN);
            books[selected].status[STATUS_LEN - 1] = '\0';
            
            save_books(books, count);
            
            char msg[256];
            snprintf(msg, 256, "Now reading: [%d] %s", books[selected].id, books[selected].title);
            show_message_curses("Success", msg);
            
        } else if (ch == 'e' || ch == 'E') {
            edit_book_curses(&books[selected]);
            needs_save = 1;
            
        } else if (ch == 'd' || ch == 'D') {
            char question[256];
            snprintf(question, 256, "Delete [%d] %s?", books[selected].id, books[selected].title);
            
            if (confirm_yes_no_curses(question)) {
                for (int i = selected; i < count - 1; i++) {
                    books[i] = books[i + 1];
                }
                count--;
                
                if (selected >= count && count > 0) {
                    selected = count - 1;
                }
                
                save_books(books, count);
                show_message_curses("Success", "Book deleted!");
                
                if (count == 0) {
                    show_message_curses("No Books", "All books deleted!");
                    break;
                }
            }
        }
    }
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

static void add_books_menu_curses(void) {
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "Add Book(s)");
    mvhline(2, 1, ACS_HLINE, COLS - 2);
    
    mvprintw(4, 2, "What would you like to do?");
    mvprintw(6, 4, "[N] Add a new book to library");
    mvprintw(7, 4, "[R] Start reading an existing book");
    mvprintw(8, 4, "[Q] Cancel");
    refresh();

    int ch = getch();
    
    if (ch == 'n' || ch == 'N') {
        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "Add New Book");
        mvhline(2, 1, ACS_HLINE, COLS - 2);
        
        mvprintw(4, 2, "Enter book title:");
        mvprintw(5, 2, "> ");
        
        echo();
        curs_set(1);
        refresh();
        
        char title[TITLE_LEN] = {0};
        getnstr(title, TITLE_LEN - 1);
        
        if (title[0] == '\0') {
            noecho();
            curs_set(0);
            show_message_curses("Cancelled", "No title entered.");
            return;
        }
        
        mvprintw(7, 2, "Enter total pages:");
        mvprintw(8, 2, "> ");
        refresh();
        
        char buf[64] = {0};
        getnstr(buf, 63);
        int total_pages = atoi(buf);
        
        noecho();
        curs_set(0);
        
        if (total_pages <= 0) {
            show_message_curses("Error", "Invalid page count!");
            return;
        }
        
        Book books[MAX_BOOKS];
        int count = load_books(books, MAX_BOOKS);
        
        if (count >= MAX_BOOKS) {
            show_message_curses("Error", "Book limit reached!");
            return;
        }
        
        Book b;
        int max_id = 0;
        for (int i = 0; i < count; i++) {
            if (books[i].id > max_id) max_id = books[i].id;
        }
        b.id = max_id + 1;
        
        strncpy(b.title, title, TITLE_LEN - 1);
        b.title[TITLE_LEN - 1] = '\0';
        b.total_pages = total_pages;
        b.current_page = 0;
        strncpy(b.status, STATUS_UNREAD, STATUS_LEN - 1);
        b.status[STATUS_LEN - 1] = '\0';
        
        books[count++] = b;
        
        if (save_books(books, count) != 0) {
            show_message_curses("Error", "Failed to save book!");
            return;
        }
        
        char msg[256];
        snprintf(msg, 256, "Added: [%d] %s (%d pages)", b.id, b.title, b.total_pages);
        show_message_curses("Success", msg);
        
    } else if (ch == 'r' || ch == 'R') {
        Book books[MAX_BOOKS];
        int count = load_books(books, MAX_BOOKS);
        
        if (count == 0) {
            show_message_curses("Error", "No books available.");
            return;
        }

        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "Start Reading Book");
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
            show_message_curses("Error", "Can only start reading unread books.");
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
}

static void show_projection_curses(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "Year-End Projection (2026)");
    mvhline(2, 1, ACS_HLINE, COLS - 2);
    
    FILE *fp = fopen("data/logs.tsv", "r");
    if (!fp) {
        mvprintw(4, 2, "No reading logs found yet.");
        mvprintw(5, 2, "Start logging your reading progress!");
        wait_for_key_curses();
        return;
    }
    
    char dates[365][11];
    int date_count = 0;
    int total_pages_logged = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), fp)) {
        char line_copy[1024];
        strncpy(line_copy, line, sizeof(line_copy));
        
        char *date = strtok(line_copy, "\t");
        char *bid = strtok(NULL, "\t");
        char *pgs = strtok(NULL, "\t");
        
        if (!date || !pgs) continue;
        
        int pages = atoi(pgs);
        total_pages_logged += pages;
        
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
        mvprintw(4, 2, "No reading logs found yet.");
        mvprintw(5, 2, "Start logging your reading progress!");
        wait_for_key_curses();
        return;
    }
    
    double pages_per_day = (double)total_pages_logged / date_count;
    
    char today[11];
    char target[11] = "2026-12-31";
    if (today_ymd(today) != 0) {
        mvprintw(4, 2, "Error: couldn't get today's date");
        wait_for_key_curses();
        return;
    }
    
    int days_left = days_between_ymd(today, target);
    if (days_left <= 0) {
        mvprintw(4, 2, "We're past 2026! Time to set a new goal.");
        wait_for_key_curses();
        return;
    }
    
    int total_pages_readable = (int)(pages_per_day * days_left);
    
    int books_finishable = 0;
    int pages_counted = 0;
    int unread_count = 0;
    
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_UNREAD) == 0) {
            unread_count++;
            if (pages_counted + books[i].total_pages <= total_pages_readable) {
                pages_counted += books[i].total_pages;
                books_finishable++;
            }
        }
    }
    
    mvprintw(4, 2, "Reading speed: %.1f pages/day", pages_per_day);
    mvprintw(5, 2, "Days until 2026-12-31: %d days", days_left);
    mvprintw(6, 2, "On pace to read: %d pages", total_pages_readable);
    
    if (books_finishable == unread_count) {
        mvprintw(8, 2, "All %d books finishable", unread_count);
    } else {
        mvprintw(8, 2, "%d of %d books finishable", books_finishable, unread_count);
    }
    
    wait_for_key_curses();
}

static void show_stats_curses(void) {
    Book books[MAX_BOOKS];
    int count = load_books(books, MAX_BOOKS);
    
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "Reading Statistics");
    mvhline(2, 1, ACS_HLINE, COLS - 2);
    
    int unread = 0, reading = 0, finished = 0;
    int total_pages = 0, pages_read = 0;
    
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_UNREAD) == 0) unread++;
        else if (strcmp(books[i].status, STATUS_READING) == 0) reading++;
        else if (strcmp(books[i].status, STATUS_FINISHED) == 0) finished++;
        
        total_pages += books[i].total_pages;
        pages_read += books[i].current_page;
    }
    
    float completion = (total_pages > 0) ? (100.0 * pages_read / total_pages) : 0;
    
    mvprintw(4, 2, "Total Books: %d", count);
    mvprintw(5, 4, "Unread: %d", unread);
    mvprintw(6, 4, "Reading: %d", reading);
    mvprintw(7, 4, "Finished: %d", finished);
    
    mvprintw(9, 2, "Progress:");
    mvprintw(10, 4, "Total pages: %d", total_pages);
    mvprintw(11, 4, "Pages read: %d (%.1f%%)", pages_read, completion);
    
    int cur = -1;
    for (int i = 0; i < count; i++) {
        if (strcmp(books[i].status, STATUS_READING) == 0) {
            cur = i;
            break;
        }
    }
    
    if (cur >= 0) {
        mvprintw(13, 2, "Currently Reading:");
        mvprintw(14, 4, "[%d] %s", books[cur].id, books[cur].title);
        float pct = 100.0 * books[cur].current_page / books[cur].total_pages;
        mvprintw(15, 4, "%d/%d pages (%.1f%%)", 
                 books[cur].current_page, books[cur].total_pages, pct);
        
        double speed = avg_pages_per_day(books[cur].id, 7);
        if (speed > 0) {
            int remaining = books[cur].total_pages - books[cur].current_page;
            int days_left = (int)(remaining / speed);
            mvprintw(17, 4, "Reading speed: %.1f pages/day (last 7 days)", speed);
            mvprintw(18, 4, "Estimated completion: %d days", days_left);
        }
    }
    
    wait_for_key_curses();
}

int cmd_tui(void) {
    const char *items[] = {
        "Books",
        "Log pages",
        "Pick book",
        "Add book(s)",
        "Finish book",
        "Projection",
        "Stats",
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
            if (choice == 7) {
                break;
            } else if (choice == 0) {
                show_books_curses();
            } else if (choice == 1) {
                log_pages_curses();
            } else if (choice == 2) {
                pick_random_curses();
            } else if (choice == 3) {
                add_books_menu_curses();
            } else if (choice == 4) {
                finish_current_curses();
            } else if (choice == 5) {
                show_projection_curses();
            } else if (choice == 6) {
                show_stats_curses();
            }
        }
    }

    endwin();
    return 0;
}