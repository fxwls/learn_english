// word_manager.c
#include "word_manager.h"
#include "vocab_core.h"
#include "ui_utils.h"
#include "time_utils.h"
#include "file_io.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// is_valid_english
    int is_valid_english(const char *word) {// 检查英文单词是否只包含英文字母的函数
        if (word == NULL || strlen(word) == 0) return 0;
        for (int i = 0; word[i] != '\0'; i++) {
            unsigned char c = (unsigned char)word[i];
            if (!isalpha(c) && c != '-' && c != '\'' && c != ' '){
                return 0;  // 如果包含非字母字符，返回0
            }
        }   
        return 1;  // 所有字符都是字母，返回1
    }
// is_duplicate
    int is_duplicate(const char *english) {// 检查是否有重复的英文单词的函数
        for (int i = 0; i < g_vocab.count; i++) {// 遍历当前存储的单词信息数组
            if (strcasecmp_custom(g_vocab.words[i].english, english) == 0) // 使用自定义的字符串比较函数检查是否有重复的英文单词
                return 1; // 如果找到重复的单词，返回1
        }
        return 0; // 如果没有找到重复的单词，返回0
    }
// show_word_detail
    void show_word_detail(Word *word) {// 显示单词详细信息的函数
        char last_review_str[30], next_review_str[30];// 定义两个字符串变量，用于存储格式化后的上次复习时间和下次复习时间
        format_time(word->last_review, last_review_str, sizeof(last_review_str)); // 将单词的上次复习时间格式化为可读的字符串
        format_time(word->next_review, next_review_str, sizeof(next_review_str)); // 将单词的下次复习时间格式化为可读的字符串
        
        printf("\n===单词详情===\n");
        printf("ID: %d\n", word->id); // 显示单词ID
        printf("英文: %s\n", word->english); // 显示英文单词
        printf("中文: %s\n", word->chinese); // 显示中文
        printf("记忆等级: %d (最高7级)\n", word->level); // 显示单词记忆等级
        printf("记忆稳定性: %.1f秒(%.2f小时)\n", word->stability, word->stability / 3600); // 显示单词记忆稳定性
        printf("上次复习: %s\n", last_review_str); // 显示单词的上次复习时间
        printf("下次复习: %s\n", next_review_str); // 显示单词的下次复习时间
        printf("正确次数：%d | 错误次数：%d\n", word->correct_count, word->wrong_count); // 显示单词的正确记忆次数和错误记忆次数
    }
// add_word
    void add_word() {// 添加新单词到词库中的函数
        if (g_vocab.count >= MAX_WORD) {
            printf("词库已满，无法添加新单词！\n");
            printf("按回车键继续...");
            getchar();
            return; // 如果词库已经满了，提示用户无法添加新单词并返回
        }

        char en[MAX_STR], zh[MAX_STR];
        printf("\n--录入新单词--\n");

        // 输入英文单词
        printf("请输入英文单词： ");
        if (!safe_input(en, MAX_STR) || !is_valid_english(en)) {
            printf("输入无效，英文单词只能包含字母！\n");
            printf("按回车键继续...");
            getchar();
            return; // 如果输入无效，提示用户并返回
        }

        // 检查是否有重复的英文单词
        if (is_duplicate(en)) {
            printf("单词 %s 已存在！\n", en);
            printf("按回车键继续...");
            getchar();
            return; // 如果单词已经存在，提示用户并返回
        }

        // 输入中文释义
        printf("请输入中文释义： ");
        if (!safe_input(zh, MAX_STR) || strlen(zh) == 0) {
            printf("输入无效，取消录入。 \n");
            printf("按回车键继续...");
            getchar();
            return; // 如果输入无效，提示用户并返回
        }

        // 初始化新单词
        Word new_word = {0}; // 定义一个新的Word结构体变量，并将其所有字段初始化为0
        new_word.id = g_vocab.count + 1; // 设置单词ID为当前单词数量加1
        strncpy(new_word.english, en, MAX_STR - 1); // 将输入的英文单词复制到新单词的english字段
        new_word.english[MAX_STR - 1] = '\0'; // 确保字符串以null结尾，防止溢出
        strncpy(new_word.chinese, zh, MAX_STR - 1); // 将输入的中文释义复制到新单词的chinese字段
        new_word.chinese[MAX_STR - 1] = '\0'; // 确保字符串以null结尾，防止溢出
        new_word.level = 0; // 初始化单词记忆等级为0
        new_word.stability = LEVEL_0_INTERVAL; // 初始化记忆稳定性为300秒
        new_word.last_review = 0; // 初始化上次复习时间为0
        new_word.next_review = get_current_time(); //表示新单词需要立即复习
        new_word.correct_count = 0; // 初始化正确记忆次数为0
        new_word.wrong_count = 0; // 初始化错误记忆次数为0

        // 将新单词添加到词库中
        g_vocab.words[g_vocab.count] = new_word; // 将新单词添加到单词信息数组中
        g_vocab.count++; // 增加单词数量

        g_vocab.last_add_date = get_today(); // 更新最近添加单词的日期为今天

        printf("单词 %s 添加成功！\n", en); // 提示用户单词添加成功
        save_vocab(); // 保存当前的单词信息到文件中
        printf("按回车键继续...");
        getchar(); // 等待用户按下回车键继续操作

        
   } 
// browse_all_words
    void browse_all_words() {// 浏览所以单词的函数
        clear_screen();
        printf("\n=====浏览所有单词=====\n");

        if (g_vocab.count == 0) {
            printf("当前词库为空，无法浏览。\n");
            printf("\n按回车键返回主菜单...");
            getchar();
            return;
        }
        
        int page_size =10;
        int total_pages = (g_vocab.count + page_size - 1) / page_size;
        int current_page = 1;
        char choice;

        do {
            clear_screen();
            printf("\n=========== 所有单词列表 (第 %d/%d 页) ===========\n", current_page, total_pages);
            printf("ID  | 英文单词      | 中文释义      | 记忆等级\n");
            printf("-----------------------------------------------\n");

            int start = (current_page - 1) * page_size;
            int end = start + page_size;
            if (end > g_vocab.count) end = g_vocab.count;

            for (int i = start; i < end; i++) {
                Word *w = &g_vocab.words[i];
                printf("%3d  | %-15s | %-15s | 等级 %d\n", 
                    w->id, w->english, w->chinese, w->level);
            }

            printf("\n共 %d 个单词\n", g_vocab.count);
            printf("操作： n - 下一页  p - 上一页  q - 返回主菜单\n");
            char line[16];
            safe_input(line, sizeof(line));
            choice = tolower(line[0]);

            if (choice == 'n' && current_page < total_pages) {
                current_page++;
                } else if (choice == 'p' && current_page > 1) {
                current_page--;
                } else if (choice == 'q') {
                break;
            }
        } while(1);

        printf("\n按回车键返回主菜单...");
        getchar();
    }
// search_word
    void search_word() {// 搜索单词的函数
        clear_screen();
        printf("\n=====搜索单词=====\n");

        if (g_vocab.count == 0) {
            printf("词库为空，无法搜索。\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }
        char keyword[MAX_STR];
        printf("请输入要搜索的关键词（英文单词或中文释义）: ");
        safe_input(keyword, MAX_STR);

        char key_lower[MAX_STR];
        strncpy(key_lower, keyword, MAX_STR - 1);
        key_lower[MAX_STR - 1] = '\0';
        for (int i = 0; key_lower[i]; i++) {
            key_lower[i] = tolower(key_lower[i]);
        }

        int found = 0;
        printf("\n搜索结果:\n");
        printf("ID  | 英文单词      | 中文释义      | 记忆等级\n");
        printf("-----------------------------------------------\n");
        for (int i = 0; i < g_vocab.count; i++) {
            Word *w = &g_vocab.words[i];
             
            char en_lower[MAX_STR];
            strncpy(en_lower, w->english, MAX_STR - 1);
            en_lower[MAX_STR - 1] = '\0';
            for (int j = 0; en_lower[j]; j++) {
                en_lower[j] = tolower(en_lower[j]);
            }

            int match_en = strstr(en_lower, key_lower) != NULL;
            int match_ch = strstr(w->chinese, keyword) != NULL;

            if (match_en || match_ch) {
                printf("%3d  | %-15s | %-15s | 等级 %d\n", 
                    w->id, w->english, w->chinese, w->level);
                found ++;
            }
        }

        if (!found) {
            printf("没有找到与关键词匹配的单词。\n");
        } else {
            printf("\n共找到 %d 个单词\n", found);
        }

        printf("\n按回车键返回主菜单...");
        getchar();
    }
// edit_word
    void edit_word() {// 编辑单词的函数
        clear_screen();
        printf("\n=====编辑单词=====\n");

        if (g_vocab.count == 0) {
            printf("词库为空，无法编辑。\n");
            printf("\n按回车键返回主菜单...");
            getchar();
            return;
        }
        
        char input[MAX_STR];
        printf("请输入要编辑的单词的 ID 或 英文单词: ");
        if (!safe_input(input, MAX_STR)) {
            printf("输入无效，将返回。\n");
            printf("\n按回车键返回主菜单...");
            getchar();
            return;
        }

        Word *target = NULL;

        int id = atoi(input);
        if (id > 0) {
            for (int i = 0; i < g_vocab.count; i++) {
                if (g_vocab.words[i].id == id) {
                    target = &g_vocab.words[i];
                    break;
                }
            }
        }

        if (target == NULL) {
            for (int i = 0; i < g_vocab.count; i++) {
                if (strcasecmp_custom(g_vocab.words[i].english, input) == 0) {
                    target = &g_vocab.words[i];
                    break;
                } 
            }
        }

        if (target == NULL) {
            printf("未找到该单词！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }

        printf("\n当前单词信息:\n");
        printf("ID: %d\n", target->id);
        printf("英文单词: %s\n", target->english);
        printf("中文释义: %s\n", target->chinese);
        printf("记忆等级: %d\n", target->level);

        printf("\n请输入新的英文单词(直接回车不修改): ");
        char new_english[MAX_STR];
        if (safe_input(new_english, MAX_STR) && strlen(new_english) > 0) {
            if (strcasecmp_custom(new_english, target->english) == 0) {
                printf("未修改英文单词！\n");
            } else if (is_duplicate(new_english)) {
                printf("该英文单词已存在！\n");
            } else {
                strcpy(target->english, new_english);
                printf("英文单词已更新！\n");
            }
        } else {
            printf("未修改英文单词！\n");
        }

        printf("请输入新的中文释义(直接回车不修改): ");
        char new_chinese[MAX_STR];
        if (safe_input(new_chinese, MAX_STR) && strlen(new_chinese) > 0) {
            strcpy(target->chinese, new_chinese);
            printf("中文释义已更新！\n");
        } else {
            printf("未修改中文释义！\n");
        }

        save_vocab();
        printf("\n单词编辑完成！数据已保存。\n");
        printf("\n按回车键返回主菜单...");
        getchar();
    }
// delete_word
    void delete_word() {// 删除单词的函数
        clear_screen();
        printf("\n=====删除单词=====\n");

        if (g_vocab.count == 0) {
            printf("词库为空，无法删除。\n");
            printf("\n按回车键返回主菜单...");
            getchar();
            return;
        }

        char input[256];
        printf("请输入要删除的单词的 ID 或 英文单词: ");
        safe_input(input, sizeof(input));

        int index = -1;
        int id = atoi(input);

        if (id > 0) {
            for (int i = 0; i < g_vocab.count; i++) {
                if (g_vocab.words[i].id == id) {
                    index = i;
                    break;
                }
            }
        }

        if (index == -1) {
            for (int i = 0; i < g_vocab.count; i++) {
                if (strcasecmp_custom(g_vocab.words[i].english, input) == 0) {
                    index = i;
                    break;
                }
            }
        }

        if (index == -1) {
            printf("未找到该单词！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }

        printf("\n将要删除：\n");
        printf("ID: %d\n", g_vocab.words[index].id);
        printf("英文单词: %s\n", g_vocab.words[index].english);
        printf("中文释义: %s\n", g_vocab.words[index].chinese);

        printf("\n确认删除吗?(y/n)");
        char line[10];
        safe_input(line, sizeof(line));
        char c = line[0];
        if (c != 'y' && c != 'Y') {
            printf("取消删除。\n");
            printf("\n按回车键返回主菜单...");
            getchar();
            getchar();
            return;
        }

        for (int i = index; i < g_vocab.count - 1; i++) {
            g_vocab.words[i] = g_vocab.words[i + 1];
        }
        g_vocab.count--;

        for (int i = 0; i < g_vocab.count; i++) {
            g_vocab.words[i].id = i + 1;
        }

        save_vocab();
        printf("\n单词删除成功！数据已保存。\n");
        printf("\n按回车键返回主菜单...");
        getchar();
    }