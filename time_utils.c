#include "time_utils.h"
#include "vocab_core.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


time_t get_current_time(void) {
    return (g_mock_time > 0) ? g_mock_time : time(NULL);
}

void set_mock_time(void) {
    int year, month, day, hour, min;
    printf("请输入模拟时间（年 月 日 时 分）：");
    scanf("%d%d%d%d%d", &year, &month, &day, &hour, &min);
    getchar();
    struct tm tm = {0};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    g_mock_time = mktime(&tm);
    printf("时间已模拟为：%s", ctime(&g_mock_time));
    system("pause");
}

int get_today(void) {
    return time_to_date(get_current_time());
}

int time_to_date(time_t t) {
    if (t == 0) return 0;
    struct tm *tm = localtime(&t);
    return (tm->tm_year+1900)*10000 + (tm->tm_mon+1)*100 + tm->tm_mday;
}

time_t date_to_time_t(int date) {
    struct tm tm = {0};
    tm.tm_year = date/10000 - 1900;
    tm.tm_mon = (date%10000)/100 - 1;
    tm.tm_mday = date%100;
    tm.tm_hour = 12;
    return mktime(&tm);
}

void format_time(time_t t, char *buf, int size) {
    if (t == 0) { strcpy(buf, "未复习"); return; }
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", localtime(&t));
}