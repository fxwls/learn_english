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
// 艾宾浩斯记忆等级对应的复习间隔（秒），根据记忆等级设置不同的复习间隔时间
#define LEVEL_0_INTERVAL 300// 记忆等级0的复习间隔为5分钟
#define LEVEL_1_INTERVAL 1800// 记忆等级1的复习间隔为30分钟
#define LEVEL_2_INTERVAL 3600// 记忆等级2的复习间隔为1小时
#define LEVEL_3_INTERVAL 43200// 记忆等级3的复习间隔为12小时
#define LEVEL_4_INTERVAL 86400// 记忆等级4的复习间隔为1天
#define LEVEL_5_INTERVAL 172800// 记忆等级5的复习间隔为2天
#define LEVEL_6_INTERVAL 604800// 记忆等级6的复习间隔为7天
#define LEVEL_7_INTERVAL 2592000// 记忆等级7的复习间隔为30天
#define MAX_LEVEL 7// 最大记忆等级为7，表示单词已经非常熟悉了

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

typedef enum {// 定义一个枚举类型TestMode，用于表示单词测试的模式
    MODE_CN_TO_EN, // 中译英测试模式
    MODE_EN_TO_CN  // 英译中测试模式  
} TestMode;
TestMode current_test_mode = MODE_CN_TO_EN; // 当前的测试模式，默认为中译英测试模式

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

int get_need_review_count(); // 获取需要复习的单词数量的函数
void sort_words_by_review(); // 按复习紧迫度排序单词的比较
int compare_word_by_review(const void *a, const void *b); // 按复习紧迫度排序单词的比较函数（用于qsort函数）
void format_time(time_t t, char *buf, int buf_size); // 时间格式化函数，将时间戳转换为可读的日期时间字符串
void show_word_detail(Word *word); // 显示单词详细信息的函数
int quiz_word(Word *word); // 单词测试函数，根据当前的测试模式调用相应的测试函数进行单词测试，返回1表示用户回答正确，返回0表示用户回答错误
int quiz_cn_to_en(Word *word); // 单词测试（中译英），返回1=正确，0=错误的函数
int quiz_en_to_cn(Word *word); // 单词测试（英译中），返回1=正确，0=错误的函数
void update_word_level(Word *word, int is_correct); // 更新单词记忆等级和复习时间的函数，根据用户的测试结果调整单词的记忆等级，并计算下次复习时间
void review_words(); // 复习待复习单词主函数
void select_test_mode(); // 选择测试模式的函数，允许用户选择中译英测试还是英译中测试


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
        printf("当前词库单词数: %d | 待复习单词数: %d\n", g_vocab.count, get_need_review_count()); // 显示当前词库中的单词数量和需要复习的单词数量
        printf("1.录入新单词\n");
        printf("2.复习单词\n");
        printf("3.退出系统\n");
        printf("请输入你的选择(1/2/3): ");

        if (scanf("%d", &choice) != 1) { // 读取用户输入的选择，如果输入不是整数，提示用户输入无效并继续循环
            choice = 0; // 将choice设置为0，表示无效的选择 
            while (getchar() != '\n'); // 清除输入缓冲区中的无效输入，直到遇到换行符为止
        }

        getchar();
        switch (choice) {
            case 1:// 如果用户选择1，进入录入新单词的流程
                add_word(); // 调用函数添加新单词到词库中
                break;
            case 2:// 如果用户选择2，进入复习单词的流程
                review_words(); // 调用函数进行单词复习
                break;
            case 3:// 如果用户选择3，进入退出系统的流程
                printf("正在保存数据...\n");
                save_vocab(); // 调用函数将当前的单词信息保存到文件中
                printf("数据保存成功！再见！\n");
                break;
            default:// 如果用户输入了无效的选择，提示用户重新输入
                printf("无效的选择，请重新输入！\n");
                printf("按回车键继续...");
                getchar(); // 等待用户按下回车键继续操作
        }
    } while (choice != 3); // 循环直到用户选择退出系统

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
        new_word.next_review = time(NULL) - 100; // 设置下次复习时间为当前时间减去100秒，表示新单词需要立即复习
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
    
   
    int get_need_review_count() {// 获取需要复习的单词数量的函数
        int count = 0;
        time_t now = time(NULL);// 获取当前时间
        for (int i = 0; i < g_vocab.count; i++) {// 遍历当前存储的单词信息数组
            if (g_vocab.words[i].next_review <= now) {// 如果单词的下次复习时间小于或等于当前时间，说明需要复习
            count++;
            } 
        }
        return count;
    }

    
    void sort_words_by_review() {// 按复习紧迫度排序单词的比较函数（快速排序使用）
        if (g_vocab.count <= 1) return; // 如果单词数量小于或等于1，不需要排序

        qsort(g_vocab.words, g_vocab.count, sizeof(Word), compare_word_by_review); // 使用标准库函数qsort对单词信息数组进行排序，按照复习紧迫度排序
    }

    int compare_word_by_review(const void *a, const void *b) {// 按复习紧迫度排序单词的比较函数（用于qsort函数）
        const Word *word1 = (const Word *)a;
        const Word *word2 = (const Word *)b;
        if (word1->next_review < word2->next_review) return -1;// 如果第一个单词的下次复习时间小于第二个单词的下次复习时间，返回-1表示第一个单词需要更紧迫地复习
        if (word1->next_review > word2->next_review) return 1;// 如果第一个单词的下次复习时间大于第二个单词的下次复习时间，返回1表示第二个单词需要更紧迫地复习
        return 0;
    }

    
    void format_time(time_t t, char *buf, int buf_size) {// 时间格式化函数，将时间戳转换为可读的日期时间字符串
        if (t == 0) {// 如果时间戳为0，表示单词还没有被复习过，返回"未复习"
            strncpy(buf, "未复习", buf_size - 1);
            buf[buf_size -1] = '\0';
            return;
        }
        struct tm *tm_info = localtime(&t);// 将时间戳转换为本地时间的结构体tm_info
        strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", tm_info);// 将tm_info中的时间信息格式化为"年-月-日 时:分:秒"的字符串，并存储在buf中
    }

    
    void show_word_detail(Word *word) {// 显示单词详细信息的函数
        char last_review_str[30], next_review_str[30];// 定义两个字符串变量，用于存储格式化后的上次复习时间和下次复习时间
        format_time(word->last_review, last_review_str, sizeof(last_review_str)); // 将单词的上次复习时间格式化为可读的字符串
        format_time(word->next_review, next_review_str, sizeof(next_review_str)); // 将单词的下次复习时间格式化为可读的字符串
        
        printf("\n===单词详情===\n");
        printf("ID: %d\n", word->id); // 显示单词ID
        printf("英文: %s\n", word->english); // 显示英文单词
        printf("中文: %s\n", word->chinese); // 显示中文
        printf("记忆等级: %d (最高7级)\n", word->level); // 显示单词记忆等级
        printf("上次复习: %s\n", last_review_str); // 显示单词的上次复习时间
        printf("下次复习: %s\n", next_review_str); // 显示单词的下次复习时间
        printf("正确次数：%d | 错误次数：%d\n", word->correct_count, word->wrong_count); // 显示单词的正确记忆次数和错误记忆次数
    }

    
    int quiz_word(Word *word) {// 单词测试函数，根据当前的测试模式调用相应的测试函数进行单词测试，返回1表示用户回答正确，返回0表示用户回答错误
        if (current_test_mode == MODE_EN_TO_CN) {
            return quiz_en_to_cn(word); // 如果当前测试模式是英译中，调用quiz_en_to_cn函数进行测试
        } else {
            return quiz_cn_to_en(word); // 如果当前测试模式是中译英，调用quiz_cn_to_en函数进行测试
        }
    }

    int quiz_cn_to_en(Word *word) {// 单词测试（中译英），返回1=正确，0=错误的函数
        char input[MAX_STR];// 定义一个字符串变量，用于存储用户输入的英文单词
        printf("\n【中译英】%s\n", word->chinese); // 提示用户输入单词的英文翻译
        safe_input(input, MAX_STR); // 获取用户输入的英文单词

        if (strcasecmp_custom(input,word->english) == 0) {
            printf("回答正确！\n");
            return 1; // 如果用户输入的英文单词与单词的英文字段匹配，返回1表示正确
        } else {
            printf("回答错误！正确答案是：%s\n", word->english); // 如果用户输入的英文单词与单词的英文字段不匹配，提示用户正确答案并返回0表示错误
            return 0;
        }
    }

    int quiz_en_to_cn(Word *word) {// 单词测试（英译中），返回1=正确，0=错误的函数
        char input[MAX_STR];// 定义一个字符串变量，用于存储用户输入的中文释义
        printf("\n【英译中】%s\n", word->english); // 提示用户输入单词的中文释义
        safe_input(input, MAX_STR); // 获取用户输入的中文释义

        if (strcasecmp_custom(input,word->chinese) == 0) {
            printf("回答正确！\n");
            return 1; // 如果用户输入的中文释义与单词的中文字段匹配，返回1表示正确
        } else {
            printf("回答错误！正确答案是：%s\n", word->chinese); // 如果用户输入的中文释义与单词的中文字段不匹配，提示用户正确答案并返回0表示错误
            return 0;
        }
    }

    
    void update_word_level(Word *word, int is_correct) {// 更新单词记忆等级和复习时间的函数，根据用户的测试结果调整单词的记忆等级，并计算下次复习时间
        if (word == NULL) return;

        time_t now = time(NULL);// 获取当前时间
        word->last_review = now;// 更新单词的上次复习时间为当前时间

        if (is_correct) {
            word->correct_count++;// 如果用户回答正确，增加单词的正确记忆次数
            if (word->level < MAX_LEVEL) {// 如果单词的记忆等级小于最大等级，增加单词的记忆等级
                word->level++;
            }

            switch (word->level) {// 根据单词的记忆等级设置下次复习时间，使用预定义的复习间隔常量
                case 0: word->next_review = now + LEVEL_0_INTERVAL; break;
                case 1: word->next_review = now + LEVEL_1_INTERVAL; break;
                case 2: word->next_review = now + LEVEL_2_INTERVAL; break;
                case 3: word->next_review = now + LEVEL_3_INTERVAL; break;
                case 4: word->next_review = now + LEVEL_4_INTERVAL; break;
                case 5: word->next_review = now + LEVEL_5_INTERVAL; break;
                case 6: word->next_review = now + LEVEL_6_INTERVAL; break;
                case 7: word->next_review = now + LEVEL_7_INTERVAL; break;
            }
        } else {
            word->wrong_count++;// 如果用户回答错误，增加单词的错误记忆次数
            if (word->level > 0) {
                word->level--;
            }
            word->next_review = now + LEVEL_0_INTERVAL; // 错误时立即安排下一次复习，间隔为5分钟
        }
    }

    
    void review_words() {// 复习待复习单词主函数
        clear_screen();
        printf("\n=======单词复习=======\n");

        int need_review = get_need_review_count();
        if (need_review == 0) {
            printf("当前暂无需要复习的单词！\n");
            printf("按回车键返回主菜单...");
            getchar();
            return;
        }

        printf("待复习单词数量：%d\n", need_review);
        select_test_mode(); // 选择测试模式，允许用户选择中译英测试还是英译中测试
        printf("按回车键开始复习...");
        getchar();

        sort_words_by_review(); // 按复习紧迫度排序单词，确保最需要复习的单词优先出现

        int reviewed = 0, correct = 0;
        time_t now = time(NULL);

        // 遍历所有单词，复习需要复习的单词
        for (int i = 0; i <g_vocab.count; i++) {
            Word *word = &g_vocab.words[i];
            if (word->next_review > now) continue; 

            clear_screen();
            printf("【复习进度】%d/%d\n", reviewed + 1, need_review);

            // 进行单词测试，获取用户的测试结果
            int is_correct = quiz_word(word);
            if (is_correct) correct++;
            reviewed++;
        
            // 更新单词状态
            update_word_level(word, is_correct);
        

            printf("\n按回车键继续复习（输入q退出复习）...");
            char quit[10];
            safe_input(quit, sizeof(quit));
            if (strcasecmp_custom(quit, "q") == 0) {
                break; // 如果用户输入q，退出复习循环
            }

        }

        // 保存复习后的状态
        save_vocab();
        printf("\n复习完成！本次复习：%d个单词，正确率%.2f%%\n", reviewed, (reviewed > 0) ? (correct * 100.0 / reviewed) : 0);
        printf("按回车键返回主菜单...");
        getchar();
    }

    void select_test_mode() {// 选择测试模式的函数，允许用户选择中译英测试还是英译中测试
        int choice;
        printf("\n请选择测试模式：\n");
        printf("1. 中译英测试\n");
        printf("2. 英译中测试\n");
        printf("请输入你的选择(1/2): ");

        if (scanf("%d", &choice) != 1 ) {
            choice = 0;
            while (getchar() != '\n');
        }
        getchar(); // 清除输入缓冲区中的换行符

        if (choice == 2) {
            current_test_mode = MODE_EN_TO_CN; // 设置测试模式为英译中
        } else if (choice == 1) {
            current_test_mode = MODE_CN_TO_EN; // 默认设置测试模式为中译英
        } else {
            printf("无效的选择，默认使用中译英测试模式！\n");
             current_test_mode = MODE_CN_TO_EN; // 如果用户输入无效，默认设置测试模式为中译英
        }
    }





