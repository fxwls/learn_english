#include "ui_utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void clear_screen() {
    system("cls");
}

void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n')
        s[len-1] = '\0';
}

int safe_input(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL)
        return 0;
    trim_newline(buf);
    trim(buf);
    if (strlen(buf) == 0)
        return 0;
    return 1;
}

void trim(char *str) {
    if (!str || !*str) return;
    char *start = str;
    char *end = str + strlen(str) - 1;
    while (isspace((unsigned char)*start)) start++;
    if (start > end) { str[0] = 0; return; }
    while (end > start && isspace((unsigned char)*end)) end--;
    memmove(str, start, end - start + 1);
    str[end - start + 1] = 0;
}

int strcasecmp_custom(const char *a, const char *b) {
    while (*a && *b) {
        unsigned char ca = tolower((unsigned char)*a);
        unsigned char cb = tolower((unsigned char)*b);
        if (ca != cb)
            return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}