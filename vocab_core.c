// vocab_core.c
#include "vocab_core.h"


Vocab g_vocab = {.count = 0};
time_t g_mock_time = 0;
char g_current_vocab_file[256] = "vocab.dat";
char g_current_backup_vocab_file[300] = "vocab_backup.dat";
TestMode current_test_mode = MODE_CN_TO_EN;

int g_mock_mode = 0;
char g_original_vocab_file[256] = "";