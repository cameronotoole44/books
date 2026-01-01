#ifndef STORE_H
#define STORE_H

#include "models.h"

int load_books(Book books[], int max_books);
int save_books(Book books[], int count);
int append_log(const LogEntry *entry);
int load_logs(LogEntry logs[], int max_logs);

#endif
