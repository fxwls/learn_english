#ifndef UI_UTILS_H
#define UI_UTILS_H

void set_color(int color);
void clear_screen();
void trim_newline(char *s);
int safe_input(char *buf, int size);
void trim(char *str);
int strcasecmp_custom(const char *a, const char *b);

#endif