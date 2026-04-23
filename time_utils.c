#include "time_utils.h"
#include "vocab_core.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


time_t get_current_time(void) {
    return (g_mock_time > 0) ? g_mock_time : time(NULL);
}

void set_mock_time(void) {
    printf("\n=====设置模拟时间=====\n");
    time_t now_t = get_current_time();
    printf("当前时间: %s\n", ctime(&now_t));
    printf("1. 增加指定小时\n");
    printf("2. 增加指定分钟\n");
    printf("3. 增加指定秒数\n");
    printf("4. 增加指定天数\n");
    printf("5. 重置为真实时间\n");
    printf("请输入你的选择(1-5): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        while(getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    if (choice < 1 || choice > 5) {
        printf("无效的选择，将返回。\n");
        return;
    }
    if (g_mock_time == 0) {
        g_mock_time = time(NULL);
    }
    int delta;
    switch(choice) {
        case 1:
            printf("增加的小时数: ");
            scanf("%d", &delta);
            while (getchar() != '\n');
            g_mock_time += delta * 3600;
            break;
        case 2:
            printf("增加的分钟数: ");
            scanf("%d", &delta);
            while (getchar() != '\n');
            g_mock_time += delta * 60;
            break;
        case 3:
            printf("增加的秒数: ");
            scanf("%d", &delta);
            while (getchar() != '\n');
            g_mock_time += delta;
            break;
        case 4:
            printf("增加的天数: ");
            scanf("%d", &delta);
            while (getchar() != '\n');
            g_mock_time += delta * 86400;
            break;
        case 5:
            g_mock_time = 0;
            break;
        default:
            printf("无效的选择，将返回。\n");
            return;
    }
    time_t new_time = get_current_time();
    printf("模拟时间已更新为: %s\n", ctime(&new_time));
    printf("\n按回车键返回主菜单...");
    getchar();
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