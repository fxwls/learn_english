#ifndef WORD_MANAGER_H
#define WORD_MANAGER_H

#include "vocab_core.h"

void add_word(void);
void edit_word(void);
void delete_word(void);
void search_word(void);
void browse_all_words(void);
int is_duplicate(const char *english);
int is_valid_english(const char *word);
void show_word_detail(Word *word);

#endif