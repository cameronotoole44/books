#include <curses.h>
#include <stdlib.h>
#include <string.h>

#include "tui.h"
#include "commands.h"

static void wait_for_enter(void) {
    puts("\nPress ENTER to continue...");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static int prompt_int_curses(const char *label) {
    echo();
    curs_set(1);

    int row = LINES - 4;
    mvhline(row, 1, ' ', COLS - 2);
    mvhline(row + 1, 1, ' ', COLS - 2);

    mvprintw(row, 2, "%s", label);
    mvprintw(row + 1, 2, "> ");
    refresh();

    char buf[64] = {0};
    getnstr(buf, (int)sizeof(buf) - 1);

    noecho();
    curs_set(0);

    return atoi(buf);
}

int cmd_tui(void) {
    const char *items[] = {
        "List books",
        "Pick random unread",
        "Set current (by id)",
        "Finish current",
        "Edit title (by id)",
        "Edit total pages (by id)",
        "Delete book (by id)",
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
            int id = 0;
            int pages = 0;
            char title[256] = {0};

            if (choice == 2) {
                id = prompt_int_curses("Enter book id to set as current:");
            } else if (choice == 4) {
                id = prompt_int_curses("Enter book id to edit title:");
                echo();
                curs_set(1);
                int row = LINES - 4;
                mvhline(row, 1, ' ', COLS - 2);
                mvhline(row + 1, 1, ' ', COLS - 2);
                mvprintw(row, 2, "Enter new title:");
                mvprintw(row + 1, 2, "> ");
                refresh();
                getnstr(title, 255);
                noecho();
                curs_set(0);
            } else if (choice == 5) {
                id = prompt_int_curses("Enter book id to edit total pages:");
                pages = prompt_int_curses("Enter new total pages:");
            } else if (choice == 6) 
                id = prompt_int_curses("Enter book id to delete:");
            }

            endwin();

            if (choice == 0) {
                cmd_list();
            } else if (choice == 1) {
                cmd_pick();
            } else if (choice == 2) {
                cmd_set_current(id);
            } else if (choice == 3) {
                cmd_finish();
            } else if (choice == 4) {
                cmd_edit_title(id, title);
            } else if (choice == 5) {
                cmd_edit_pages(id, pages);
            } else if (choice == 6) {
                cmd_delete(id);
            } else if (choice == 7) {
                break;
            }

            wait_for_enter();

            initscr();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
            curs_set(0);
        }
    }

    endwin();
    return 0;
}
