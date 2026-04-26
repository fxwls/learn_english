// file_io.h
#ifndef FILE_IO_H
#define FILE_IO_H

#include "vocab_core.h"

void save_vocab(void);
void load_vocab(void);
void backup_vocab(void);
void restore_vocab(void);
void create_new_vocab(void);
void switch_vocab(void);
void generate_backup_filename(const char *vocab_file, char *backup_file);
int is_file_same(void);
void encrypt_data(void *data, int len);
void decrypt_data(void *data, int len);
int calculate_checksum(Vocab *vocab);
void sanitize_filename(char *name);
const char* get_basename(const char *path);

#endif