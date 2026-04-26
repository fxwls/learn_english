// statistics.c
#include "statistics.h"
#include "vocab_core.h"
#include "ui_utils.h"
#include "time_utils.h"
#include "file_io.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>

// get_today_daily_stat
    DailyStat* get_today_daily_stat() {// 获取今天的日志
        int today = get_today();
        // 找到今天的日志
        for (int i = 0; i < g_vocab.daily_stats_count; i++) {
            if (g_vocab.daily_stats[i].date == today) {
                return &g_vocab.daily_stats[i];
            }
        }
        // 如果没有找到，创建新的日志
        if (g_vocab.daily_stats_count >= 30) {// 如果日志数量超过30，删除最早的日志
            int min_idx = 0;
            for (int i = 1; i < g_vocab.daily_stats_count; i++) {
                if (g_vocab.daily_stats[i].date < g_vocab.daily_stats[min_idx].date) {
                    min_idx = i;
                }
            }
            for (int i = min_idx; i < g_vocab.daily_stats_count - 1; i++) {
                g_vocab.daily_stats[i] = g_vocab.daily_stats[i + 1];
            }
            g_vocab.daily_stats_count--;
        }
        //找到插入位置，保持数组按日期升序
        int pos = 0;
        while (pos <g_vocab.daily_stats_count && g_vocab.daily_stats[pos].date < today) {
            pos++;
        }
        //把pos及之后的元素后移一位，腾出位置插入新的日志
        for (int i = g_vocab.daily_stats_count; i > pos; i--) {
            g_vocab.daily_stats[i] = g_vocab.daily_stats[i - 1];
        }

        //插入新的日志
        g_vocab.daily_stats[pos].date = today;
        g_vocab.daily_stats[pos].total_count = 0;
        g_vocab.daily_stats[pos].correct_count = 0;
        g_vocab.daily_stats_count++;
        return &g_vocab.daily_stats[pos];
    }
// write_daily_log
    void write_daily_log() {
        // 生成独立日志文件名：daily_review_词库名.log
        char log_filename[512];
        const char *vocab_base = get_basename(g_current_vocab_file);
        char base_no_ext[256];
        strncpy(base_no_ext, vocab_base, sizeof(base_no_ext) - 1);
        base_no_ext[sizeof(base_no_ext) - 1] = '\0';
        char *dot = strstr(base_no_ext, ".dat");
        if (dot) *dot = '\0';
        snprintf(log_filename, sizeof(log_filename), "daily_review_%s.log", base_no_ext);

        FILE *log = fopen(log_filename, "w");
        if (!log) {
            printf("日志文件创建失败！(%s)\n", log_filename);
            return;
        }

        for (int i = 0; i < g_vocab.daily_stats_count; i++) {
            DailyStat *stat = &g_vocab.daily_stats[i];
            double rate = (stat->total_count == 0) ? 0.0 : (double)stat->correct_count / stat->total_count * 100;
            fprintf(log, "%04d-%02d-%02d %d %d  %.2f%%\n",
                    stat->date / 10000, (stat->date / 100) % 100, stat->date % 100,
                    stat->correct_count, stat->total_count, rate);
        }
        fclose(log);
    }
// show_statistics
    void show_statistics() {// 显示复习统计
        clear_screen();
        set_color(COLOR_TITLE);
        printf("===================================================\n");
        printf("                     学习统计\n");
        printf("===================================================\n");
        set_color(COLOR_DEFAULT);
        printf("连续学习天数：%d\n", g_vocab.continuous_days);
        printf("每日目标：%d\n", g_vocab.daily_goal);
        // 先展示学习率
        printf("累计复习总次数：%d\n", g_vocab.total_review);
        double current_lr = 0.05 / (1.0 + g_vocab.total_review / 500.0);
        if (current_lr < 0.002) current_lr = 0.002;
        printf("当前学习率：%.4f\n", current_lr);

        time_t now = get_current_time();
        int dates[7];
        int counts[7] = {0};
        for (int i = 0; i < 7; i++) {
            time_t day = now - (6 - i) * 86400; // 计算7天前的时间戳
            dates[i] = time_to_date(day); // 将时间戳转换为日期格式，并存储在dates数组中
        }
        for (int i = 0; i < g_vocab.daily_stats_count; i++) {
            DailyStat *stat = &g_vocab.daily_stats[i];
            for (int j = 0; j < 7; j++) {
                if (stat->date == dates[j]) {
                    counts[j] = stat->total_count; // 如果日志中的日期与dates数组中的日期匹配，将对应的复习数量存储在counts数组中
                    break;
                }
            }
        }

        int max_count = 1;
        for (int i = 0; i < 7; i++) {
            if (counts[i] > max_count) {
                max_count = counts[i]; // 找到counts数组中的最大值，用于后续的图表显示
            }
        }
        int scale = (max_count + 19) / 20; // 计算图表的缩放比例，每个#代表20个单词
        if (scale < 1) scale = 1; // 确保缩放比例至少为1，避免除以0的情况
        printf("【每日复习单词数】（最近7天）\n");
        printf("（每个 # 代表 %d 个单词）\n", scale);
        for (int i = 0; i < 7; i++) {
            time_t day = now - (6 - i) * 86400; // 计算每一天的时间戳
            struct tm *tm_info = localtime(&day); // 将时间戳转换为本地时间的结构体tm_info
            char date_str[20];
            strftime(date_str, sizeof(date_str), "%m-%d", tm_info); // 将时间格式化为字符串，格式为月-日

            int bar_count = counts[i] / scale; // 计算图表中#的数量，根据复习数量和缩放比例进行计算
            if (bar_count < 1 && counts[i] > 0) bar_count = 1; // 如果复习数量大于0但计算出的#数量为0，至少显示一个#，表示有复习
            printf("%5s |", date_str); // 输出日期标签
            for (int j = 0; j < bar_count; j++) {
                putchar('#'); // 输出#字符，表示复习数量
            }
            printf("(%d)\n", counts[i]);
        }
        printf("\n【每日复习正确率趋势】（最近%d天）\n", g_vocab.daily_stats_count);
        if (g_vocab.daily_stats_count == 0) {
            printf("暂无复习记录。\n");
        } else {
            int start = (g_vocab.daily_stats_count > 10) ? g_vocab.daily_stats_count - 10 : 0; // 只显示最近10天的正确率趋势
            int n = g_vocab.daily_stats_count - start;
            
            printf("正确率\n");
            for (int level = 100; level >= 0; level -= 10) {
                printf("%3d%% |", level);
                for (int i = 0; i < n; i++) {
                    DailyStat *stat = &g_vocab.daily_stats[start + i];
                    double rate = (stat->total_count == 0) ? 0.0 : (double)stat->correct_count / stat->total_count * 100.0;
                   if (rate >= (double)level) {
                        printf("  #  ");   // 5字符宽，与日期对齐
                    } else {
                        printf("     ");
                    }
                }
                printf("\n");
            }
            printf("     +");
            for (int i = 0; i < n; i++) printf("-----");  // 每列5字符
            printf("\n      ");
            for (int i = 0; i < n; i++) {
                int d = g_vocab.daily_stats[start + i].date;
                printf("%02d/%02d", (d / 100) % 100, d % 100);  // 正好5字符，对齐
            }
            printf("\n\n");

            // 文字汇总：每天的具体数字
            printf("%-10s %6s %6s %8s\n", "日期", "复习数", "正确数", "正确率");
            printf("----------------------------------\n");
            for (int i = 0; i < n; i++) {
                DailyStat *ds = &g_vocab.daily_stats[start + i];
                double rate = (ds->total_count == 0)? 0.0 : (double)ds->correct_count / ds->total_count * 100.0;
                printf("%04d-%02d-%02d %-8d %-8d %.1f%%\n",ds->date / 10000,(ds->date / 100) % 100,ds->date % 100,ds->total_count,ds->correct_count,rate);
            }
        }

        printf("\n【系统自适应参数】\n");
        printf("答对增长系数(gain): %.4f\n", g_vocab.gain);
        int gain_bar = (int)(g_vocab.gain / 0.50f * 20);
        for (int i = 0; i < gain_bar; i++) {
            putchar('#');
        }
        printf("\n");

        printf("答错减少系数(loss): %.4f\n", g_vocab.loss);
        int loss_bar = (int)(g_vocab.loss / 0.50f * 20);
        for (int i = 0; i < loss_bar; i++) {
            putchar('#');
        }
        printf("\n");
        printf("(参数范围：gain[0.02~0.50], loss[0.05~0.80]), 使用越多越精准\n");
        printf("\n按回车键返回主菜单...");
        getchar();
    }
// reset_learning_params
    void reset_learning_params() {// 重置学习参数
        printf("\n=====重置学习参数=====\n");
        printf("当前 gain: %.4f, loss: %.4f\n", g_vocab.gain, g_vocab.loss);
        printf("确认重置为默认值(gain=0.10, loss=0.30)吗?(y/n)");
        char confirm[10];
        safe_input(confirm, sizeof(confirm));
        if (tolower(confirm[0]) == 'y') {
            g_vocab.gain = 0.10f;
            g_vocab.loss = 0.30f;
            save_vocab();
            printf("学习参数已重置为默认值。\n");
        } else {
            printf("取消重置学习参数。\n");
        }
        printf("\n按回车键返回主菜单...");
        getchar();
    }
// set_daily_goal
    void set_daily_goal() {// 设置每日学习目标的函数
        printf("\n=====设置每日学习目标=====\n");
        printf("当前每日学习目标: %d\n", g_vocab.daily_goal);
        printf("请输入你的每日学习目标: ");
        char line[16];
        if (!safe_input(line, sizeof(line))) {
            printf("无效的目标，将返回。\n");
            return;
        }
        int goal = atoi(line);
        if (goal < 1) goal = 10;
        g_vocab.daily_goal = goal;
        save_vocab();
        printf("每日学习目标已设置为: %d\n", g_vocab.daily_goal);
        printf("\n按回车键返回主菜单...");
        getchar();
    }

    void toggle_require_daily_add(void) {
    printf("\n=====每日添加单词限制=====\n");
    printf("当前状态：%s\n", g_vocab.require_daily_add ? "启用（复习前必须添加新词）" : "禁用");
    printf("是否切换？(y/n): ");
    char confirm[10];
    safe_input(confirm, sizeof(confirm));
    if (tolower(confirm[0]) == 'y') {
        g_vocab.require_daily_add = !g_vocab.require_daily_add;
        save_vocab();
        printf("已切换为：%s\n", g_vocab.require_daily_add ? "启用" : "禁用");
    } else {
        printf("取消操作。\n");
    }
    printf("\n按回车键返回主菜单...");
    getchar();
}

void manual_backup(void) {
    printf("\n=====手动备份当前词库=====\n");
    printf("正在备份当前词库：%s\n", g_current_vocab_file);
    backup_vocab();
    printf("手动备份完成！\n");
    printf("按回车键返回主菜单...");
    getchar();
}