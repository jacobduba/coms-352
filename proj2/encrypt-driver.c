#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "encrypt-module.h"

typedef enum { EMPTY, UNCOUNTED, COUNTED } flag_t;

typedef struct {
  char data;
  flag_t flag;
  sem_t lock;
} buffer_slot;

buffer_slot *input_buffer;
buffer_slot *output_buffer;
pthread_t input_thread;
pthread_t input_counter_thread;
pthread_t encrypt_thread;
pthread_t output_counter_thread;
pthread_t output_thread;
// All threads wait when they run out of data
// Wait until these signals to try again
pthread_cond_t signal_input_thread;
pthread_cond_t signal_input_counter_thread;
pthread_cond_t signal_encrypt_thread;
pthread_cond_t signal_output_counter_thread;
pthread_cond_t signal_output_thread;

void reset_requested() { log_counts(); }

void reset_finished() {}

void *input_thread_fun() {}

void *input_counter_thread_fun() {}

void *encrypt_thread_fun() {}

void *output_counter_thread_fun() {}

void *output_thread_fun() {}

int main(int argc, char *argv[]) {
  int input_buffer_size, output_buffer_size;
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
    input_buffer[i].data = NULL;
    input_buffer[i].flag = EMPTY;
    sem_init(&input_buffer[i].lock, 0, 1);
  }
  for (int i = 0; i < output_buffer_size; i++) {
    output_buffer[i].data = NULL;
    output_buffer[i].flag = EMPTY;
    sem_init(&output_buffer[i].lock, 0, 1);
  }

  init(argv[1], argv[2], argv[3]);

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

  while ((c = read_input()) != EOF) {
    count_input(c);
    c = encrypt(c);
    count_output(c);
    write_output(c);
  }

  printf("End of file reached.\n");

  log_counts();

  free(input_buffer);
  free(output_buffer);

  return 0;
}
