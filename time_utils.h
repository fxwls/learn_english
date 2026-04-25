// time_utils.h
#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>

time_t get_current_time(void);
void set_mock_time(void);
int get_today(void);
int time_to_date(time_t t);
time_t date_to_time_t(int date);
void format_time(time_t t, char *buf, int buf_size);
static int copy_file(const char *src, const char *dst);
void cleanup_mock_mode(void);

#endif