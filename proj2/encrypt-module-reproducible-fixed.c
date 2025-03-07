#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "encrypt-module.h"

FILE *input_file;
FILE *output_file;
FILE *log_file;
int input_counts[256];
int output_counts[256];
int input_total_count;
int output_total_count;
int key = 1;
int read_count = 0;
sem_t *sem_char_read;

void clear_counts() {
  memset(input_counts, 0, sizeof(input_counts));
  memset(output_counts, 0, sizeof(output_counts));
  input_total_count = 0;
  output_total_count = 0;
}

void *not_random_reset() {
  while (1) {
    sem_wait(sem_char_read);
    read_count++;
    if (read_count == 200) {
      reset_requested();
      key++;
      clear_counts();
      reset_finished();
      read_count = 0;
    }
  }
}

void init(char *inputFileName, char *outputFileName, char *logFileName) {
  pthread_t pid;
  sem_char_read = sem_open("/sem_char_read", O_CREAT, 0644, 0);
  sem_unlink("/sem_char_read");
  pthread_create(&pid, NULL, &not_random_reset, NULL);
  input_file = fopen(inputFileName, "r");
  output_file = fopen(outputFileName, "w");
  log_file = fopen(logFileName, "w");
}

void log_counts() {
  fprintf(log_file, "Counts using key %d:\n", key);
  fprintf(log_file, "Total input count: %d\n", input_total_count);
  fprintf(log_file, "Plaintext frequency counts: [ %d", input_counts[0]);
  for (int i = 1; i < 256; i++) {
    fprintf(log_file, ", %d", input_counts[i]);
  }
  fprintf(log_file, "]\n");
  fprintf(log_file, "Total output count: %d\n", output_total_count);
  fprintf(log_file, "Ciphertext frequency counts: [ %d", output_counts[0]);
  for (int i = 1; i < 256; i++) {
    fprintf(log_file, ", %d", output_counts[i]);
  }
  fprintf(log_file, "]\n\n");
}

int read_input() {
  sem_post(sem_char_read);
  // usleep(10000);
  return fgetc(input_file);
}

void write_output(int c) { fputc(c, output_file); }

int encrypt(int c) {
  if (c >= 'a' && c <= 'z') {
    c += key;
    if (c > 'z') {
      c = c - 'z' + 'a' - 1;
    }
  } else if (c >= 'A' && c <= 'Z') {
    c += key;
    if (c > 'Z') {
      c = c - 'Z' + 'A' - 1;
    }
  }
  return c;
}

void count_input(int c) {
  input_counts[toupper(c)]++;
  input_total_count++;
}

void count_output(int c) {
  output_counts[toupper(c)]++;
  output_total_count++;
}

int get_input_count(int c) { return input_counts[toupper(c)]; }

int get_output_count(int c) { return output_counts[toupper(c)]; }

int get_input_total_count() { return input_total_count; }

int get_output_total_count() { return output_total_count; }
