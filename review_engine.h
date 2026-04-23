#ifndef REVIEW_ENGINE_H
#define REVIEW_ENGINE_H

#include "vocab_core.h"

int get_need_review_count(void);
void review_words(void);
void review_mistakes(void);
int quiz_word(Word *word);
int quiz_cn_to_en(Word *word);
int quiz_en_to_cn(Word *word);
void select_test_mode(void);
void show_review_rank(void);

#endif