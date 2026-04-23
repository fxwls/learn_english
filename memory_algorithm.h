#ifndef MEMORY_ALGORITHM_H
#define MEMORY_ALGORITHM_H

#include "vocab_core.h"

void update_word_level(Word *word, int is_correct);
void update_stability(Word *w, int is_correct, time_t now);
void calibrate_params(Word *w, int is_correct, time_t now);
double forgetting_probability(Word *w, time_t now);
int compare_word_ptr_by_forgetting(const void *a, const void *b);

#endif