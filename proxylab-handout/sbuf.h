#ifndef __SBUF_H__
#define __SBUF_H__

#include <semaphore.h>

typedef struct {
    int *buf;          /* buffer array */
    int n;             /* capacity */
    int front;         /* index of first item */
    int rear;          /* index of last item */
    sem_t mutex;       /* protects accesses to buf */
    sem_t slots;       /* available slots */
    sem_t items;       /* available items */
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int  sbuf_remove(sbuf_t *sp);

#endif
