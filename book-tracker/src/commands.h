#ifndef COMMANDS_H
#define COMMANDS_H

int cmd_add(const char *title, int total_pages);
int cmd_list(void);

int cmd_edit_pages(int id, int new_total_pages);
int cmd_edit_title(int id, const char *new_title);
int cmd_delete(int id);
int cmd_log(int ending_page);

int cmd_pick(void);
int cmd_set_current(int id);
int cmd_projection(void);
int cmd_finish(void);

#endif

