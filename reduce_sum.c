#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <time.h>

typedef struct _arg_t {
  float *arr;
  size_t len;
  cpu_set_t *cpuset;
} arg_t;

typedef struct _ret_t {
  float sum;
} ret_t;

void init_array(float *p, size_t len) {
  for (int i = 0; i < len; ++i) {
    p[i] = (float)rand() / RAND_MAX;
  }
}

float *gen_array(size_t len) {
  float *p = (float *)malloc(len * sizeof(float));
  if (!p) {
    printf("failed to allocate %ld bytes memory!", len * sizeof(float));
    exit(1);
  }
  return p;
}

void *reduce_sum(void *args) {
  arg_t *vec = (arg_t *)args;

  if (vec->cpuset) {
    int rc;
    rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), vec->cpuset);
    assert(rc == 0);
  }

  size_t len = vec->len;
  double sum = 0.0f;
  for (int i = 0; i < len; ++i) {
    sum += vec->arr[i];
  }

  ret_t *ret = (ret_t *)malloc(sizeof(ret_t));
  ret->sum = sum;
  return ret;
}

void run(float *arr, int len, arg_t *args, int k) {
  pthread_t ph[k];
  int rc;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  for (int i = 0; i < k; ++i) {
    rc = pthread_create(&ph[i], NULL, reduce_sum, (void *)&args[i]);
    assert(rc == 0);
  }

  ret_t *rets[k];
  for (int i = 0; i < k; ++i) {
    rc = pthread_join(ph[i], (void **)&rets[i]);
    assert(rc == 0);
  }

  double sum = 0.0;
  for (int i = 0; i < k; ++i) {
    sum += rets[i]->sum;
  }

  printf("final sum=%.6f\n", sum);

  for (int i = 0; i < k; ++i) {
    free(rets[i]);
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);

  uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1.0e6 +
                      (end.tv_nsec - start.tv_nsec) * 1.0e-3;
  printf("[Merge sort]The elapsed time is %.2f ms.\n", delta_us / 1000.0);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Error: The number of input integers now is %d. Please input 3 "
           "integers.\n",
           argc - 1);
    exit(1);
  }
  const int num = atoi(argv[1]);
  const int k = atoi(argv[2]);

  srand(time(NULL));

  float *arr = gen_array(num);
  init_array(arr, num);

  int chunk_size = num / k;
  int r = num % k;
  arg_t args[k];
  int lo = 0, hi = 0;
  for (int i = 0; i < k; ++i) {
    lo = hi;
    hi += chunk_size;
    args[i] = (arg_t){arr + lo, hi - lo, NULL};
  }
  args[k - 1].len += r;

  // without setting affinity
  run(arr, num, args, k);

  printf("Set affinity mask to include CPUs (1, 3, 5, ...  2n+1)\n");
  for (int i = 0, j = 1; j < get_nprocs() && i < k; j += 2, i++) {
    cpu_set_t *cpuset = (cpu_set_t *)malloc(sizeof(cpu_set_t));
    CPU_SET(j, cpuset);
    args[i].cpuset = cpuset;
  }

  run(arr, num, args, k);

  for (int i = 0; i < k; ++i) {
    free(args[i].cpuset);
  }

  return 0;
}