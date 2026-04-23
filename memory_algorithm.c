#include "memory_algorithm.h"
#include "time_utils.h"
#include <math.h>

void update_word_level(Word *word, int is_correct) {
    if (is_correct) {
        if (word->level < MAX_LEVEL) word->level++;
    } else {
        word->level = 0;
    }
}

void update_stability(Word *w, int is_correct, time_t now) {
    double interval = difftime(now, w->last_review);
    if (is_correct) {
        w->stability = w->stability * 1.5 + interval / 86400.0;
    } else {
        w->stability *= 0.4;
    }
    if (w->stability < 0.1) w->stability = 0.1;
}

void calibrate_params(Word *w, int is_correct, time_t now) {
    update_stability(w, is_correct, now);
    update_word_level(w, is_correct);
    w->last_review = now;
    double next_interval;
    if (w->level == 0) next_interval = LEVEL_0_INTERVAL;
    else if (w->level == 1) next_interval = LEVEL_1_INTERVAL;
    else if (w->level == 2) next_interval = LEVEL_2_INTERVAL;
    else if (w->level == 3) next_interval = LEVEL_3_INTERVAL;
    else if (w->level == 4) next_interval = LEVEL_4_INTERVAL;
    else if (w->level == 5) next_interval = LEVEL_5_INTERVAL;
    else if (w->level == 6) next_interval = LEVEL_6_INTERVAL;
    else next_interval = LEVEL_7_INTERVAL;
    w->next_review = now + next_interval;
}

double forgetting_probability(Word *w, time_t now) {
    double t = difftime(now, w->last_review);
    return exp(-t / w->stability);
}

int compare_word_ptr_by_forgetting(const void *a, const void *b) {
    Word *wa = *(Word**)a;
    Word *wb = *(Word**)b;
    time_t now = get_current_time();
    double fa = forgetting_probability(wa, now);
    double fb = forgetting_probability(wb, now);
    if (fa > fb) return -1;
    if (fa < fb) return 1;
    return 0;
}