// vocab_core.h
#ifndef VOCAB_CORE_H
#define VOCAB_CORE_H

#include <time.h>

#define MAX_WORD 2000
#define MAX_STR 1000

#define LEVEL_0_INTERVAL 300
#define LEVEL_1_INTERVAL 1800
#define LEVEL_2_INTERVAL 3600
#define LEVEL_3_INTERVAL 43200
#define LEVEL_4_INTERVAL 86400
#define LEVEL_5_INTERVAL 172800
#define LEVEL_6_INTERVAL 604800
#define LEVEL_7_INTERVAL 2592000
#define MAX_LEVEL 7

#define ENCRYPT_KEY 0x7B

#define COLOR_DEFAULT  7
#define COLOR_MENU_TEXT 7
#define COLOR_TITLE    11
#define COLOR_SUCCESS  10
#define COLOR_WARN     14
#define COLOR_ERROR    12

typedef struct {
    int date;
    int total_count;
    int correct_count;
} DailyStat;

typedef struct {
    int id;
    char english[MAX_STR];
    char chinese[MAX_STR];
    int level;
    float stability;
    time_t last_review;
    time_t next_review;
    int correct_count;
    int wrong_count;
    int is_mistake;
} Word;

typedef struct {
    Word words[MAX_WORD];
    int count;
    int last_add_date;
    int checksum;
    DailyStat daily_stats[30];
    int daily_stats_count;
    float gain;
    float loss;
    int total_review;
    int continuous_days;
    int last_study_date;
    int daily_goal;
} Vocab;

typedef enum {
    MODE_CN_TO_EN,
    MODE_EN_TO_CN
} TestMode;

extern Vocab g_vocab;
extern time_t g_mock_time;
extern char g_current_vocab_file[256];
extern char g_current_backup_vocab_file[300];
extern TestMode current_test_mode;

#endif