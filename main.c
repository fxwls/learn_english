// main.c
#include "vocab_core.h"
#include "ui_utils.h"
#include "time_utils.h"
#include "file_io.h"
#include "review_engine.h"
#include "word_manager.h"
#include "statistics.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <locale.h>


int main() {
    #ifdef _WIN32
    system("chcp 65001 > nul");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "zh_CN.UTF-8");
    #endif
    load_vocab();
    printf("加载完成，按回车继续...");
    getchar();

    int choice;
    do {
        clear_screen();
        int today = get_today();
        int today_add = 0;
        for (int i=0; i<g_vocab.count; i++) {
            if (time_to_date(g_vocab.words[i].last_review) == today) today_add++;
        }
        set_color(COLOR_TITLE);
        printf("===== 艾宾浩斯记忆系统 =====\n");
        if (g_mock_mode) {
            set_color(COLOR_WARN);
            printf("当前处于模拟时间模式！\n");
            set_color(COLOR_DEFAULT);
        }
        printf("\n");
        set_color(COLOR_DEFAULT);
        printf("词库：%s  总数：%d\n", g_current_vocab_file, g_vocab.count);
        printf("待复习：%d   今日新增：%d\n", get_need_review_count(), today_add);
        printf("[1]添加 [2]复习 [3]浏览 [4]搜索\n");
        printf("[5]排行 [6]错词 [7]统计 [8]词库\n");
        printf("[9]编辑 [10]删除 [11]设置 [12]退出\n");
        printf("选择：");

        char line[16];
        if (!safe_input(line, sizeof(line))) {
            choice = 0;
        } else {
            choice = atoi(line);
        }

        switch(choice) {
            case 1: add_word(); break;
            case 2: review_words(); break;
            case 3: browse_all_words(); break;
            case 4: search_word(); break;
            case 5: show_review_rank(); break;
            case 6: review_mistakes(); break;
            case 7: show_statistics(); break;
            case 8:
                clear_screen();
                printf("[1]新建 [2]切换 [3]恢复\n选择：");
                safe_input(line, sizeof(line));
                int s = atoi(line);
                if(s==1) create_new_vocab();
                else if(s==2) switch_vocab();
                else if(s==3) restore_vocab();
                break;
            case 9: edit_word(); break;
            case 10: delete_word(); break;
            case 11:
                clear_screen();
                set_color(COLOR_TITLE);
                printf("==================== 设置 ====================\n");
                set_color(COLOR_DEFAULT);
                printf("  [1] 重置学习参数\n");
                printf("  [2] 模拟时间\n");
                printf("  [3] 每日学习目标\n");
                printf("  [4] 每日添加单词限制（当前：%s）\n", g_vocab.require_daily_add ? "启用" : "禁用");
                printf("  [0] 返回主菜单\n");
                set_color(COLOR_WARN);
                printf("请选择：");
                set_color(COLOR_DEFAULT);
    
                char line[16];
                safe_input(line, sizeof(line));
                int t = atoi(line);
    
                switch(t) {
                    case 1: reset_learning_params(); break;
                    case 2: set_mock_time(); break;
                    case 3: set_daily_goal(); break;
                    case 4: toggle_require_daily_add(); break;
                    default: break;
                }
                break;
            case 12:
                save_vocab();
                if (!g_mock_mode) backup_vocab();
                cleanup_mock_mode();
                printf("已保存并清理模拟词库，请按回车键退出\n"); break;
            default: printf("无效\n"); system("pause");
        }
    } while(choice != 12);
    return 0;
}