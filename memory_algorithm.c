// memory_algorithm.c
#include "memory_algorithm.h"
#include "time_utils.h"
#include "vocab_core.h"
#include <math.h>

void calibrate_params(Word *w, int is_correct, time_t now) {
    if (w->last_review == 0) return;
    double elapsed = difftime(now, w->last_review);
    if (elapsed <= 0) return;
    double R = exp(-elapsed / w->stability);
    double predicted = R;
    double actual = is_correct ? 1.0 : 0.0;
    double error = actual - predicted;
    double lr = 0.05 / (1.0 + g_vocab.total_review / 500.0);
    if (lr < 0.002) lr = 0.002;
    float raw_gain = g_vocab.gain;
    float raw_loss = g_vocab.loss;
    if (is_correct) raw_gain += (float)(lr * error);
    else raw_loss -= (float)(lr * error);
    if (raw_gain < 0.02f) raw_gain = 0.02f;
    if (raw_gain > 0.50f) raw_gain = 0.50f;
    if (raw_loss < 0.05f) raw_loss = 0.05f;
    if (raw_loss > 0.80f) raw_loss = 0.80f;
    float alpha = 0.3f;
    g_vocab.gain = alpha * raw_gain + (1.0f - alpha) * g_vocab.gain;
    g_vocab.loss = alpha * raw_loss + (1.0f - alpha) * g_vocab.loss;
}

void update_stability(Word *w, int is_correct, time_t now) {
    if (w->last_review == 0) {
        w->stability = LEVEL_0_INTERVAL;
        return;
    }
    calibrate_params(w, is_correct, now);
    double elapsed = difftime(now, w->last_review);
    if (elapsed <= 0) elapsed = 1;
    int total = w->correct_count + w->wrong_count;
    double historical_rate = (total == 0) ? 0 : (double)w->correct_count / total;
    if (is_correct) {
        double increase = elapsed / (w->stability + 1) * (double)g_vocab.gain;
        if (increase > 1.0) increase = 1.0;
        if (increase < 0.02) increase = 0.02;
        w->stability *= (float)(1.0 + increase);
        if (w->stability > LEVEL_7_INTERVAL) w->stability = LEVEL_7_INTERVAL;
    } else {
        double decrease = (double)g_vocab.loss;
        if (elapsed < w->stability * 0.5) decrease *= 1.5;
        if (historical_rate < 0.5) decrease *= 0.7;
        if (decrease > 0.95) decrease = 0.95;
        w->stability *= (float)(1.0 - decrease);
        if (w->stability < LEVEL_0_INTERVAL) w->stability = LEVEL_0_INTERVAL;
    }
}

void update_word_level(Word *word, int is_correct) {
    if (word == NULL) return;
    time_t now = get_current_time();
    word->last_review = now;
    if (is_correct) {
        word->correct_count++;
        if (word->level < MAX_LEVEL) word->level++;
        word->is_mistake = 0;
    } else {
        word->wrong_count++;
        if (word->level > 0) word->level--;
        word->is_mistake = 1;
    }
    update_stability(word, is_correct, now);
    word->next_review = now + (time_t)word->stability;
}

double forgetting_probability(Word *w, time_t now) {
    if (w->last_review == 0) return 1.0;
    double elapsed = difftime(now, w->last_review);
    if (elapsed <= 0) return 1.0;
    double ratio = elapsed / w->stability;
    if (ratio > 30.0) return 1.0;
    double recall = exp(-ratio);
    double forgetting = 1.0 - recall;
    if (forgetting < 0) forgetting = 0;
    if (forgetting > 1) forgetting = 1;
    return forgetting;
}

int compare_word_ptr_by_forgetting(const void *a, const void *b) {
    Word *wa = *(Word **)a;
    Word *wb = *(Word **)b;
    time_t now = get_current_time();
    double pa = forgetting_probability(wa, now);
    double pb = forgetting_probability(wb, now);
    if (pa > pb) return -1;
    if (pa < pb) return 1;
    if (wa->next_review < wb->next_review) return -1;
    if (wa->next_review > wb->next_review) return 1;
    return 0;
}