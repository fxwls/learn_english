// statistics.h
#ifndef STATISTICS_H
#define STATISTICS_H

#include "vocab_core.h"

DailyStat* get_today_daily_stat(void);
void write_daily_log(void);
void show_statistics(void);
void reset_learning_params(void);
void set_daily_goal(void);

#endif