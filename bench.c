#include "qht.h"
#include <assert.h>

int main()
{
  qht *table = qht_init(10000);

  for (int i = 0; i < 10000; i++)
    assert(qht_insert(table, i, (void *)((uint64_t)0xFF+i)) == true);

  for (int i = 0; i < 10000; i++) {
    assert(qht_lookup(table, i) != NULL);
  }
}
