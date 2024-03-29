/* Quick Hash Table
 *
 * A lock-free hash table implementation using atomics and sequence
 * locks.
 */

#include "qht.h"

#include <stdlib.h>
#include <string.h>

void qht_free(qht *table)
{
  for (uint32_t i = 0; i < table->num_buckets; i++) {
    qht_bucket *p = table->buckets[i]->next;
    while (p != NULL) {
      qht_bucket *tmp = p->next;
      free(p);
      p = tmp;
    }
    free(table->buckets[i]);
  }

  free(table->buckets);
  free(table);
}

qht *qht_init(uint32_t nb)
{
  qht *table = (qht *)malloc(sizeof(qht));
  memset(table, 0, sizeof(qht));
  table->buckets = (qht_bucket **)malloc(nb * sizeof(qht_bucket *));
  table->num_buckets = nb;

  for (uint32_t i = 0; i < nb; i++) {
    table->buckets[i] = (qht_bucket *)malloc(sizeof(qht_bucket));
    memset(table->buckets[i], 0, sizeof(qht_bucket));
    pthread_mutex_init(&table->buckets[i]->lock, NULL);
    table->buckets[i]->next = NULL;
  }

  return table;
}

void *qht_search(qht_bucket *bucket, uint32_t key, int num_entries)
{
  qht_bucket *curr_bucket = bucket;

  while (curr_bucket) {
    STAT_INC(bucket_lookup_counter);
    for (int i = 0; i < num_entries; i++) {
      STAT_INC(comp_counter);
      if (curr_bucket->keys[i] == key)
	return curr_bucket->values[i];
    }
    curr_bucket = curr_bucket->next;
  }

  /* key not found */
  return NULL;
}

uint32_t map_key_to_index(uint32_t key, uint32_t n)
{
  return key & (n - 1);
}

void *qht_lookup(qht *table, uint32_t key)
{
  uint32_t index = map_key_to_index(key, table->num_buckets);
  unsigned version;
  void *result = NULL;
  
  do {
    version = seqlock_read_begin(&table->buckets[index]->seqlock);
    result = qht_search(table->buckets[index], key, PER_BUCKET);
  } while (seqlock_read_end(&table->buckets[index]->seqlock, version));
  
  return result;
}

bool insert_in_bucket(qht_bucket *bucket, uint32_t key, void *ptr)
{
  qht_bucket *curr_bucket = bucket, *prev_bucket;
  while (curr_bucket) {
    STAT_INC(bucket_insert_counter);
    for (int i = 0; i < PER_BUCKET; i++) {
      STAT_INC(comp_counter);
      if (curr_bucket->values[i] == ptr)
	return false;
    
      if (curr_bucket->values[i] == NULL) {
	curr_bucket->values[i] = ptr;
	curr_bucket->keys[i] = key;
	return true;
      }
    }
    prev_bucket = curr_bucket;
    curr_bucket = curr_bucket->next;
  }
  
  // insertion failed
  prev_bucket->next = (qht_bucket *)malloc(sizeof(qht_bucket));
  memset(prev_bucket->next, 0, sizeof(qht_bucket));
  pthread_mutex_init(&prev_bucket->next->lock, NULL);
  prev_bucket->next->keys[0] = key;
  prev_bucket->next->values[0] = ptr;
  prev_bucket->next->next = NULL;

  return true;
}

bool qht_insert(qht *table, uint32_t key, void *value)
{
  uint32_t index = map_key_to_index(key, table->num_buckets);
  
  // disallow other writers
  pthread_mutex_lock(&table->buckets[index]->lock);
  // inform readers
  seqlock_write_begin(&table->buckets[index]->seqlock);
  bool success = insert_in_bucket(table->buckets[index], key, value);
  seqlock_write_end(&table->buckets[index]->seqlock);
  pthread_mutex_unlock(&table->buckets[index]->lock);

  return success;
}

bool delete_from_bucket(qht_bucket *bucket, uint32_t key)
{
  qht_bucket *curr_bucket = bucket, *prev_bucket;
  while (curr_bucket) {
    STAT_INC(bucket_lookup_counter);
    for (int i = 0; i < PER_BUCKET; i++) {
      STAT_INC(comp_counter);
      if (bucket->keys[i] == key) {
	bucket->keys[i] = 0;
	bucket->values[i] = NULL;
	return true;
      }
    }
    prev_bucket = curr_bucket;
    curr_bucket = curr_bucket->next;
  }
 
  return false;
}

bool qht_delete(qht *table, uint32_t key)
{
  int index = map_key_to_index(key, table->num_buckets);
  
  pthread_mutex_lock(&table->buckets[index]->lock);
  seqlock_write_begin(&table->buckets[index]->seqlock);
  bool success = delete_from_bucket(table->buckets[index], key);
  seqlock_write_end(&table->buckets[index]->seqlock);
  pthread_mutex_unlock(&table->buckets[index]->lock);
  
  return success;
}

void qht_print_stats(int threads, int ENTRIES_PER_THREAD)
{
  #if STATS
  printf("Comparisons: %lu\n", comp_counter);
  printf("Buckets touched: %lu\n", bucket_lookup_counter+ \
                                   bucket_insert_counter+ \
                                   bucket_delete_counter);
  printf("Operations: %d\n", ENTRIES_PER_THREAD * threads);
  #endif
}


