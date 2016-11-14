#include "qht.h"
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <thread>
#include <atomic>

#define ENTRIES_PER_THREAD 100000

std::atomic<bool> threads_spawned(false);

void thread_fn(qht *table, int tid, int update)
{
  while (!threads_spawned);

  for (int operations = 0; operations < ENTRIES_PER_THREAD/100; operations++) {
    for (int ops = 0; ops < 100; ops++) {
      uint32_t key = tid * 1000000 + operations * 1000 + ops + 1;
      if (ops < update) {
        // insert
        assert(qht_insert(table, key, (void *)(uint64_t)key) == true);
      } else {
        // read
        qht_lookup(table, key);
      }
    }
  }
}

int main(int argc, char **argv)
{
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " --threads(-t) threads --update(-u)" << std::endl;
    exit(1);
  }

  static struct option long_options[] = {
    {"help",  no_argument, NULL, 'h'},
    {"threads", required_argument, NULL, 't'}
  };

  std::thread **threads;

  qht *table = qht_init(100000);

  int c = 0, nthreads = 0;
  int update = 0;
  while((c = getopt_long(argc, argv, "ht:u:", long_options, NULL)) != -1) {
    switch (c) {
      case 't':
        nthreads = atoi(optarg);
        break;
      case 'u':
        update = atoi(optarg);
        assert(0 <= update);
        assert(update <= 100);
        break;
      default:
        std::cout << "Usage: " << argv[0] << " --threads(-t) threads --update(-u)" << std::endl;
        exit(1);
    }
  }

  for (int i = 0; i < nthreads * ENTRIES_PER_THREAD; i++)
    assert(qht_insert(table, i, (void *)((uint64_t)0xFF+i)) == true);

  threads = new std::thread*[nthreads];

  for (int i = 0; i < nthreads; i++) {
    threads[i] = new std::thread(&thread_fn, table, i, update);
  }

  threads_spawned = true;

  for (int i = 0; i < nthreads; i++) {
    threads[i]->join();
  }
}
