#ifndef _MBOX_H_
#define _MBOX_H_
#include "os_api.h"

typedef OS_EVENT *mbox_t;
void mbox_new(mbox_t *mbox, void *pmsg);
void *mbox_get(mbox_t *mbox, unsigned int timeout);
int mbox_post(mbox_t *mbox, void *pmsg);
int mbox_destroy(mbox_t *mbox);

#endif