#ifndef STATS_H
#define STATS_H

#include "models.h"

double avg_pages_per_day(int book_id, int last_n_days);
int yearly_projection(Book books[], int book_count);

#endif
