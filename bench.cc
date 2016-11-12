#include "qht.h"
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <thread>

#define ENTRIES_PER_THREAD 100000

void thread_fn(qht *table, int tid, int update)
{
  uint32_t key = 0xFF >> tid;
  for (int operations = 0; operations < ENTRIES_PER_THREAD/100; operations++) {
    for (int ops = 0; ops < 100; ops++) {
      if (ops < update) {
	// insert
	qht_insert(table, key+operations*1000+ops,
		   (void*)((uint64_t)key+operations*1000+ops));
      } else {
	// read
	qht_lookup(table, key+operations*1000+ops);
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

  qht *table = qht_init(1000000);

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

  for (int i = 0; i < nthreads; i++) {
    threads[i]->join();
  }

  for (int i = 0; i < ENTRIES_PER_THREAD * nthreads; i++) {
    assert(qht_lookup(table, i) != NULL);
  }
}
