#include "qht.h"

#include <stdlib.h>

qht *qht_init(uint32_t nb)
{
  qht *table = (qht *)malloc(sizeof(qht));
  table->buckets = (qht_bucket *)malloc(nb * sizeof(qht_bucket));
  table->num_buckets = nb;

  return table;
}

void *qht_search(qht_bucket *bucket, uint32_t key, int num_entries)
{
  for (int i = 0; i < num_entries; i++) {
    if (bucket->keys[i] == key)
      return bucket->values[i];
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
    version = seqlock_read_begin(&table->buckets[index].seqlock);
    result = qht_search(&table->buckets[index], key, PER_BUCKET);
  } while (seqlock_read_end(&table->buckets[index].seqlock, version));
  
  return result;
}

bool insert_in_bucket(qht_bucket *bucket, uint32_t key, void *ptr)
{
  for (int i = 0; i < PER_BUCKET; i++) {
    if (bucket->keys[i] == 0) {
      bucket->values[i] = ptr;
      bucket->keys[i] = key;
      return true;
    }
  }
  
  // insertion failed
  return false;
}

bool qht_insert(qht *table, void *value, uint32_t key)
{
  uint32_t index = map_key_to_index(key, table->num_buckets);
  
  // disallow other writers
  pthread_mutex_lock(&table->buckets[index].lock);
  // inform readers
  seqlock_write_begin(&table->buckets[index].seqlock);
  bool success = insert_in_bucket(&table->buckets[index], key, value);
  seqlock_write_end(&table->buckets[index].seqlock);
  pthread_mutex_unlock(&table->buckets[index].lock);

  return success;
}

bool delete_from_bucket(qht_bucket *bucket, uint32_t key)
{
  for (int i = 0; i < PER_BUCKET; i++) {
    if (bucket->keys[i] == key) {
      bucket->keys[i] = 0;
      bucket->values[i] = NULL;
      return true;
    }
  }

  return false;
}

bool qht_delete(qht *table, uint32_t key)
{
  int index = map_key_to_index(key, table->num_buckets);
  
  pthread_mutex_lock(&table->buckets[index].lock);
  seqlock_write_begin(&table->buckets[index].seqlock);
  bool success = delete_from_bucket(&table->buckets[index], key);
  seqlock_write_end(&table->buckets[index].seqlock);
  pthread_mutex_unlock(&table->buckets[index].lock);
  
  return success;
}
