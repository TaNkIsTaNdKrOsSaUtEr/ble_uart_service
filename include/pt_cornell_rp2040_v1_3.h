#include <string.h>
#include "hardware/sync.h"

#ifndef __LC_H__
#define __LC_H__

typedef void * lc_t;

#define LC_INIT(s) s = NULL
#define LC_RESUME(s) \
  do { \
    if(s != NULL) { \
      goto *s; \
    } \
  } while(0)

#define LC_SET(s) \
  do { \
    ({ __label__ resume; resume: (s) = &&resume; }); \
  } while(0)

#define LC_END(s)

#endif

#ifndef __PT_H__
#define __PT_H__

struct pt {
  lc_t lc;
};

#define PT_WAITING 0
#define PT_YIELDED 1
#define PT_EXITED  2
#define PT_ENDED   3

#define PT_INIT(pt)   LC_INIT((pt)->lc)

#define PT_THREAD(name_args) char name_args

#define PT_BEGIN(pt) \
  { \
    char PT_YIELD_FLAG = 1; \
    LC_RESUME((pt)->lc)

#define PT_END(pt) \
    LC_END((pt)->lc); \
    PT_INIT(pt); \
    return PT_ENDED; \
  }

#define PT_WAIT_UNTIL(pt, condition) \
  do { \
    LC_SET((pt)->lc); \
    if(!(condition)) { \
      return PT_WAITING; \
    } \
  } while(0)

#define PT_WAIT_WHILE(pt, cond)  PT_WAIT_UNTIL((pt), !(cond))

#define PT_YIELD(pt) \
  do { \
    PT_YIELD_FLAG = 0; \
    LC_SET((pt)->lc); \
    if(PT_YIELD_FLAG == 0) { \
      return PT_YIELDED; \
    } \
  } while(0)

#define PT_RESTART(pt) \
  do { \
    PT_INIT(pt); \
    return PT_WAITING; \
  } while(0)

#define PT_EXIT(pt) \
  do { \
    PT_INIT(pt); \
    return PT_EXITED; \
  } while(0)

#define PT_SCHEDULE(f) ((f) < PT_EXITED)

typedef struct {
    volatile int count;
    volatile uint32_t lock;
} pt_sem_t;

typedef struct {
    volatile uint32_t lock;
} pt_lock_t;

void PT_SEM_INIT(pt_sem_t *s, int count);
#define PT_SEM_WAIT(pt, s) PT_WAIT_UNTIL(pt, pt_sem_wait(s))
#define PT_SEM_SIGNAL(pt, s) pt_sem_signal(s)

bool pt_sem_wait(pt_sem_t *s);
void pt_sem_signal(pt_sem_t *s);

void PT_LOCK_INIT(pt_lock_t *l);
#define PT_LOCK_WAIT(pt, l) PT_WAIT_UNTIL(pt, pt_lock_try_acquire(l))
#define PT_LOCK_RELEASE(pt, l) pt_lock_release(l)

bool pt_lock_try_acquire(pt_lock_t *l);
void pt_lock_release(pt_lock_t *l);

#endif
