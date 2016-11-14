#ifndef __QHT__
#define __QHT__

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#include "seqlock.h"

#define PER_BUCKET 6
#define STATS 1

#if STATS
#define STAT_INC(x) __atomic_add_fetch(&x, 1, __ATOMIC_SEQ_CST);
#else
#define STAT_INC(x)
#endif

typedef struct qht_bucket {
  pthread_mutex_t   lock;
  seqlock_t         seqlock;
  uint32_t          keys[PER_BUCKET];
  void              *values[PER_BUCKET];
  struct qht_bucket *next;
} qht_bucket;

typedef struct qht {
  qht_bucket **buckets;
  uint32_t num_buckets;
} qht;

#ifdef __cplusplus
extern "C" {
#endif

uint64_t comp_counter = 0;
uint64_t bucket_lookup_counter = 0;
uint64_t bucket_insert_counter = 0;
uint64_t bucket_delete_counter = 0;

qht *qht_init(uint32_t nb);
void *qht_lookup(qht *table, uint32_t hash);
bool qht_insert(qht *table, uint32_t key, void *value);
bool qht_delete(qht *table, uint32_t hash);

#ifdef __cplusplus
}
#endif

#endif
