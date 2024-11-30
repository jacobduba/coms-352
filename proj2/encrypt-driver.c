#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "encrypt-module.h"

#define CHAR_NULL '\0'

typedef enum { EMPTY, UNCOUNTED, COUNTED } flag_t;

typedef struct {
  char data;
  flag_t flag;
  pthread_mutex_t lock;
  pthread_cond_t unlocked;
} buffer_slot;

int input_buffer_size, output_buffer_size;

buffer_slot *input_buffer;
buffer_slot *output_buffer;
pthread_t input_thread;
pthread_t input_counter_thread;
pthread_t encrypt_thread;
pthread_t output_counter_thread;
pthread_t output_thread;
// All threads wait when they run out of data
// Wait until these signals to try again
pthread_cond_t input_thread_finished;
pthread_cond_t input_counter_thread_finished;
pthread_cond_t encrypt_thread_finished;
pthread_cond_t output_counter_thread_finished;
pthread_cond_t output_thread_finished;

void reset_requested() { log_counts(); }

void reset_finished() {}

void *input_thread_fun() {
  char c;
  int i;

  for (i = 0;; i = (i + 1) % input_buffer_size) {
    c = read_input();

    sem_wait(&input_buffer[i].lock);
    // if (input_buffer[i])

    if (c == EOF) return;
  }
}

void *input_counter_thread_fun() {}

void *encrypt_thread_fun() {}

void *output_counter_thread_fun() {}

void *output_thread_fun() {}

int main(int argc, char *argv[]) {
  char c;

  if (argc != 4) {
    printf(
        "Error: incorrect number of params.\nUsage: ./encrypt <input> <output> "
        "<log>\n");
    return 1;
  }

  printf("Enter input buffer size: ");
  scanf("%d", &input_buffer_size);

  printf("Enter output buffer size: ");
  scanf("%d", &output_buffer_size);

  if (input_buffer_size < 2 || output_buffer_size < 2) {
    printf("Error: input and output buffers must be >1.\n");
    return 1;
  }

  if ((input_buffer = malloc(input_buffer_size * sizeof(buffer_slot))) ==
          NULL ||
      (output_buffer = malloc(output_buffer_size * sizeof(buffer_slot))) ==
          NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }

  for (int i = 0; i < input_buffer_size; i++) {
    input_buffer[i].data = CHAR_NULL;
    input_buffer[i].flag = EMPTY;
    sem_init(&input_buffer[i].lock, 0, 1);
  }
  for (int i = 0; i < output_buffer_size; i++) {
    output_buffer[i].data = CHAR_NULL;
    output_buffer[i].flag = EMPTY;
    sem_init(&output_buffer[i].lock, 0, 1);
  }

  init(argv[1], argv[2], argv[3]);

  pthread_cond_init(&input_thread_finished, NULL);
  pthread_cond_init(&input_counter_thread_finished, NULL);
  pthread_cond_init(&encrypt_thread_finished, NULL);
  pthread_cond_init(&output_counter_thread_finished, NULL);
  pthread_cond_init(&output_thread_finished, NULL);

  pthread_create(&input_thread, NULL, input_thread_fun, NULL);
  pthread_create(&input_counter_thread, NULL, input_counter_thread_fun, NULL);
  pthread_create(&encrypt_thread, NULL, encrypt_thread_fun, NULL);
  pthread_create(&output_counter_thread, NULL, output_counter_thread_fun, NULL);
  pthread_create(&output_thread, NULL, output_counter_thread_fun, NULL);

  pthread_join(input_thread, NULL);
  pthread_join(input_counter_thread, NULL);
  pthread_join(encrypt_thread, NULL);
  pthread_join(output_counter_thread, NULL);
  pthread_join(output_thread, NULL);

  printf("End of file reached.\n");

  log_counts();

  free(input_buffer);
  free(output_buffer);

  return 0;
}
