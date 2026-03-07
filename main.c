#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<time.h>
#include<windows.h>

// 常量定义
#define MAX_WORD 2000// 最多存储2000个单词
#define MAX_STR 100// 每个字符串（英文单词和中文释义）的最大长度
#define FILE_NAME "vocab.dat"// 数据文件的名称，所有单词信息将保存在这个文件中

//结构体定义
    typedef struct {// 定义一个结构体类型Word，用于存储单词信息
        int id;// 单词ID，唯一标识每个单词
        char english[MAX_STR];// 英文单词
        char chinese[MAX_STR];// 中文释义
        int level;// 单词记忆等级，用于表示单词的熟悉程度
        time_t last_review;// 上次复习时间，记录单词上次被复习的时间
        time_t next_review;// 下次复习时间，记录单词下次需要复习的时间
        int correct_count;// 正确记忆次数，记录用户正确记忆该单词的次数
        int wrong_count;// 错误记忆次数，记录用户错误记忆该单词的次数
    } Word;
    
    typedef struct {// 定义一个结构体类型Vocab，用于存储整个词库的信息
        Word words[MAX_WORD];// 存储单词信息的数组，最多可以存储2000个单词
        int count;// 当前存储的单词数量，记录已经存储了多少个单词
    } Vocab;

// 全局变量
    Vocab g_vocab = {.count = 0};// 全局变量，存储当前的单词信息，初始化单词数量为0, 显性初始化结构体，确保所有字段都被正确初始化

// 函数声明
void clear_screen(); // 清屏函数，清除控制台上的内容
void trim_newline(char *s); // 去除字符串末尾的换行符函数
int safe_input(char *buf, int size); // 安全输入函数，限制输入长度
int strcasecmp_custom(const char *a,const char *b); // 自定义的字符串比较函数，忽略大小写
void save_vocab(); // 保存单词信息到文件的函数
void load_vocab(); // 从文件中加载单词信息的函数
int is_duplicate(const char *english); // 检查是否有重复的英文单词的函数
void add_word(); // 添加新单词到词库中的函数

// 主函数
int main() {

//强制控制台输入输出使用UTF-8编码，确保中文显示正常
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出编码为UTF-8
    SetConsoleCP(CP_UTF8); // 设置控制台输入编码为UTF-8

    memset(&g_vocab, 0, sizeof(Vocab)); // 初始化全局变量g_vocab的内存空间，确保所有字段都被初始化为0，避免内存垃圾值导致崩溃
    g_vocab.count = 0;// 初始化单词数量为0，表示当前没有存储任何单词
    load_vocab(); // 从文件中加载单词信息到全局变量g_vocab中

    int choice;
    do {
        clear_screen(); // 清屏函数，清除控制台上的内容
        printf("\n============艾宾浩斯单词记忆系统============\n");
        printf("当前词库单词数: %d\n", g_vocab.count); // 显示当前词库中存储的单词数量
        printf("1.录入新单词\n");
        printf("2.退出系统\n");
        printf("请输入你的选择(1/2): ");

        if (scanf("%d", &choice) != 1) { // 读取用户输入的选择，如果输入不是整数，提示用户输入无效并继续循环
            choice = 0; // 将choice设置为0，表示无效的选择 
            while (getchar() != '\n'); // 清除输入缓冲区中的无效输入，直到遇到换行符为止
        }

        getchar();
        switch (choice) {
            case 1:// 如果用户选择1，进入录入新单词的流程
                add_word(); // 调用函数添加新单词到词库中
                break;
            case 2:// 如果用户选择2，进入退出系统的流程
                printf("正在保存数据...\n");
                save_vocab(); // 调用函数将当前的单词信息保存到文件中
                printf("数据保存成功！再见！\n");
                break;
            default:// 如果用户输入了无效的选择，提示用户重新输入
                printf("无效的选择，请重新输入！\n");
                printf("按回车键继续...");
                getchar(); // 等待用户按下回车键继续操作
        }
    } while (choice != 2); // 循环直到用户选择退出系统

    return 0;
}

// 函数定义
    void clear_screen() {// 清屏函数，根据操作系统不同使用不同的命令清屏
        #ifdef _WIN32
            system("cls"); // Windows系统使用cls命令清屏
        #else
            system("clear"); // 其他系统使用clear命令清屏
        #endif
    }

    void trim_newline(char *s) {// 去除字符串末尾的换行符函数
        size_t len = strlen(s);// 获取字符串的长度
        if (len > 0 && s[len - 1] == '\n') // 如果字符串末尾是换行符
            s[len - 1] = '\0'; // 将末尾的换行符替换为字符串结束符
    }

    int safe_input(char *buf, int size) {// 安全输入函数，限制输入长度
        if (fgets(buf, size, stdin) == NULL)// 使用fgets函数读取输入，如果输入失败，返回NULL
            return 0; // 如果输入失败，返回0
        trim_newline(buf); // 去除输入字符串末尾的换行符
        return 1; // 输入成功，返回1
    }

    int strcasecmp_custom(const char *a,const char *b) {// 自定义的字符串比较函数，忽略大小写
        while (*a && *b) {
         // 转换为小写（加unsigned char避免负数字符问题）
            unsigned char c1 = tolower((unsigned char)*a);// 将当前字符转换为小写
            unsigned char c2 = tolower((unsigned char)*b);// 将当前字符转换为小写
            if (c1 != c2) {// 如果两个字符不相等，返回它们的差值
              return c1 - c2;
            }
         a++;
         b++;
       }
    // 处理长度不同的情况：一个字符串已结束，另一个未结束
    return (*a != '\0') ? 1 : ((*b != '\0') ? -1 : 0);
    }

    void save_vocab() {// 保存单词信息到文件的函数
        if (g_vocab.count < 0 || g_vocab.count > MAX_WORD) {// 检查单词数量是否在合理范围内，防止异常数据导致文件写入错误
            printf("错误:单词数量异常，无法保存！\n");
            return; 
        }

        FILE *fp = fopen(FILE_NAME, "wb");// 以二进制写入模式打开文件，如果文件不存在会创建新文件
        if (!fp) {
            printf("错误:无法打开文件保存！\n");
            return;
        }

        size_t write_count = fwrite(&g_vocab.count, sizeof(int), 1, fp);// 首先写入单词数量，如果写入失败，说明文件可能无法写入
        if (write_count != 1) {
            printf("错误:保存单词数量失败！\n");
            fclose(fp); // 关闭文件
            return; 
        }

        if (g_vocab.count > 0) {// 如果有单词需要写入，才写入单词信息数组
            write_count = fwrite(g_vocab.words, sizeof(Word), g_vocab.count, fp);// 然后写入单词信息数组，如果写入的数量不匹配，说明文件可能无法写入
            if (write_count != g_vocab.count) {
                printf("错误:保存单词数据不完整！\n");
                fclose(fp); // 关闭文件
                return;
            }
        }

        fclose(fp); // 关闭文件
        printf("数据保存成功！\n");
    }

    void load_vocab() {// 从文件中加载单词信息的函数
        memset(&g_vocab, 0, sizeof(Vocab));// 初始化全局变量g_vocab的内存空间，确保所有字段都被初始化为0，避免内存垃圾值导致崩溃
        g_vocab.count = 0;// 初始化单词数量为0，表示当前没有存储任何单词

        FILE *fp = fopen(FILE_NAME, "rb");// 以二进制读取模式打开文件，如果文件不存在会返回NULL
        if (!fp) {
            printf("首次运行，未找到词库文件，已初始化空词库！\n");
            return;
        }

        size_t read_size = fread(&g_vocab.count, sizeof(int), 1, fp);// 从文件中读取单词数量，如果读取失败，说明文件可能损坏
        if (read_size != 1 || g_vocab.count < 0 || g_vocab.count > MAX_WORD) {// 检查读取的单词数量是否在合理范围内，防止异常数据导致后续读取错误
            printf("词库文件损坏，初始化空词库! \n");
            fclose(fp); // 关闭文件
            g_vocab.count = 0;// 初始化单词数量为0，表示当前没有存储任何单词
            return;
        }

        if (g_vocab.count > 0) {// 如果有单词需要读取，才从文件中读取单词信息数组
            read_size = fread(g_vocab.words, sizeof(Word), g_vocab.count, fp);// 从文件中读取单词信息数组，如果读取的数量不匹配，说明文件可能损坏
            if (read_size != g_vocab.count){// 如果读取的数量不匹配，说明文件可能损坏
                printf("词库文件数据不完整，初始化空词库！\n");
                fclose(fp); // 关闭文件
                g_vocab.count = 0;// 初始化单词数量为0，表示当前没有存储任何单词
                return;
            }
        }

        fclose(fp); // 关闭文件
        printf("已加载 %d个单词!\n", g_vocab.count); // 打印加载的单词数量
    }

    int is_duplicate(const char *english) {// 检查是否有重复的英文单词的函数
        for (int i = 0; i < g_vocab.count; i++) {// 遍历当前存储的单词信息数组
            if (strcasecmp_custom(g_vocab.words[i].english, english) == 0) // 使用自定义的字符串比较函数检查是否有重复的英文单词
                return 1; // 如果找到重复的单词，返回1
        }
        return 0; // 如果没有找到重复的单词，返回0
    }

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
        if (!safe_input(en, MAX_STR)) {
            printf("输入无效，取消录入。 \n");
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
        strncpy(new_word.chinese, zh, MAX_STR - 1); // 将输入的中文释义复制到新单词的chinese字段
        new_word.level = 0; // 初始化单词记忆等级为0
        new_word.last_review = 0; // 初始化上次复习时间为0
        new_word.next_review = time(NULL) + 24 * 3600; // 设置下次复习时间为当前时间加上24小时
        new_word.correct_count = 0; // 初始化正确记忆次数为0
        new_word.wrong_count = 0; // 初始化错误记忆次数为0

        // 将新单词添加到词库中
        g_vocab.words[g_vocab.count] = new_word; // 将新单词添加到单词信息数组中
        g_vocab.count++; // 增加单词数量

        printf("单词 %s 添加成功！\n", en); // 提示用户单词添加成功
        save_vocab(); // 保存当前的单词信息到文件中
        printf("按回车键继续...");
        getchar(); // 等待用户按下回车键继续操作
    }