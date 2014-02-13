#ifndef __SYS_UTILITLY_H__
#define __SYS_UTILITLY_H__

#define CREATE_LOCK(lock) {pthread_mutex_init(lock,NULL);}
#define LOCK(lock) {pthread_mutex_lock(lock);}
#define UNLOCK(lock) {pthread_mutex_unlock(lock);}
#define RELEASE_LOCK(lock) {pthread_mutex_destroy(lock);}

#endif
