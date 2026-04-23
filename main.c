#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<time.h>
#include<windows.h>
#include<math.h>
#include<sys/stat.h>
#include<errno.h>


// 常量定义
#define MAX_WORD 2000// 最多存储2000个单词
#define MAX_STR 1000// 每个字符串（英文单词和中文释义）的最大长度
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
//加密
#define ENCRYPT_KEY 0x7B// 加密密钥
// 控制台颜色（美化版）
#define COLOR_DEFAULT  7    // 白色
#define COLOR_MENU_TEXT 7    // 菜单白色
#define COLOR_TITLE    11   // 标题蓝色
#define COLOR_SUCCESS  10   // 成功绿色
#define COLOR_WARN     14   // 提示黄色
#define COLOR_ERROR    12   // 错误红色


//结构体定义
typedef struct {// 定义一个结构体类型DailyStat，用于存储每天的复习统计信息
    int date;         // 复习日期，格式为YYYYMMDD
    int total_count;  // 复习单词总数，记录当天复习的单词总数量
    int correct_count;// 正确复习单词数，记录当天正确复习的单词数量
} DailyStat;

typedef struct {// 定义一个结构体类型Word，用于存储单词信息
    int id;// 单词ID，唯一标识每个单词
    char english[MAX_STR];// 英文单词
    char chinese[MAX_STR];// 中文释义
    int level;// 单词记忆等级，用于表示单词的熟悉程度
    float stability; // 记忆稳定性
    time_t last_review;// 上次复习时间，记录单词上次被复习的时间
    time_t next_review;// 下次复习时间，记录单词下次需要复习的时间
    int correct_count;// 正确记忆次数，记录用户正确记忆该单词的次数
    int wrong_count;// 错误记忆次数，记录用户错误记忆该单词的次数
    int is_mistake;// 错词判断
} Word;
    
typedef struct {// 定义一个结构体类型Vocab，用于存储整个词库的信息
    Word words[MAX_WORD];// 存储单词信息的数组，最多可以存储2000个单词
    int count;// 当前存储的单词数量，记录已经存储了多少个单词
    int last_add_date;// 最近添加单词的日期，记录最近一次添加单词的日期，用于统计每天添加单词的数量
    int checksum;// 校验和，用于检测词库文件是否损坏
    DailyStat daily_stats[30];// 存储每天的复习统计信息的数组，最多可以存储30天的数据
    int daily_stats_count;// 当前存储的复习统计信息数量，记录已经存储了多少天的复习统计信息
    float gain;// 答对时稳定性增长系数，初始值为0.10
    float loss;// 答错时稳定性减少系数，初始值为0.30
    int total_review;// 总复习次数
    int continuous_days;// 连续复习天数
    int last_study_date;// 最近复习的日期
    int daily_goal;// 每天的目标单词数
} Vocab;


typedef enum {// 定义一个枚举类型TestMode，用于表示单词测试的模式
    MODE_CN_TO_EN, // 中译英测试模式
    MODE_EN_TO_CN  // 英译中测试模式  
} TestMode;
TestMode current_test_mode = MODE_CN_TO_EN; // 当前的测试模式，默认为中译英测试模式

// 全局变量
Vocab g_vocab = {.count = 0};// 全局变量，存储当前的单词信息，初始化单词数量为0, 显性初始化结构体，确保所有字段都被正确初始化
time_t g_mock_time = 0;// 全局变量，存储模拟时间，初始值为0
// 文件名定义(可自定义词库名称，但请勿修改文件扩展名，保持为.dat以启用加密功能)
char g_current_vocab_file[256] = "vocab.dat";// 当前词库文件名
char g_current_backup_vocab_file[300] = "vocab_backup.dat";// 备份词库文件名


// 函数声明
void set_color(int color);// 设置字体颜色的函数
void clear_screen(); // 清屏函数，清除控制台上的内容
void trim_newline(char *s); // 去除字符串末尾的换行符函数
int safe_input(char *buf, int size); // 安全输入函数，限制输入长度
int strcasecmp_custom(const char *a,const char *b); // 自定义的字符串比较函数，忽略大小写
void save_vocab(); // 保存单词信息到文件的函数
void load_vocab(); // 从文件中加载单词信息的函数
int is_duplicate(const char *english); // 检查是否有重复的英文单词的函数
void add_word(); // 添加新单词到词库中的函数

int get_need_review_count(); // 获取需要复习的单词数量的函数
int is_valid_english(const char *word); // 检查英文单词是否只包含英文字母的函数
void format_time(time_t t, char *buf, int buf_size); // 时间格式化函数，将时间戳转换为可读的日期时间字符串
void show_word_detail(Word *word); // 显示单词详细信息的函数
int quiz_word(Word *word); // 单词测试函数，根据当前的测试模式调用相应的测试函数进行单词测试，返回1表示用户回答正确，返回0表示用户回答错误
int quiz_cn_to_en(Word *word); // 单词测试（中译英），返回1=正确，0=错误的函数
int quiz_en_to_cn(Word *word); // 单词测试（英译中），返回1=正确，0=错误的函数

void calibrate_params(Word *w, int is_correct, time_t now);// 校准单词的参数
void update_stability(Word *w, int is_correct, time_t now);// 更新单词的稳定性，根据用户的测试结果调整单词的稳定性
void update_word_level(Word *word, int is_correct); // 更新单词记忆等级和复习时间的函数，根据用户的测试结果调整单词的记忆等级，并计算下次复习时间
void review_words(); // 复习待复习单词主函数
void select_test_mode(); // 选择测试模式的函数，允许用户选择中译英测试还是英译中测试

void show_review_rank();//复习排行榜
double forgetting_probability(Word *w, time_t now);//计算忘记概率(越高表示越容易忘记)
int compare_word_ptr_by_forgetting(const void *a, const void *b);//比较函数

void review_mistakes();// 专项复习错词
void trim(char *str);// 去除字符串两端的空格

int time_to_date(time_t t);// 时间戳转换为日期（年月日）的函数，返回一个整数表示日期，格式为YYYYMMDD
int get_today();// 获取今天的日期，返回一个整数表示日期，格式为YYYYMMDD

void encrypt_data(void *data, int len);// 加密数据的函数，使用简单的异或加密方法对数据进行加密
void decrypt_data(void *data, int len);// 解密数据的函数，使用与加密相同的方法对数据进行解密
int calculate_checksum(Vocab *vocab);// 计算校验和的函数，根据词库中的单词信息计算一个校验和，用于检测文件是否损坏
void backup_vocab(); // 备份单词信息到加密文件的函数，将当前的单词信息加密后保存到一个备份文件中，以防止数据丢失或被未授权访问
int is_file_same() ;// 检查当前词库文件和备份文件是否相同的函数，以此来判断是否损坏或被篡改

void generate_backup_filename(const char *vocab_file,  char *backup_file);// 生成备份文件名的函数，根据当前的词库文件名生成一个对应的备份文件名，确保备份文件与词库文件在同一目录下，并且具有相似的命名规则

void create_new_vocab();// 新建词库
void switch_vocab(); // 切换词库
void restore_vocab(); // 从备份恢复词库

DailyStat* get_today_daily_stat();// 获取今天的日志
void write_daily_log();// 写入日志
void show_statistics();// 显示复习统计
time_t date_to_time_t(int date);// 日期相邻判断时间戳转换

void reset_learning_params();// 重置学习参数


time_t get_current_time(void);// 获取模拟时间
void set_mock_time();// 设置模拟时间

void set_daily_goal();// 设置每日学习目标

void browse_all_words();// 浏览所有单词
void search_word();// 搜索单词
void edit_word();// 编辑单词
void delete_word();// 删除单词


// 主函数
int main() {

//强制控制台输入输出使用UTF-8编码，确保中文显示正常
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8); // 设置控制台输出编码为UTF-8
        SetConsoleCP(CP_UTF8); // 设置控制台输入编码为UTF-8
    #else
        setlocale(LC_ALL, ""); // Linux 使用系统 locale 支持 UTF-8
    #endif

    memset(&g_vocab, 0, sizeof(Vocab)); // 初始化全局变量g_vocab的内存空间，确保所有字段都被初始化为0，避免内存垃圾值导致崩溃
    g_vocab.count = 0;// 初始化单词数量为0，表示当前没有存储任何单词
    g_vocab.gain = 0.10f;// 初始化答对时稳定性增长系数为0.10
    g_vocab.loss = 0.30f;// 初始化答错时稳定性减少系数为0.30
    g_vocab.total_review = 0;// 初始化总复习次数为0，表示当前没有进行任何复习操作
    load_vocab(); // 从文件中加载单词信息到全局变量g_vocab中
    printf("按回车键继续...");
    getchar(); // 等待用户按下回车键继续操作

    int choice;
    do {
        clear_screen(); // 清屏函数，清除控制台上的内容

        // 统计今天新增的单词数量，统计最近添加单词的日期与今天的日期是否相同，如果相同则统计为今天新增的单词
        int today = get_today(); 
        int today_add = 0;
        for (int i = 0; i < g_vocab.count; i++) {
            if (time_to_date(g_vocab.words[i].last_review) == today) {
                today_add++;
            }
        }
        set_color(COLOR_TITLE);
        printf("\n===================================================\n");
        printf("               艾宾浩斯单词记忆系统                 \n");
        printf("===================================================\n");
        set_color(COLOR_DEFAULT);

        printf("  当前词库：%-18s  总单词：%d\n", g_current_vocab_file, g_vocab.count);
        printf("  待复习：%-18d  今日新增：%d\n", get_need_review_count(), today_add);
        printf("---------------------------------------------------\n");

        set_color(COLOR_MENU_TEXT);
        printf("  [1] 录入单词    [2] 开始复习    [3] 浏览所有\n");
        printf("  [4] 搜索单词    [5] 复习排行    [6] 错词本\n");
        printf("  [7] 学习统计    [8] 词库管理    [9] 编辑单词\n");
        printf("  [10] 删除单词   [11] 设置       [12] 退出系统\n");
        set_color(COLOR_DEFAULT);

        set_color(COLOR_WARN);
        printf("\n  请选择功能(1-11)：");
        set_color(COLOR_DEFAULT);

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
            case 3:// 如果用户选择3，进入查看所有单词的流程
                browse_all_words(); // 调用函数浏览所有单词
                break;
            case 4:// 如果用户选择4，进入搜索单词的流程
                search_word(); // 调用函数搜索单词
                break;
            case 5:// 如果用户选择5，打开复习排行榜
                show_review_rank();
                break;
            case 6:// 如果用户选择6，打开专项复习错词
                review_mistakes();
                break;
            case 7:// 如果用户选择7，进入学习统计可视化的流程
                show_statistics();
                break;
            case 8:// 如果用户选择8，进入词库管理的流程
                clear_screen();
                set_color(COLOR_TITLE);
                printf("==================== 词库管理 ====================\n");
                set_color(COLOR_DEFAULT);
                printf("  [1] 新建词库    [2] 切换词库    [3] 恢复备份\n");
                set_color(COLOR_WARN);
                printf("\n  请选择(1-3)：");
                set_color(COLOR_DEFAULT);

                int sub_choice;
                if (scanf("%d", &sub_choice) != 1) {// 读取用户输入的选择，如果输入不是整数，提示用户输入无效并继续循环
                    sub_choice = 0;
                    while (getchar() != '\n');
                }
                getchar();// 清除scanf留下的换行符
                switch (sub_choice) {
                    case 1:
                        create_new_vocab();
                        break;
                    case 2:
                        switch_vocab();
                        break;
                    case 3:
                        restore_vocab();
                        break;
                    default:
                        printf("无效的选择，请重新输入！\n");
                        printf("按回车键继续...");
                        getchar();
                }
                break;
            case 9:// 如果用户选择9，进入编辑单词的流程
                edit_word(); // 调用函数编辑单词
                break;
            case 10:// 如果用户选择10，进入删除单词的流程
                delete_word(); // 调用函数删除单词
                break;
            case 11:// 如果用户选择11，进入设置的流程
                clear_screen();
                set_color(COLOR_TITLE);
                printf("==================== 设置 ====================\n");
                set_color(COLOR_DEFAULT);
                printf("  [1] 重置参数    [2] 时间模拟    [3] 每日目标\n");
                set_color(COLOR_WARN);
                printf("\n  请选择(1-3)：");
                set_color(COLOR_DEFAULT);
                int sub_choice2;
                if (scanf("%d", &sub_choice2) != 1) {// 读取用户输入的选择，如果输入不是整数，提示用户输入无效并继续循环
                    sub_choice = 0;
                    while (getchar() != '\n');
                }
                getchar();// 清除scanf留下的换行符
                switch (sub_choice2) {
                    case 1:
                        reset_learning_params();
                        break;
                    case 2:
                        set_mock_time();
                        break;
                    case 3:
                        set_daily_goal();
                        break;
                    default:
                        printf("无效的选择，请重新输入！\n");
                        printf("按回车键继续...");
                        getchar();
                }
                break;
            case 12:// 如果用户选择11，进入退出系统的流程
                printf("正在保存数据...\n");
                save_vocab(); // 调用函数将当前的单词信息保存到文件中
                backup_vocab(); // 调用函数将当前的单词信息加密后保存到备份文件中，以防止数据丢失或被未授权访问
                printf("数据保存成功！再见！\n");
                printf("提示：备份文件已保存为 %s，建议定期备份以防止数据丢失！\n", g_current_backup_vocab_file);
                printf("按回车键继续...");
                getchar(); // 等待用户按下回车键继续操作
                break;
            default:// 如果用户输入了无效的选择，提示用户重新输入
                printf("无效的选择，请重新输入！\n");
                printf("按回车键继续...");
                getchar(); // 等待用户按下回车键继续操作
        }
    } while (choice != 12); // 循环直到用户选择退出系统

    return 0;
}

// 函数定义
    void set_color(int color) {// 设置字体颜色的函数
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);    
    }

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
        trim(buf);// 去除输入字符串两端的空格
        if (strlen(buf) == 0) {
            return 0; // 如果输入的字符串长度为0，返回0表示输入无效
        }
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

    int is_duplicate(const char *english) {// 检查是否有重复的英文单词的函数
        for (int i = 0; i < g_vocab.count; i++) {// 遍历当前存储的单词信息数组
            if (strcasecmp_custom(g_vocab.words[i].english, english) == 0) // 使用自定义的字符串比较函数检查是否有重复的英文单词
                return 1; // 如果找到重复的单词，返回1
        }
        return 0; // 如果没有找到重复的单词，返回0
    }

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
        printf("记忆稳定性: %.1f秒(%.2f小时)\n", word->stability, word->stability / 3600); // 显示单词记忆稳定性
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

    void calibrate_params(Word *w, int is_correct, time_t now) {// 校准单词的参数
        if (w->last_review == 0) {
            return;
        }

        double elapsed = difftime(now, w->last_review);// 计算从上次复习到现在的时间差，单位为秒
        if (elapsed <= 0) return;// 如果时间差小于等于0，返回，不进行校准

        double R = exp(-elapsed / w->stability);// 计算遗忘概率
        double predicted = R;
        double actual = is_correct ? 1.0 : 0.0;// 根据用户回答是否正确计算实际正确率
        double error = actual - predicted;// 计算预测错误率(正：低估用户，负：高估用户)

        double lr = 0.05 / (1.0 + g_vocab.total_review / 500.0);// 初始0.05，每500次复习减半
        if (lr < 0.002) lr = 0.002;// 最低学习率0.002

        float raw_gain = g_vocab.gain;// 记录原始增长系数
        float raw_loss = g_vocab.loss;// 记录原始减少系数

        if (is_correct) {// 如果用户回答正确
            raw_gain += (float)(lr * error);// 更新增长系数
        } else {
            raw_loss -= (float)(lr * error);// 更新增加系数
        }

        if (raw_gain < 0.02f) raw_gain = 0.02f;
        if (raw_gain > 0.50f) raw_gain = 0.50f;
        if (raw_loss < 0.05f) raw_loss = 0.05f;
        if (raw_loss > 0.80f) raw_loss = 0.80f;

        float alpha = 0.3f;// 指数移动平均平滑
        g_vocab.gain = alpha * raw_gain + (1.0f - alpha) * g_vocab.gain;// 更新增长系数
        g_vocab.loss = alpha * raw_loss + (1.0f - alpha) * g_vocab.loss;// 更新减少系数
    }

    void update_stability(Word *w, int is_correct, time_t now) {// 动态更新单词的记忆稳定性（秒）
        if (w->last_review == 0) {// 如果单词的上次复习时间为0，说明是新单词，设置记忆稳定性为300秒
            w->stability = LEVEL_0_INTERVAL;// 300 秒
            return;
        }

        calibrate_params(w, is_correct, now);// 先校准单词的参数

        double elapsed = difftime(now, w->last_review);// 计算从上次复习到现在的时间差，单位为秒
        if (elapsed <= 0) elapsed = 1;// 如果时间差小于等于0，设置为1秒

        int total = w->correct_count + w->wrong_count;// 计算总记忆次数
        double historical_rate = (total == 0) ? 0 : (double)w->correct_count / total ;// 计算历史正确率，如果总记忆次数为0，设置为0

        if (is_correct) {// 如果用户回答正确
            double increase = elapsed / (w->stability + 1) * (double)g_vocab.gain;// 计算增加的记忆稳定性
            if (increase < 0.02) increase = 0.02;// 如果增加的记忆稳定性小于0.02，设置为0.02
            w->stability *= (float)(1.0 + increase);// 计算增加的记忆稳定性
            if (w->stability > LEVEL_7_INTERVAL) w->stability = LEVEL_7_INTERVAL;// 如果记忆稳定性大于7级，设置为7级
        } else {
            double decrease = (double)g_vocab.loss;
            if (elapsed < w->stability * 0.5) decrease *= 1.5;// 如果时间差小于记忆稳定性，设置减少的记忆稳定性为1.5倍
            if (historical_rate < 0.5) decrease *= 0.7;// 如果历史正确率小于0.5，设置减少的记忆稳定性为0.7倍
            if (decrease > 0.95) decrease = 0.95;// 防止stability过大
            w->stability *= (float)(1.0 - decrease);// 计算减少的记忆稳定性
            if (w->stability < LEVEL_0_INTERVAL) w->stability = LEVEL_0_INTERVAL;// 如果记忆稳定性小于0级，设置为0级
        }
    }
  
    void update_word_level(Word *word, int is_correct) {// 更新单词记忆等级和复习时间的函数，根据用户的测试结果调整单词的记忆等级，并计算下次复习时间
        if (word == NULL) return;

        time_t now = get_current_time();// 获取当前时间
        word->last_review = now;// 更新单词的上次复习时间为当前时间

        // 先更新等级与计数
        if (is_correct) {
            word->correct_count++;// 如果用户回答正确，增加正确记忆次数
            if (word->level < 7) word->level++;// 如果记忆等级小于7，增加等级
            word->is_mistake = 0;   // 标记单词为正确
        } else {
            word->wrong_count++;// 如果用户回答错误，增加错误记忆次数
            if (word->level > 0) {
                word->level--;// 如果记忆等级大于0，减少等级
            }
                word->is_mistake = 1;   // 标记单词为错词
        }
        
        // 再更新下次复习时间
        update_stability(word, is_correct, now);// 更新单词的记忆稳定性
        word->next_review = now + (time_t)word->stability;// 计算下次复习时间
    }
  
    void review_words() {// 复习待复习单词主函数(使用临时指针数组排序)
        clear_screen();
        // ======每日必须先添加单词才能复习 ======
        int today = get_today();
        if (g_vocab.last_add_date != today) {
            printf("今日还未添加新单词！\n");
            printf("请先录入至少1个单词，才能开始复习！\n");
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
            save_vocab();  // 实时保存进度

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

        // 更新连续复习天数
        today = get_today();
        if (reviewed > 0) {
            if (g_vocab.last_study_date == 0) {
                g_vocab.continuous_days = 1;
            } else {
                time_t last_t = date_to_time_t(g_vocab.last_study_date);
                time_t now_t = date_to_time_t(today);
                double diff_sec = difftime(now_t, last_t);
                int diff_days = (int)(diff_sec / (60 * 60 * 24) + 0.5); // 四舍五入计算天数差

                if (diff_days == 1) {
                    g_vocab.continuous_days++;
                } else if (diff_days >=2) {
                    g_vocab.continuous_days = 1;
                }
            }
            g_vocab.last_study_date = today;
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
                choice = getchar();
                while (getchar() != '\n');// 清除输入缓冲区
                choice = tolower(choice); // 转为小写

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

    double forgetting_probability(Word *w, time_t now) {
        if (w->last_review == 0) return 1.0;// 从未复习，肯定遗忘
        double elapsed = difftime(now, w->last_review);// 计算从上次复习到现在的时间差，单位为秒
        if (elapsed <= 0) return 1.0;// 如果时间差小于等于0，肯定遗忘

        // 计算回忆概率 R = exp(-elapsed / stability)
        double recall = exp(-elapsed / w->stability);
        double forgetting = 1.0 - recall;// 计算遗忘概率 = 1 - 回忆概率

        if (forgetting < 0) forgetting = 0;// 如果遗忘概率小于0，设置为0
        if (forgetting > 1) forgetting = 1;// 如果遗忘概率大于1，设置为1
        return forgetting;
    }
    int compare_word_ptr_by_forgetting(const void *a, const void *b) {//比较函数
        Word *wa = *(Word **)a;
        Word *wb = *(Word **)b;
        time_t now = get_current_time();// 获取当前时间
        double pa = forgetting_probability(wa, now);// 计算遗忘概率
        double pb = forgetting_probability(wb, now);// 计算遗忘概率
        // 根据遗忘概率和下次复习时间进行排序
        if (pa > pb) return -1;
        if (pa < pb) return 1;
        // 如果遗忘概率相同，根据下次复习时间进行排序
        if (wa->next_review < wb->next_review) return -1;
        if (wa->next_review > wb->next_review) return 1;
        return 0;
    }

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
            save_vocab();  // 实时保存进度

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

        // 更新连续学习天数
        int today = get_today();
        if (reviewed > 0) {
            if (g_vocab.last_study_date == 0) {
                g_vocab.continuous_days = 1;
            } else {
                time_t last_t = date_to_time_t(g_vocab.last_study_date);
                time_t now_t = date_to_time_t(today);
                double diff_sec = difftime(now_t, last_t);
                int diff_days = (int)(diff_sec / (60.0 * 60 * 24) + 0.5);
                if (diff_days == 1)
                    g_vocab.continuous_days++;
                else if (diff_days >= 2)
                    g_vocab.continuous_days = 1;
            }
            g_vocab.last_study_date = today;
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

    void trim(char *str) {// 去除字符串两端的空格的函数
        if (str == NULL || strlen(str) == 0) return;

        char *start = str;
        char *end = str + strlen(str) - 1;

        while (isspace((unsigned char)*start)) start++;

        if (start > end) {
            str[0] = '\0'; // 字符串全是空格，结果是空字符串
            return;
        }

        while (end > start && isspace((unsigned char)*end)) end--;
        memmove(str, start, end - start + 1);
        str[end - start + 1] = '\0';
    }

    int time_to_date(time_t t) {// 时间戳转换为日期（年月日）的函数，返回一个整数表示日期，格式为YYYYMMDD
        if (t == 0) return 0; // 如果时间戳为0，返回0表示未复习过
        struct tm *tm_info = localtime(&t);// 将时间戳转换为本地时间的结构体tm_info
        int year = tm_info->tm_year + 1900; // 获取年份，tm_year是从1900年开始的
        int month = tm_info->tm_mon + 1; // 获取月份，tm_mon是从0开始的
        int day = tm_info->tm_mday; // 获取日期
        return year * 10000 + month * 100 + day; // 将年月日组合成一个整数，格式为YYYYMMDD
    }

    int get_today() {// 获取今天的日期，返回一个整数表示日期，格式为YYYYMMDD
        return time_to_date(get_current_time()); // 获取当前时间的时间戳，并转换为日期格式
    }

    void encrypt_data(void *data, int len) {// 加密数据的函数，使用简单的异或加密方法对数据进行加密
        unsigned char *p = (unsigned char *)data;
        for (int i = 0; i < len; i++) {// 对数据中的每个字节进行异或操作
            p[i] ^= ENCRYPT_KEY;
        }
    }

    void decrypt_data(void *data, int len) {// 解密数据的函数，使用与加密相同的方法对数据进行解密
        // 加密和解密是对称的，直接调用加密函数即可
        encrypt_data(data, len);
    }

    int calculate_checksum(Vocab *vocab) {// 计算词库数据的校验和的函数，用于验证数据的完整性，返回一个整数表示校验和
        int sum = vocab->count + vocab->last_add_date; // 将单词数量和最后添加日期的整数值进行加法运算
        for (int i = 0; i < vocab->count; i++) {
            sum += vocab->words[i].id;
            sum += vocab->words[i].level;
            sum += (int)vocab->words[i].last_review;
            sum += (int)vocab->words[i].next_review;
            sum += vocab->words[i].correct_count;
            sum += vocab->words[i].wrong_count;
        }
        for (int j = 0; vocab->words[j].english[j] != '\0'; j++) {
            sum += vocab->words[j].english[j];
        }
        for (int k = 0; vocab->words[k].chinese[k] != '\0'; k++) {
            sum += vocab->words[k].chinese[k];
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
            if (strstr(find_data.cFileName, "_backup") != NULL) continue;
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

    void write_daily_log() {// 写入日志
        FILE *log = fopen("daily_review.log", "w");// 以写入模式打开日志文件，如果文件不存在会创建新文件
        if (!log) {
            printf("日志文件创建失败！\n");
            return;
        }

        for (int i = 0; i < g_vocab.daily_stats_count; i++) {
            DailyStat *stat = &g_vocab.daily_stats[i];
            double rate = (stat->total_count == 0) ? 0.0 : (double)stat->correct_count / stat->total_count * 100;
            fprintf(log, "%04d-%02d-%02d %d %d  %.2f%%\n", stat->date / 10000, stat->date % 10000 / 100, stat->date % 100, stat->correct_count, stat->total_count, rate); // 将日志写入文件中，格式为年月日：正确率%%%")
        }
        fclose(log); // 关闭日志文件
    }

    void show_statistics() {// 显示复习统计
        clear_screen();
        printf("\n=======复习统计=======\n");
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

    time_t get_current_time(void) {// 获取当前时间
    if (g_mock_time > 0) return g_mock_time;
    return time(NULL);
}

    void set_mock_time(void) {// 设置模拟时间
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
        while (getchar() != '\n');// 清除输入缓冲区中的换行符

        if (choice < 1 || choice > 5) {
            printf("无效的选择，将返回。\n");
            return;
        }

        // 如果没有设置过模拟时间,那么设置模拟时间为当前时间
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

    void set_daily_goal() {// 设置每日学习目标的函数
        printf("\n=====设置每日学习目标=====\n");
        printf("当前每日学习目标: %d\n", g_vocab.daily_goal);
        printf("请输入你的每日学习目标: ");
        int goal;
        if (scanf("%d", &goal) != 1) {
            while(getchar() != '\n');
            printf("无效的目标，将返回。\n");
            return;
        } 
        getchar();
        if (goal < 1) goal = 10;
        g_vocab.daily_goal = goal;
        save_vocab();
        printf("每日学习目标已设置为: %d\n", g_vocab.daily_goal);
        printf("\n按回车键返回主菜单...");
        getchar();
    }

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
            choice = getchar();
            while (getchar() != '\n');
            choice = tolower(choice);

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
        char c;
        scanf("%c", &c);
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

    time_t date_to_time_t(int date) {// 日期相邻判断时间戳转换
        struct tm tm = {0};
        tm.tm_year = date / 10000 - 1900;
        tm.tm_mon = (date % 10000) / 100 - 1;
        tm.tm_mday = date % 100;
        tm.tm_hour = 12;// 设置为中午12点
        return mktime(&tm);
    }

