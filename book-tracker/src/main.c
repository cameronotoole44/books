#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"

static void usage(void) {
    puts("booktracker\n"
         "Usage:\n"
         "  booktracker add \"Title\" <total_pages>\n"
         "  booktracker list\n");
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

    if (strcmp(argv[1], "list") == 0) {
        return cmd_list();
    }

    usage();
    return 1;
}
