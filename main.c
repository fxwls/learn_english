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
    // === 强制 UTF-8 代码块开始 ===
    system("chcp 65001 > nul");   // 切换控制台代码页为 UTF-8
    setlocale(LC_ALL, ".UTF-8");  // 设置区域为 UTF-8
    // === 强制 UTF-8 代码块结束 ===
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
        set_color(COLOR_DEFAULT);
        printf("词库：%s  总数：%d\n", g_current_vocab_file, g_vocab.count);
        printf("待复习：%d   今日新增：%d\n", get_need_review_count(), today_add);
        printf("[1]添加 [2]复习 [3]浏览 [4]搜索\n");
        printf("[5]排行 [6]错词 [7]统计 [8]词库\n");
        printf("[9]编辑 [10]删除 [11]设置 [12]退出\n");
        printf("选择：");

        if (scanf("%d", &choice) != 1) { choice=0; while(getchar()!='\n'); }
        getchar();

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
                int s; scanf("%d",&s); getchar();
                if(s==1) create_new_vocab();
                else if(s==2) switch_vocab();
                else if(s==3) restore_vocab();
                break;
            case 9: edit_word(); break;
            case 10: delete_word(); break;
            case 11:
                clear_screen();
                printf("[1]重置 [2]时间 [3]目标\n选择：");
                int t; scanf("%d",&t); getchar();
                if(t==1) reset_learning_params();
                else if(t==2) set_mock_time();
                else if(t==3) set_daily_goal();
                break;
            case 12:
                save_vocab(); backup_vocab();
                printf("已保存\n"); break;
            default: printf("无效\n"); system("pause");
        }
    } while(choice != 12);
    return 0;
}