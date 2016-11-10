/*
 * Seqlock implementation for QEMU
 *
 * Copyright Red Hat, Inc. 2013
 *
 * Author:
 *  Paolo Bonzini <pbonzini@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#ifndef QEMU_SEQLOCK_H
#define QEMU_SEQLOCK_H

typedef struct seqlock_t {
    unsigned sequence;
} seqlock_t;

static inline void seqlock_init(seqlock_t *sl)
{
    sl->sequence = 0;
}

/* Lock out other writers and update the count.  */
static inline void seqlock_write_begin(seqlock_t *sl)
{
  __atomic_store_n(&sl->sequence, sl->sequence + 1, __ATOMIC_RELEASE);
}

static inline void seqlock_write_end(seqlock_t *sl)
{
  __atomic_store_n(&sl->sequence, sl->sequence + 1, __ATOMIC_RELEASE);
}

static inline unsigned seqlock_read_begin(seqlock_t *sl)
{
    /* Always fail if a write is in progress.  */
  unsigned ret = __atomic_load_n(&sl->sequence, __ATOMIC_ACQUIRE);

  return ret & ~1;
}

static inline bool seqlock_read_end(const seqlock_t *sl, unsigned start)
{
  return (__atomic_load_n(&sl->sequence, __ATOMIC_ACQUIRE) != start);
}

#endif
