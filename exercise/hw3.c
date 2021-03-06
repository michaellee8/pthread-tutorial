#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>

typedef struct _context_t {
  uint64_t id;
  char *name;
  bool initialized;
} context_t;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

_Atomic(context_t*) ctx = NULL;

// singleton
context_t *get_instance() {
  atomic_thread_fence(memory_order_acquire);
  if (ctx == NULL) {
    pthread_mutex_lock(&mut);
    if (ctx == NULL){
      ctx = (context_t *)malloc(sizeof(context_t)); 
      ctx->initialized = false;
      atomic_thread_fence(memory_order_release);
    }
    pthread_mutex_unlock(&mut);
  }
  return ctx;
}



int id = 0;

void *do_work(void *arg) {
  context_t *ctx = get_instance();
  if (!ctx->initialized) {
    ctx->name = (char *)arg;
    ctx->id = ++id;
    ctx->initialized = true;
  }
  printf("name=%s\tid=%ld\n", ctx->name, ctx->id);
  return NULL;
}

int main(int argc, char *argv[]) {
  int rc;
  pthread_t p1, p2;

  rc = pthread_create(&p1, NULL, do_work, "A");
  assert(rc == 0);
  rc = pthread_create(&p2, NULL, do_work, "B");
  assert(rc == 0);

  rc = pthread_join(p1, NULL);
  assert(rc == 0);
  rc = pthread_join(p2, NULL);
  assert(rc == 0);

  if (ctx) {
    free(ctx);
  }
  return 0;
}
