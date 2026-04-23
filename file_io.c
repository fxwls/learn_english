#include "file_io.h"
#include "vocab_core.h"
#include "ui_utils.h"
#include "time_utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

int get_need_review_count(void); // 临时声明

// encrypt_data
    void encrypt_data(void *data, int len) {// 加密数据的函数，使用简单的异或加密方法对数据进行加密
        unsigned char *p = (unsigned char *)data;
        for (int i = 0; i < len; i++) {// 对数据中的每个字节进行异或操作
            p[i] ^= ENCRYPT_KEY;
        }
    }
// decrypt_data
    void decrypt_data(void *data, int len) {// 解密数据的函数，使用与加密相同的方法对数据进行解密
        // 加密和解密是对称的，直接调用加密函数即可
        encrypt_data(data, len);
    }
// calculate_checksum
    int calculate_checksum(Vocab *vocab) {// 计算词库数据的校验和的函数，用于验证数据的完整性，返回一个整数表示校验和
        int sum = vocab->count + vocab->last_add_date; // 将单词数量和最后添加日期的整数值进行加法运算
        for (int i = 0; i < vocab->count; i++) {
            sum += vocab->words[i].id;
            sum += vocab->words[i].level;
            sum += (int)vocab->words[i].last_review;
            sum += (int)vocab->words[i].next_review;
            sum += vocab->words[i].correct_count;
            sum += vocab->words[i].wrong_count;
            for (const char *p = vocab->words[i].english; *p; p++) {
                sum += (unsigned char)(*p);
            }
            for (const char *p = vocab->words[i].chinese; *p; p++) {
                sum += (unsigned char)(*p);
            }
        }
        for (int i = 0; i < vocab->daily_stats_count && i < 30; i++) {
            sum += vocab->daily_stats[i].date;
            sum += vocab->daily_stats[i].correct_count;
            sum += vocab->daily_stats[i].total_count;
        }
        sum += (int)(vocab->gain * 1000);
        sum += (int)(vocab->loss * 1000);
        return sum;
    }
// save_vocab
    void save_vocab() {// 保存单词信息到文件的函数
        if (g_vocab.count < 0 || g_vocab.count > MAX_WORD) {// 检查单词数量是否在合理范围内，防止异常数据导致文件写入错误
            printf("错误:单词数量异常，无法保存！\n");
            return; 
        }

        g_vocab.checksum = calculate_checksum(&g_vocab); // 计算校验和，确保文件完整性
        FILE *fp = fopen(g_current_vocab_file, "wb");// 以二进制写入模式打开文件，如果文件不存在会创建新文件
        if (!fp) {
            printf("错误:无法打开文件保存！\n");
            return;
        }

        // 对词库信息进行加密   
        static Vocab tmp ;// 定义一个静态变量tmp，保存当前的词库信息，使用静态变量可以避免频繁创建和销毁变量，提高性能
        tmp = g_vocab; 
        encrypt_data(&tmp, sizeof(Vocab)); // 对词库信息进行加密
        fwrite(&tmp, sizeof(Vocab), 1, fp); // 将加密后的词库信息写入文件

        fclose(fp); // 关闭文件
        printf("数据保存成功！\n");
    }
// load_vocab
    void load_vocab() {// 从文件中加载单词信息的函数
        memset(&g_vocab, 0, sizeof(Vocab));// 初始化全局变量g_vocab的内存空间，确保所有字段都被初始化为0，避免内存垃圾值导致崩溃
        g_vocab.count = 0;// 初始化单词数量为0，表示当前没有存储任何单词

        FILE *fp = fopen(g_current_vocab_file, "rb");// 以二进制读取模式打开文件，如果文件不存在会返回NULL
        if (!fp) {
            printf("首次运行，未找到词库文件，已初始化空词库！\n");
            printf("提示：请先通过[录入新单词]功能添加至少一个单词，才能开始复习！\n");
            return;
        }

        // 读取文件中的词库信息
        static Vocab tmp;// 创建一个临时变量，保存从文件中读取的词库信息
        memset(&tmp, 0, sizeof(Vocab)); // 初始化临时变量的内存空间，确保所有字段都被初始化为0，避免内存垃圾值导致崩溃
        if (fread(&tmp, sizeof(Vocab), 1, fp) != 1) {
            printf("\n======================================\n");
            printf("错误：词库文件损坏（读取失败）！\n");
            printf("可能原因：文件被手动修改/磁盘损坏\n");
            printf("解决方案：\n");
            printf("1. 删除 vocab.dat 重新创建词库\n");
            printf("2. 若有备份，后续可通过恢复功能找回\n");
            printf("======================================\n");
            fclose(fp);
            return;
        }
        fclose(fp);

        // 对词库信息进行解密
        decrypt_data(&tmp, sizeof(Vocab)); // 对词库信息进行解密

        // 验证校验和，确保文件完整性
        if (tmp.checksum != calculate_checksum(&tmp)) {
            printf("\n======================================\n");
            printf("错误：词库文件损坏（校验失败）！\n");
            printf("可能原因：文件被手动修改/磁盘损坏\n");
            printf("解决方案：\n");
            printf("1. 删除 vocab.dat 重新创建词库\n");
            printf("2. 若有备份，后续可通过恢复功能找回\n");
            printf("======================================\n");
            return;
        }

        // 比较主文件与备份文件,防止数据被篡改
        FILE *fb = fopen(g_current_backup_vocab_file, "rb");
        if (fb != NULL) {// 如果备份文件存在，则比较主文件和备份文件是否相同
            fclose(fb);
            if (!is_file_same()) {
                printf("\n======================================\n");
                printf("警告：检测到词库文件异常！可能已被篡改或损坏！\n");
                printf("建议：删除 %s 文件后重新运行程序，或从备份文件恢复数据！\n", g_current_vocab_file);
                printf("======================================\n");
            }
        }

        g_vocab = tmp; // 将解密后的词库信息赋值给全局变量g_vocab
        // 兼容旧版词库
        if (g_vocab.daily_stats_count < 0 || g_vocab.daily_stats_count > 30) {
            g_vocab.daily_stats_count = 0;
        }
        if (g_vocab.gain < 0.01f || g_vocab.gain > 1.0f) g_vocab.gain = 0.10f;
        if (g_vocab.loss < 0.01f || g_vocab.loss > 0.99f) g_vocab.loss = 0.30f;
        if (g_vocab.total_review < 0) g_vocab.total_review = 0;

        printf("已加载词库文件：%s\n", g_current_vocab_file);// 提示用户加载的词库文件名
        printf("已加载 %d个单词!\n", g_vocab.count); // 打印加载的单词数量
        printf("待复习单词数: %d\n", get_need_review_count()); // 打印需要复习的单词数量
    }
// backup_vocab
    void backup_vocab() {// 备份词库的函数，将当前的词库数据加密后保存到备份文件中，以防止数据泄露或被未授权访问
        // 备份文件也加密保存，防止数据泄露或被未授权访问
        static Vocab tmp ;// 定义一个静态变量tmp，用于保存当前的词库信息，静态变量在函数调用结束后仍然存在，避免了频繁的内存分配和释放
        tmp = g_vocab; 
        encrypt_data(&tmp, sizeof(Vocab)); // 对词库信息进行加密

        FILE *fp = fopen(g_current_backup_vocab_file, "wb");// 以二进制写入模式打开备份文件，如果文件不存在会创建新文件
        if (!fp) {
            printf("备份文件创建失败！\n");
            printf("建议：检查程序目录权限，或手动创建空的 %s 文件后重试！\n", g_current_backup_vocab_file);
            return;
        }
        fwrite(&tmp, sizeof(Vocab), 1, fp); // 将整个词库数据结构写入备份文件中
        fclose(fp); // 关闭文件

        printf("数据备份成功！生成备份文件：%s\n", g_current_backup_vocab_file);// 提示用户数据备份成功，并显示备份文件的名称
    }
// restore_vocab
    void restore_vocab() {// 恢复词库
        clear_screen();
        printf("=======恢复词库======\n");

        char backup_file[50][300]; // 定义一个二维数组，用于存储当前目录下的备份文件名，最多支持50个文件，每个文件名最长255字符
        int backup_count = 0; // 定义一个整数变量，用于记录当前目录下的备份文件数量

        WIN32_FIND_DATA find_data; // 定义一个WIN32_FIND_DATA结构体变量，用于存储查找文件的信息
        HANDLE hFind = FindFirstFile("*_backup.dat", &find_data); // 查找当前目录下的所有备份文件，并将第一个找到的文件的信息存储在find_data中，返回一个句柄hFind用于后续操作
        if (hFind != INVALID_HANDLE_VALUE) { 
           do {
                if (backup_count >= 50) break; // 如果文件数量超过50，停止查找
                strncpy(backup_file[backup_count], find_data.cFileName, 299); // 将找到的备份文件名复制到backup_file数组中
                backup_file[backup_count][299] = '\0'; // 确保字符串以null结尾，防止溢出
                backup_count++;
            } while (FindNextFile(hFind, &find_data));
            FindClose(hFind); // 关闭查找句柄
        }
        
        if (backup_count == 0) {
            printf("当前目录下未找到任何备份文件！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }

        printf("可恢复的备份文件:\n");
        printf("==============================\n");
        for (int i = 0; i < backup_count; i++) {
            printf("%d. %s\n", i + 1, backup_file[i]);
        }
        printf("==============================\n");
        printf("请输入序号选择要恢复的备份文件(0表示返回主菜单):");

        int choice;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("输入无效！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return; // 如果用户输入无效，提示用户并返回主菜单
        }
        getchar(); // 清除输入缓冲区中的换行符

        if (choice == 0) {
            return; // 用户选择返回主菜单
        }
        if (choice < 1 || choice > backup_count) {
            printf("选择无效！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return; // 如果用户输入的序号无效，提示用户并返回主菜单
        }
        // 推算出用户选择的备份文件对应的原词库文件名(去掉"_backup.dat"后缀)
        char selected_backup[300];
        strncpy(selected_backup, backup_file[choice - 1], 299); // 获取用户选择的备份文件名
        selected_backup[299] = '\0'; // 确保字符串以null结尾，防止溢出
        
        char target_vocab[256];
        strncpy(target_vocab, backup_file[choice - 1], 255);
        target_vocab[255] = '\0';
        char *p = strstr(target_vocab, "_backup.dat");
        if (p) {
            strcpy(p, ".dat"); // 将"_backup.dat"替换为".dat"，得到原词库文件名
        } else {
            printf("备份文件名格式不正确！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return; // 如果备份文件名格式不正确，提示用户并返回主菜单
        }

        // 二次确认是否恢复
        printf("确定要恢复备份文件 %s 吗？\n这将覆盖当前词库 %s 的数据！\n确认恢复吗?(y/n): ", selected_backup, target_vocab);
        char confirm[10];
        safe_input(confirm, sizeof(confirm));
        if (tolower(confirm[0]) != 'y') {
            printf("已取消恢复操作！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return; // 如果用户选择不确认，提示用户并返回主菜单
        }

        static Vocab tmp; // 定义一个静态变量tmp，用于保存从备份文件中读取的词库信息，静态变量在函数调用结束后仍然存在，避免了频繁的内存分配和释放
        memset(&tmp, 0, sizeof(Vocab)); // 将tmp的内存空间清零，初始化词库信息

        FILE *fp = fopen(selected_backup, "rb"); // 以二进制读取模式打开用户选择的备份文件
        if (!fp) {
            printf("备份文件打开失败！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return; // 如果备份文件无法打开，提示用户并返回主菜单
        }
        if (fread(&tmp, sizeof(Vocab), 1, fp) != 1) { // 从备份文件中读取词库信息到tmp变量中，如果读取失败，提示用户并返回主菜单
            printf("备份文件读取失败！\n");
            fclose(fp);
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }
        fclose(fp);

        // 解密数据
        decrypt_data(&tmp, sizeof(Vocab)); // 对从备份文件中读取的词库信息进行解密
        if (calculate_checksum(&tmp) != tmp.checksum) { // 验证数据的完整性，如果校验和不匹配，提示用户并返回主菜单
            printf("备份文件数据损坏或被篡改！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }

        // 恢复数据
        g_vocab = tmp; // 将解密后的词库信息赋值给全局变量g_vocab，完成数据恢复
        strncpy(g_current_vocab_file, target_vocab, 255); // 更新当前词库文件名为恢复的词库文件名
        g_current_vocab_file[255] = '\0'; // 确保字符串以null结尾，防止溢出
        generate_backup_filename(g_current_vocab_file, g_current_backup_vocab_file); // 根据新的当前词库文件名生成对应的备份文件名
        g_current_backup_vocab_file[299] = '\0'; // 确保字符串以null结尾，防止溢出

        save_vocab(); // 保存恢复后的词库信息到当前词库文件中
        backup_vocab(); // 创建新的备份文件，保存当前词库的状态

        printf("数据恢复成功！\n");
        printf("已从 [%s] 恢复 %d 个单词到 [%s]\n",
           selected_backup, g_vocab.count, g_current_vocab_file);
        printf("按回车键返回主菜单...");
        getchar();
    }
// create_new_vocab
    void create_new_vocab() {// 新建词库
        clear_screen();
        printf("=======新建词库======\n");

        char file_name[256];
        printf("请输入词库文件名(不包括扩展名):\n");
        safe_input(file_name, sizeof(file_name) - 4); // 获取用户输入的词库文件名，并存储在file_name变量中

        if (strlen(file_name) == 0) {
            printf("文件名不能为空！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return; // 如果用户输入的文件名为空，提示用户并返回主菜单
        }
        // 添加.dat扩展名
        char new_file[256];
        snprintf(new_file, sizeof(new_file), "%s.dat", file_name); // 构造新的词库文件名，添加.dat扩展名
        // 生成备份文件名
        char backup_file[300];
        generate_backup_filename(new_file, backup_file); // 根据新的词库文件名生成对应的备份文件名

        // 切换当前词库
        strcpy(g_current_vocab_file, new_file); // 更新当前词库文件名为新创建的文件
        strcpy(g_current_backup_vocab_file, backup_file); // 更新备份文件名为新生成的备份文件

        // 初始化词库信息
        memset(&g_vocab, 0, sizeof(Vocab)); // 将全局变量g_vocab的内存空间清零，初始化词库信息
        g_vocab.count = 0; // 初始化单词数量为0
        g_vocab.gain = 0.10f;
        g_vocab.loss = 0.30f;
        g_vocab.total_review = 0;
        g_vocab.daily_goal = 10;
        g_vocab.continuous_days = 0;
        g_vocab.last_study_date = 0;
        g_vocab.daily_stats_count = 0;

        printf("新词库 %s 创建成功！\n", g_current_vocab_file); // 提示用户新词库创建成功，并显示新词库的文件名
        save_vocab(); // 保存空文件
        backup_vocab(); // 创建备份文件
        printf("按回车键返回主菜单...");
        getchar();
    }
// switch_vocab
    void switch_vocab() {// 切换词库
        clear_screen();
        printf("=======切换词库======\n");
        printf("当前词库：%s\n\n", g_current_vocab_file); // 显示当前使用的词库文件名

        char dat_files[50][256]; // 定义一个二维数组，用于存储当前目录下的.dat文件名，最多支持50个文件，每个文件名最长255字符
        int dat_count = 0; // 定义一个整数变量，用于记录当前目录下的.dat文件数量

        WIN32_FIND_DATA find_data; // 定义一个WIN32_FIND_DATA结构体变量，用于存储查找文件的信息
        HANDLE hFind = FindFirstFile("*.dat", &find_data); // 查找当前目录下的所有.dat文件，并将第一个找到的文件的信息存储在find_data中，返回一个句柄hFind用于后续操作
        if (hFind == INVALID_HANDLE_VALUE) { // 如果找到文件，继续查找
            printf("未找到任何.dat文件！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }

        do {// 循环查找所有.dat文件
            size_t len = strlen(find_data.cFileName);
            if (len > 10 && strcmp(find_data.cFileName + len - 10, "_backup.dat") == 0) {
                continue; // 跳过备份文件
            }
            if (dat_count >= 50) break; // 如果文件数量超过50，停止查找
            strncpy(dat_files[dat_count], find_data.cFileName, 255); // 将找到的.dat文件名复制到dat_files数组中
            dat_files[dat_count][255] = '\0'; // 确保字符串以null结尾，防止溢出
            dat_count++;
        } while (FindNextFile(hFind, &find_data));
        FindClose(hFind); // 关闭查找句柄

        if (dat_count == 0) {
            printf("未找到任何.dat文件！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }

        printf("可切换的词库文件:\n");// 输出当前目录下的所有.dat文件，供用户选择切换
        printf("==============================\n");
        for (int i = 0; i < dat_count; i++) {
            if (strcmp(dat_files[i], g_current_vocab_file) == 0) {
                printf("%d. %s (当前使用)\n", i + 1, dat_files[i]); // 如果文件名与当前使用的词库文件名相同，标记为当前使用
            } else {
                printf("%d. %s\n", i + 1, dat_files[i]); // 否则正常显示文件名
            }
        }
        printf("==============================\n");

        char file_name[256];
        printf("请输入要切换的词库文件名(不包括扩展名):\n");

        if (!safe_input(file_name, sizeof(file_name) - 4) || strlen(file_name) == 0) {
            printf("输入无效！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return; // 如果用户输入的文件名为空，提示用户并返回主菜单
        }
        // 添加.dat扩展名
        char new_file[256];
        snprintf(new_file, sizeof(new_file), "%s.dat", file_name); // 构造新的词库文件名，添加.dat扩展名
        
        // 检查文件是否存在
        FILE *fp = fopen(new_file, "rb");
        if (!fp) {
            printf("词库不存在！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }
        fclose(fp);

        // 切换
        strcpy(g_current_vocab_file, new_file); // 更新当前词库文件名为用户输入的文件
        generate_backup_filename(g_current_vocab_file, g_current_backup_vocab_file); // 根据新的当前词库文件名生成对应的备份文件名
        load_vocab(); // 加载新的词库信息
        printf("已切换到词库：%s\n", g_current_vocab_file); // 提示用户已切换到新的词库，并显示新词库的文件名
        printf("按回车键返回主菜单...");
        getchar();
    }
// generate_backup_filename
    void generate_backup_filename(const char *vocab_file,  char *backup_file) {// 生成词库文件的备份文件，备份文件名在同一目录下，并且具有相似的命名规则
        char tmp[256];
        // 先复制原名
        strncpy(tmp, vocab_file, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0'; // 确保字符串以空字符结尾

        // 去掉末尾的扩展名
        char *dot = strstr(tmp, ".dat"); // 查找扩展名
        if (dot) {
            *dot = '\0'; // 将扩展名替换为字符串结束符
        }
        // 拼接备份文件名
        snprintf(backup_file, 300, "%s_backup.dat", tmp); // 添加备份文件的后缀
    }
// is_file_same
    int is_file_same() {// 比较当前词库文件和备份文件是否相同的函数，返回1表示相同，返回0表示不同
        FILE *f1 = fopen(g_current_vocab_file, "rb");
        FILE *f2 = fopen(g_current_backup_vocab_file, "rb");

        if (!f1 || !f2) {
            if (f1) fclose(f1);
            if (f2) fclose(f2);
            return 0; // 如果任一文件无法打开，认为文件不同
        }
        // 逐字节比较两个文件的内容
        unsigned char c1, c2;
        while(1) {
            c1 = fgetc(f1);
            c2 = fgetc(f2);
            if (feof(f1) && feof(f2)) {
                fclose(f1);
                fclose(f2);
                return 1; // 如果两个文件都到达末尾，认为文件相同
            }
            if (c1 != c2) {
                fclose(f1);
                fclose(f2);
                return 0; // 如果读取的字节不同，认为文件不同
            }
        }
    }