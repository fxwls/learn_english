// time_utils.c
#include "time_utils.h"
#include "vocab_core.h"
#include "ui_utils.h"
#include "file_io.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


    time_t get_current_time(void) {
        return (g_mock_time > 0) ? g_mock_time : time(NULL);
    }

    static int copy_file(const char *src, const char *dst) {
        FILE *fsrc = fopen(src, "rb");
        if (!fsrc) return -1;
        FILE *fdst = fopen(dst, "wb");
        if (!fdst) {
            fclose(fsrc);
            return -1;
        }
        unsigned char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), fsrc)) > 0) {
            fwrite(buf, 1, n, fdst);
        }
        fclose(fsrc);
        fclose(fdst);
        return 0;
    }


    void set_mock_time(void) {
        printf("\n");
        set_color(COLOR_TITLE);
        printf("===================================================\n");
        printf("                   模拟时间设置\n");
        printf("===================================================\n");
        set_color(COLOR_DEFAULT);
        time_t now_t = get_current_time();
        printf("当前时间: %s\n", ctime(&now_t));
        printf("1. 增加指定小时\n");
        printf("2. 增加指定分钟\n");
        printf("3. 增加指定秒数\n");
        printf("4. 增加指定天数\n");
        printf("5. 重置为真实时间\n");
        set_color(COLOR_WARN);
        printf("请输入你的选择(1-5): ");
        set_color(COLOR_DEFAULT);

        char buf[32];
        if (!safe_input(buf, sizeof(buf))) {
            printf("输入无效，将返回。\n");
            return;
        }
        int choice = atoi(buf);
        if (choice < 1 || choice > 5) {
            printf("无效的选择，将返回。\n");
            return;
        }

        // 选择5：重置为真实时间
        if (choice == 5) {
            if (g_mock_mode) {
                printf("正在退出模拟时间模式，恢复原始词库...\n");

                cleanup_mock_mode();
                load_vocab();
                printf("已恢复真实时间模式。\n");
            } else {
                g_mock_time = 0;
                printf("已重置为真实时间。\n");
            }
            printf("\n按回车键返回主菜单...");
            safe_input(buf, sizeof(buf));
            return;
        }

        // 选择1-4：增加时间
        // 如果尚未进入模拟模式，则先创建沙盒
        if (!g_mock_mode) {
            printf("首次使用模拟时间，正在创建独立模拟词库...\n");
            // 保存原始词库文件名
            strcpy(g_original_vocab_file, g_current_vocab_file);
            // 构造临时文件名
            char tmp_file[300];
            char *dot = strstr(g_original_vocab_file, ".dat");
            if (dot) {
                int len = dot - g_original_vocab_file;
                strncpy(tmp_file, g_original_vocab_file, len);
                tmp_file[len] = '\0';
                strcat(tmp_file, "_mock.dat");
            } else {
                snprintf(tmp_file, sizeof(tmp_file), "%s_mock.dat", g_original_vocab_file);
            }
            // 复制词库
            if (copy_file(g_original_vocab_file, tmp_file) != 0) {
                printf("错误：无法创建模拟词库副本！请检查磁盘空间或文件权限。\n");
                printf("按回车键返回...");
                safe_input(buf, sizeof(buf));
                return;
            }
            // 切换到临时文件
            strcpy(g_current_vocab_file, tmp_file);
            load_vocab();
            // 初始化模拟时间为当前真实时间
            g_mock_time = time(NULL);
            g_mock_mode = 1;
            printf("模拟词库已创建：%s\n", g_current_vocab_file);
            printf("提示：所有修改将保存在模拟词库中，重置真实时间后自动丢弃。\n");
            // ★★★ 关键：创建沙盒后不要返回，继续执行下面的时间增加 ★★★
        }

        set_color(COLOR_WARN);
        // 现在增加时间（根据用户之前的选择，输入具体数值）
        int delta = 0;
        switch(choice) {
            case 1:
                printf("增加的小时数: ");
                safe_input(buf, sizeof(buf));
                delta = atoi(buf);
                g_mock_time += delta * 3600;
                break;
            case 2:
                printf("增加的分钟数: ");
                safe_input(buf, sizeof(buf));
                delta = atoi(buf);
                g_mock_time += delta * 60;
                break;
            case 3:
                printf("增加的秒数: ");
                safe_input(buf, sizeof(buf));
                delta = atoi(buf);
                g_mock_time += delta;
                break;
            case 4:
                printf("增加的天数: ");
                safe_input(buf, sizeof(buf));
                delta = atoi(buf);
                g_mock_time += delta * 86400;
                break;
            default:
                break;
        }
        set_color(COLOR_DEFAULT);
        time_t new_time = get_current_time();
        printf("模拟时间已更新为: %s\n", ctime(&new_time));
        printf("\n按回车键返回主菜单...");
        safe_input(buf, sizeof(buf));
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

    void cleanup_mock_mode(void) {
        printf("===== 开始强制清理模拟文件 =====\n");

        char log_file[512];
        const char *base = get_basename(g_current_vocab_file);
        char name_no_ext[256];
        strncpy(name_no_ext, base, sizeof(name_no_ext)-1);
        name_no_ext[sizeof(name_no_ext)-1] = '\0';

        char *dot = strstr(name_no_ext, ".dat");
        if (dot) *dot = '\0';

        snprintf(log_file, sizeof(log_file), "daily_review_%s.log", name_no_ext);
        printf("准备删除日志: %s\n", log_file);
        remove(log_file);

        remove(g_current_vocab_file);
        printf("已清理临时文件\n");

        strcpy(g_current_vocab_file, g_original_vocab_file);
        g_mock_mode = 0;
        g_mock_time = 0;
    }
