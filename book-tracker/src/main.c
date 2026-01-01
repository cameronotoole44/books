#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tui.h"
#include "commands.h"

static void usage(void) {
    puts("booktrack 1.0\n"
         "Usage:\n"
         "  booktracker add \"Title\" <total_pages>\n"
         "  booktracker list\n"
         "  booktracker log <pages_read>\n  "
         "  booktracker edit-pages <id> <total_pages>\n"
         "  booktracker edit-title <id> \"New Title\"\n"
         "  booktracker delete <id>\n"
         "  booktracker pick\n"
         "  booktracker set-current <id>\n"
         "  booktracker finish\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage();
        return 1;
    }

    if (strcmp(argv[1], "add") == 0) {
        if (argc != 4) {
            usage();
            return 1;
        }
        const char *title = argv[2];
        int total_pages = atoi(argv[3]);
        return cmd_add(title, total_pages);
    }

    if (strcmp(argv[1], "tui") == 0) {
        return cmd_tui();
    }


    if (strcmp(argv[1], "list") == 0) {
        return cmd_list();
    }

    if (strcmp(argv[1], "log") == 0) {
    if (argc != 3) {
        usage();
        return 1;
    }
    int pages = atoi(argv[2]);
    return cmd_log(pages);
}
    if (strcmp(argv[1], "edit-pages") == 0) {
        if (argc != 4) {
            usage();
            return 1;
        }
        int id = atoi(argv[2]);
        int pages = atoi(argv[3]);
        return cmd_edit_pages(id, pages);
    }

    if (strcmp(argv[1], "edit-title") == 0) {
        if (argc != 4) {
            usage();
            return 1;
        }
        int id = atoi(argv[2]);
        const char *title = argv[3];
        return cmd_edit_title(id, title);
    }

    if (strcmp(argv[1], "delete") == 0) {
        if (argc != 3) {
            usage();
            return 1;
        }
        int id = atoi(argv[2]);
        return cmd_delete(id);
    }

    if (strcmp(argv[1], "pick") == 0) {
        if (argc != 2) {
            usage();
            return 1;
        }
        return cmd_pick();
    }

    if (strcmp(argv[1], "set-current") == 0) {
        if (argc != 3) {
            usage();
            return 1;
        }
        int id = atoi(argv[2]);
        return cmd_set_current(id);
    }

    if (strcmp(argv[1], "finish") == 0) {
        if (argc != 2) {
            usage();
            return 1;
        }
        return cmd_finish();
    }

    usage();
    return 1;
}
