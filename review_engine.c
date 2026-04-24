// review_engine.c
#include "review_engine.h"
#include "vocab_core.h"
#include "ui_utils.h"
#include "time_utils.h"
#include "memory_algorithm.h"
#include "file_io.h"
#include "statistics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// get_need_review_count
    int get_need_review_count() {// 获取需要复习的单词数量的函数
        int count = 0;
        time_t now = get_current_time();// 获取当前时间
        // 遍历当前存储的单词信息数组
        for (int i = 0; i < g_vocab.count; i++) {
            if (g_vocab.words[i].next_review <= now) {// 如果单词的下次复习时间小于或等于当前时间，说明需要复习
            count++;
            } 
        }
        return count;
    }
// quiz_cn_to_en
    int quiz_cn_to_en(Word *word) {// 单词测试（中译英），返回1=正确，0=错误的函数
        char input[MAX_STR];// 定义一个字符串变量，用于存储用户输入的英文单词
        printf("\n【中译英】%s\n", word->chinese); // 提示用户输入单词的英文翻译
        while(1) {//
            if (!safe_input(input,MAX_STR)) {// 
                printf("输入为空，回答错误(按q退出)！\n正确答案是：%s\n", word->english);
                return 0;
            }
            trim(input);// 去除字符串两端的空格
            if (strcasecmp_custom(input,"q") == 0) {
                return -1;
            }
            if (strcasecmp_custom(input,word->english) == 0) {
                printf("回答正确！\n");
                return 1; // 如果用户输入的英文单词与单词的英文字段匹配，返回1表示正确
            } else {
                printf("回答错误！正确答案是：%s\n", word->english); // 如果用户输入的英文单词与单词的英文字段不匹配，提示用户正确答案并返回0表示错误
                return 0;
            }
        }
    }
// quiz_en_to_cn
    int quiz_en_to_cn(Word *word) {// 单词测试（英译中），返回1=正确，0=错误的函数
        char input[MAX_STR];// 定义一个字符串变量，用于存储用户输入的中文释义
        printf("\n【英译中】%s\n", word->english); // 提示用户输入单词的中文释义
        while(1) {
            if (!safe_input(input,MAX_STR)) {// 如果输入无效，提示用户重新输入，直到输入有效为止
                printf("输入为空，回答错误(按q退出)！\n正确答案是：%s\n", word->chinese);
                return 0;
            }
            trim(input);// 去除字符串两端的空格
            if (strcasecmp_custom(input, "q") == 0) {
                return -1;
            }
            if (strcmp(input,word->chinese) == 0) { // 使用 strcmp 直接比较，避免 tolower 破坏 UTF-8 中文编码
                printf("回答正确！\n");
                return 1; // 如果用户输入的中文释义与单词的中文字段匹配，返回1表示正确
            } else {
                printf("回答错误！正确答案是：%s\n", word->chinese); // 如果用户输入的中文释义与单词的中文字段不匹配，提示用户正确答案并返回0表示错误
                return 0;
            }
        }
    }
// quiz_word
    int quiz_word(Word *word) {// 单词测试函数，根据当前的测试模式调用相应的测试函数进行单词测试，返回1表示用户回答正确，返回0表示用户回答错误
        if (current_test_mode == MODE_EN_TO_CN) {
            return quiz_en_to_cn(word); // 如果当前测试模式是英译中，调用quiz_en_to_cn函数进行测试
        } else {
            return quiz_cn_to_en(word); // 如果当前测试模式是中译英，调用quiz_cn_to_en函数进行测试
        }
    }
// review_words
    void review_words() {// 复习待复习单词主函数(使用临时指针数组排序)
        clear_screen();
        // ======每日必须先添加单词才能复习 ======
        int today = get_today();
        if (g_vocab.require_daily_add && g_vocab.last_add_date != today) {
            printf("今日还未添加新单词！\n");
            printf("请先录入至少1个单词，才能开始复习！\n");
            printf("提示：你可以在[设置]中关闭此限制。\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }
        printf("\n=======单词复习=======\n");
        time_t now = get_current_time();// 获取当前时间
        Word *to_review[MAX_WORD];// 定义一个指针数组，用于存储需要复习的单词的指针
        int to_review_count = 0;// 定义一个整数变量，用于记录需要复习的单词数量

        // 遍历所有单词，复习需要复习的单词
        for (int i = 0; i <g_vocab.count; i++) {
            if(g_vocab.words[i].next_review <= now) {
                to_review[to_review_count++] = &g_vocab.words[i];
            }
        }
        if (to_review_count == 0) {
            printf("当前暂无需要复习的单词！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }

        printf("待复习单词数量：%d\n", to_review_count);
        select_test_mode(); // 选择测试模式，允许用户选择中译英测试还是英译中测试
        printf("按回车键开始复习...");
        getchar();

        // 对指针数组按复习紧迫度排序（使用compare_word_ptr函数作为比较函数）
        qsort(to_review, to_review_count, sizeof(Word *), compare_word_ptr_by_forgetting);

        // 复习单词
        int reviewed = 0, correct = 0;
        int today_updated = 0; // 标记当天是否已经更新过连续复习天数，避免重复更新
        for (int i = 0; i < to_review_count; i++) {
            Word *word = to_review[i];
            clear_screen();
            printf("【复习进度】%d/%d(输入q退出复习)\n", reviewed + 1, to_review_count);

            // 进行单词测试，获取用户的测试结果
            int is_correct = quiz_word(word);
            if (is_correct == -1) {
                printf("确认退出复习吗?(y/n)");
                char confirm[10];
                if (safe_input(confirm, sizeof(confirm)) && tolower(confirm[0]) == 'y') {
                    break;
                } else {
                    i--;
                    continue;
                }
            }
            if (is_correct == 1) correct++;
            reviewed++;
            g_vocab.total_review++;// 总复习次数加一
        
            // 更新单词状态
            update_word_level(word, is_correct);

            //在第一个单词复习后立即更新连续复习天数，确保当天的复习进度被记录
            if (!today_updated && reviewed == 1) {
                int current_day = get_today();
                if (g_vocab.last_study_date == 0) {
                    g_vocab.continuous_days = 1;
                } else {
                    time_t last_t = date_to_time_t(g_vocab.last_study_date);
                    time_t now_t = date_to_time_t(current_day);
                    double diff_sec = difftime(now_t, last_t);
                    int diff_days = (int)(diff_sec / (60 * 60 * 24) + 0.5); // 四舍五入计算天数差

                    if (diff_days == 1) {
                        g_vocab.continuous_days++;
                    } else if (diff_days >=2) {
                        g_vocab.continuous_days = 1;
                    }
                }
                g_vocab.last_study_date = current_day;
                today_updated = 1;
                save_vocab();
            }
            if (reviewed % 5 == 0) {
                save_vocab(); // 每复习5个单词保存一次数据，确保复习进度不会丢失
            }

            printf("\n按回车键继续复习（输入q退出复习）...");
            char line[16];
            if (fgets(line, sizeof(line), stdin) == NULL) {
                continue;
            }
            trim_newline(line);

            if (strcasecmp_custom(line, "q") == 0) {
                printf("确认退出复习吗?(y/n)");
                char confirm[10];
                if (safe_input(confirm, sizeof(confirm)) && tolower(confirm[0]) == 'y') {
                    break;
                }
            } else if (strlen(line) > 0) {
                printf("无效的输入，请按回车键继续复习...\n");
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);
            }
        }

        // 更新当天统计并写入日志
        if (reviewed > 0) {
            DailyStat *today_stat = get_today_daily_stat();
            today_stat->total_count += reviewed;
            today_stat->correct_count += correct;
            save_vocab(); // 保存词库状态，确保统计数据也被保存
            write_daily_log();// 将当天的统计数据写入日志文件，便于后续分析和查看复习历史
        }
        printf("\n复习完成！本次复习：%d个单词，正确率%.2f%%\n", reviewed, (reviewed > 0) ? (correct * 100.0 / reviewed) : 0);
        printf("按回车键返回主菜单...");
        getchar();
    }
// review_mistakes
    void review_mistakes() {// 专项复习错词的函数，统计错词数量，选择测试模式，按照单词ID顺序复习错词，并更新单词状态和当天统计数据
        clear_screen();
        printf("\n=======专项复习错词本======\n\n\n");

        // 统计错词数量
        int mistake_count = 0;
        for (int i = 0; i < g_vocab.count; i++) {
            if (g_vocab.words[i].is_mistake == 1) {
                mistake_count++;
            }
        }
        if (mistake_count == 0) {
            printf("当前没有需要复习的错词，恭喜！\n");
            printf("按回车键返回主菜单...\n");
            getchar();
            return;
        }

        printf("待复习错词数量:%d\n", mistake_count);
        select_test_mode(); // 选择测试模式，允许用户选择中译英测试还是英译中测试
        printf("按回车键开始复习...\n");
        getchar();

        // 为了方便，我们按照单词ID的顺序复习错词
        int reviewed = 0, correct = 0;
        int today_updated = 0; // 标记当天是否已经更新过连续复习天数，避免重复更新

        for (int i = 0; i < g_vocab.count; i++) {
            Word *word = &g_vocab.words[i];
            if (word->is_mistake != 1) {
                continue;
            }

            clear_screen();
            printf("【错词复习进度】%d/%d\n", reviewed + 1, mistake_count);

            // 进行测试
            int is_correct = quiz_word(word);
            if (is_correct == -1) {
                printf("确认退出错词复习吗?(y/n)\n");
                char confirm[10];
                if (safe_input(confirm, sizeof(confirm)) && tolower(confirm[0]) == 'y') {
                    break;
                } else {
                    i--;
                    continue;
                }
            }
            if (is_correct == 1) {
                correct++;
            }
            // 正常更新记忆等级
            update_word_level(word, is_correct);
            reviewed++;
            g_vocab.total_review++;// 总复习次数加一
            //在第一个单词复习后立即更新连续复习天数，确保当天的复习进度被记录
            if (!today_updated && reviewed == 1) {
                int current_day = get_today();
                if (g_vocab.last_study_date == 0) {
                    g_vocab.continuous_days = 1;
                } else {
                    time_t last_t = date_to_time_t(g_vocab.last_study_date);
                    time_t now_t = date_to_time_t(current_day);
                    double diff_sec = difftime(now_t, last_t);
                    int diff_days = (int)(diff_sec / (60 * 60 * 24) + 0.5); // 四舍五入计算天数差

                    if (diff_days == 1) {
                        g_vocab.continuous_days++;
                    } else if (diff_days >=2) {
                        g_vocab.continuous_days = 1;
                    }
                }
                g_vocab.last_study_date = current_day;
                today_updated = 1;
                save_vocab();
            }

            if (reviewed % 5 == 0) {
                save_vocab(); // 每复习5个单词保存一次数据，确保复习进度不会丢失
            }

            // 按回车键继续复习
            printf("按回车键继续复习(输入q退出)...\n");
            char line[16];
            if (fgets(line, sizeof(line), stdin) == NULL) {
                continue;// 读取失败，下一个
            }
            trim_newline(line);

            if (strcasecmp_custom(line, "q") == 0) {
                printf("确认退出错词复习吗?(y/n)\n");
                char confirm[10];
                if (safe_input(confirm, sizeof(confirm)) && tolower(confirm[0]) == 'y') {
                    break;
                }
            } else if (strlen(line) > 0){
                printf("无效输入，请按回车继续...\n");
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);
            }
        }

        // 更新当天统计并写入日志
        if (reviewed > 0) {
            DailyStat *today_stat = get_today_daily_stat();
            today_stat->total_count += reviewed;
            today_stat->correct_count += correct;
            save_vocab(); // 保存词库状态，确保统计数据也被保存
            write_daily_log();// 将当天的统计数据写入日志文件，便于后续分析和查看复习历史
        }
        printf("\n错词复习完成！本次复习：%d个单词，正确率：%.2f%%\n", reviewed,(reviewed > 0) ? (correct * 100.0 / reviewed) : 0.0);
        printf("按回车键返回主菜单...\n");
        getchar();
    }
// select_test_mode
    void select_test_mode() {// 选择测试模式的函数，允许用户选择中译英测试还是英译中测试
        int choice;
        printf("\n请选择测试模式：\n");
        printf("1. 中译英测试\n");
        printf("2. 英译中测试\n");
        printf("请输入你的选择(1/2): ");

        char line[16];
        if (!safe_input(line, sizeof(line))) {
        choice = 0;
        } else {
            choice = atoi(line);
        }

        if (choice == 2) {
            current_test_mode = MODE_EN_TO_CN; // 设置测试模式为英译中
        } else if (choice == 1) {
            current_test_mode = MODE_CN_TO_EN; // 默认设置测试模式为中译英
        } else {
            printf("无效的选择，默认使用中译英测试模式！\n");
             current_test_mode = MODE_CN_TO_EN; // 如果用户输入无效，默认设置测试模式为中译英
        }
    }
// show_review_rank
    void show_review_rank() {// 显示待复习单词排行榜的函数，按照单词的下次复习时间排序，显示需要复习的单词列表，并提供分页浏览功能
        clear_screen();
        printf("\n===========待复习单词排行榜===========\n");

        time_t now = get_current_time();
        //第一步：统计待复习数量
        int need_count = 0;
        Word *need_words[MAX_WORD];// 指针数组
        for (int i = 0; i < g_vocab.count; i++) {
            if (g_vocab.words[i].next_review <= now) {
                need_words[need_count] =&g_vocab.words[i];
                need_count++;
            }
        }

        if (need_count == 0) {
            printf("当前没有需要复习的单词，恭喜！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }

        //第二步：按紧迫度排序（升序，即 next_review 小的在前）(遗忘概率高的在前)
        qsort(need_words, need_count, sizeof(Word*), compare_word_ptr_by_forgetting);

        //分页显示
        int page_size = 10;
        int total_pages = (need_count + page_size - 1) / page_size;
        int current_page = 1;
        char choice;

        do {
            clear_screen();
            printf("\n===========待复习单词排行榜 (第 %d/%d 页)===========\n",current_page, total_pages);
            printf("序号 | 英文单词 | 中文释义 | 剩余时间 | 记忆等级\n");
            printf("------------------------------------------------\n");

            int start = (current_page - 1) * page_size;
            int end = start + page_size;
            if (end > need_count) end = need_count;

            for (int i = start; i < end; i++) {
                Word *w = need_words[i];
                // 计算剩余时间
                double secs = difftime(w->next_review, now);
                char time_str[50];
                if (secs <= 0) {
                    strcpy(time_str, "立即复习");
                } else if (secs < 60) {
                    sprintf(time_str, "%.0f秒", secs);
                } else if (secs < 3600) {
                    sprintf(time_str, "%.0f分钟", secs / 60);
                } else if (secs < 86400){
                    sprintf(time_str, "%0.1f小时", secs / 3600);
                } else {
                    sprintf(time_str, "%.1f天", secs / 86400);
                }

                int total = w->correct_count + w->wrong_count;// 计算总的测试次数
                double correct_rate = (total == 0) ? 0.5 : (double)w->correct_count / total;// 计算正确率   
                if (total == 0) {
                    printf("%-4d | %-10s | %-10s | %-10s | 等级：%d | 正确率:未知\n", 
                        i+1,
                        w->english, 
                        w->chinese, 
                        time_str,
                        w->level);
                } else {
                    printf("%-4d | %-10s | %-10s | %-10s | 等级：%d | 正确率:%.0f%%\n", 
                        i+1,
                        w->english, 
                        w->chinese, 
                        time_str,
                        w->level, 
                        correct_rate * 100);
                }
            }
                printf("------------------------------------------------\n");
                printf("共 %d 个待复习单词\n", need_count);
                printf("操作：n-下一页 p-上一页 q-返回主菜单：");
                char line[16];
                safe_input(line, sizeof(line));
                choice = tolower(line[0]);

                if (choice == 'n' ) {
                    if (current_page < total_pages) current_page++;
                    else{
                        printf("已经是最后一页了！按回车继续...\n");
                        getchar();// 等待用户按回车
                    }
                } else if (choice == 'p' ){
                    if (current_page > 1) current_page--;
                    else {
                        printf("已经是第一页了！按回车继续...\n");
                        getchar();// 等待用户按回车
                    }
                } else if (choice == 'q' ) {
                    break;
                } else {
                    printf("无效输入，按回车继续...");
                    getchar();// 等待回车  
                }
            } while (choice != 'q');
    }

    