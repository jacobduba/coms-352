#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "encrypt-module.h"

#define CHAR_NULL '\0'

typedef enum { EMPTY, UNCOUNTED, COUNTED } b_status_t;
#define NUM_FLAGS 3

typedef struct {
  char data;
  b_status_t status;
  pthread_mutex_t lock;
  pthread_cond_t status_set_to[NUM_FLAGS];
} buffer_slot_t;

int input_buffer_size, output_buffer_size;

buffer_slot_t *input_buffer;
buffer_slot_t *output_buffer;
pthread_t input_thread;
pthread_t input_counter_thread;
pthread_t encrypt_thread;
pthread_t output_counter_thread;
pthread_t output_thread;
// Boolean val for if input counter should stop for reset
int ic_reset;
pthread_mutex_t ic_reset_lock;
pthread_cond_t ic_reset_changed;
// Lock for just the count_output_function
int oc_reset;
pthread_mutex_t oc_reset_lock;
pthread_cond_t oc_reset_changed;

// This function freezes the input_counter thread
// Then waits until get_input_total_count() == get_output_total_count()
void reset_requested() {
  printf("reset_request 1\n");
  pthread_mutex_lock(&ic_reset_lock);
  ic_reset = 1;
  pthread_mutex_unlock(&ic_reset_lock);

  printf("reset_request 2\n");
  pthread_mutex_lock(&oc_reset_lock);
  while (!oc_reset) {
    printf("reset_request 3, itc: %d otc %d\n", get_input_total_count(),
           get_output_total_count());
    pthread_cond_wait(&oc_reset_changed, &oc_reset_lock);
  }
  pthread_mutex_unlock(&oc_reset_lock);
  // input AND output are locked here --- no race condition.
  // while (get_input_total_count() != get_output_total_count()) {
  //   printf("Start cond wait itc: %d otc %d\n", get_input_total_count(),
  //          get_output_total_count());
  //   pthread_cond_wait(&count_output_changed, &oc_reset_lock);
  // }
  printf("reset_request 4\n");

  // log_counts();
}

void reset_finished() {
  printf("reset_finished 1\n");
  // printf("Reset finished start\n");
  pthread_mutex_lock(&ic_reset_lock);
  ic_reset = 0;
  pthread_mutex_unlock(&ic_reset_lock);
  pthread_cond_signal(&ic_reset_changed);
  // printf("Reset finished end (ic_reset: %d)\n", ic_reset);
  pthread_mutex_lock(&oc_reset_lock);
  oc_reset = 0;
  pthread_mutex_unlock(&oc_reset_lock);
  pthread_cond_signal(&oc_reset_changed);
  printf("reset_finished 2\n");
}

void *input_thread_fun() {
  buffer_slot_t *b;
  char c;
  int i;

  for (i = 0;; i = (i + 1) % input_buffer_size) {
    b = &input_buffer[i];
    c = read_input();

    pthread_mutex_lock(&b->lock);
    while (b->status != EMPTY)
      pthread_cond_wait(&b->status_set_to[EMPTY], &b->lock);
    b->data = c;
    // printf("Input thread: %c\n", c);
    b->status = UNCOUNTED;
    pthread_mutex_unlock(&b->lock);
    pthread_cond_signal(&b->status_set_to[UNCOUNTED]);

    if (c == EOF) return 0;
  }
}

void *input_counter_thread_fun() {
  buffer_slot_t *b;
  char c;
  int i;

  for (i = 0;; i = (i + 1) % input_buffer_size) {
    pthread_mutex_lock(&ic_reset_lock);
    while (ic_reset) {
      printf("input_counter 1 (ic_reset: %d)\n", ic_reset);
      pthread_cond_wait(&ic_reset_changed, &ic_reset_lock);
      printf("input_counter 2 (ic_reset: %d)\n", ic_reset);
    }
    pthread_mutex_unlock(&ic_reset_lock);
    b = &input_buffer[i];

    pthread_mutex_lock(&b->lock);
    while (b->status != UNCOUNTED)
      pthread_cond_wait(&b->status_set_to[UNCOUNTED], &b->lock);
    c = b->data;
    // printf("Input counter thread: %c\n", c);
    b->status = COUNTED;
    pthread_mutex_unlock(&b->lock);
    pthread_cond_signal(&b->status_set_to[COUNTED]);

    if (c == EOF) return 0;

    count_input(c);
  }
}

void *encrypt_thread_fun() {
  buffer_slot_t *ib, *ob;
  char c;
  int i, j;

  i = 0, j = 0;
  for (;;) {
    ib = &input_buffer[i];

    pthread_mutex_lock(&ib->lock);
    while (ib->status != COUNTED)
      pthread_cond_wait(&ib->status_set_to[COUNTED], &ib->lock);
    c = ib->data;
    ib->status = EMPTY;
    pthread_mutex_unlock(&ib->lock);
    pthread_cond_signal(&ib->status_set_to[EMPTY]);

    i = (i + 1) % input_buffer_size;

    // printf("Encrypt thread: %c\n", c);
    if (c != EOF) c = encrypt(c);

    ob = &output_buffer[j];

    pthread_mutex_lock(&ob->lock);
    while (ob->status != EMPTY)
      pthread_cond_wait(&ob->status_set_to[EMPTY], &ob->lock);
    ob->data = c;
    ob->status = UNCOUNTED;
    pthread_mutex_unlock(&ob->lock);
    pthread_cond_signal(&ob->status_set_to[UNCOUNTED]);

    j = (j + 1) % output_buffer_size;

    if (c == EOF) return 0;
  }
}

void *output_counter_thread_fun() {
  buffer_slot_t *b;
  char c;
  int i;

  for (i = 0;; i = (i + 1) % output_buffer_size) {
    pthread_mutex_lock(&ic_reset_lock);
    if (ic_reset && get_input_total_count() == get_output_total_count()) {
      printf("output_counter 1\n");
      pthread_mutex_lock(&oc_reset_lock);
      printf("output_counter 2\n");
      oc_reset = 1;
      pthread_mutex_unlock(&oc_reset_lock);
      pthread_cond_signal(&oc_reset_changed);
    }
    pthread_mutex_unlock(&ic_reset_lock);

    pthread_mutex_lock(&oc_reset_lock);
    while (oc_reset) {
      pthread_cond_wait(&oc_reset_changed, &oc_reset_lock);
    }
    pthread_mutex_unlock(&oc_reset_lock);

    b = &output_buffer[i];

    pthread_mutex_lock(&b->lock);
    while (b->status != UNCOUNTED)
      pthread_cond_wait(&b->status_set_to[UNCOUNTED], &b->lock);
    c = b->data;
    // printf("Output counter thread: %c\n", c);
    b->status = COUNTED;
    pthread_mutex_unlock(&b->lock);
    pthread_cond_signal(&b->status_set_to[COUNTED]);

    if (c == EOF) return 0;

    count_output(c);
  }
}

void *output_thread_fun() {
  buffer_slot_t *b;
  char c;
  int i;

  for (i = 0;; i = (i + 1) % output_buffer_size) {
    b = &output_buffer[i];

    pthread_mutex_lock(&b->lock);
    while (b->status != COUNTED)
      pthread_cond_wait(&b->status_set_to[COUNTED], &b->lock);
    c = b->data;
    // printf("Output thread: %c\n", c);
    b->status = EMPTY;
    pthread_mutex_unlock(&b->lock);
    pthread_cond_signal(&b->status_set_to[EMPTY]);

    if (c == EOF) return 0;

    write_output(c);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf(
        "Error: incorrect number of params.\nUsage: ./encrypt <input> "
        "<output> "
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

  if ((input_buffer = malloc(input_buffer_size * sizeof(buffer_slot_t))) ==
          NULL ||
      (output_buffer = malloc(output_buffer_size * sizeof(buffer_slot_t))) ==
          NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }

  for (int i = 0; i < input_buffer_size; i++) {
    input_buffer[i].data = CHAR_NULL;
    input_buffer[i].status = EMPTY;
    pthread_mutex_init(&input_buffer[i].lock, NULL);
    for (int i = 0; i < NUM_FLAGS; i++)
      pthread_cond_init(&input_buffer[i].status_set_to[i], NULL);
  }
  for (int i = 0; i < output_buffer_size; i++) {
    output_buffer[i].data = CHAR_NULL;
    output_buffer[i].status = EMPTY;
    pthread_mutex_init(&output_buffer[i].lock, NULL);
    for (int i = 0; i < NUM_FLAGS; i++)
      pthread_cond_init(&input_buffer[i].status_set_to[i], NULL);
  }

  ic_reset = 0;
  pthread_mutex_init(&ic_reset_lock, NULL);
  pthread_cond_init(&ic_reset_changed, NULL);
  oc_reset = 0;
  pthread_mutex_init(&oc_reset_lock, NULL);
  pthread_cond_init(&oc_reset_changed, NULL);

  init(argv[1], argv[2], argv[3]);

  pthread_create(&input_thread, NULL, input_thread_fun, NULL);
  pthread_create(&input_counter_thread, NULL, input_counter_thread_fun, NULL);
  pthread_create(&encrypt_thread, NULL, encrypt_thread_fun, NULL);
  pthread_create(&output_counter_thread, NULL, output_counter_thread_fun, NULL);
  pthread_create(&output_thread, NULL, output_thread_fun, NULL);

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
