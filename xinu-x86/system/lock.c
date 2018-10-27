/*	lock.c - lock */
#include <xinu.h>

/* Lab 3: Complete this function */

syscall lock(int32 ldes, int32 type, int32 priority) {
	intmask mask;
	struct lockent *lptr;
	struct procent *prptr;
	int shouldWait = 0, i, flag = 0;
	int lockid = ldes;
	mask = disable();
	prptr = &proctab[currpid];
	if(prptr->pwaitret == DELETED) {
		kprintf("deleted lock\n");
		restore(mask);
		return SYSERR;
	}
	if(type != READ && type != WRITE) {
		kprintf("invalid type \n");
		restore(mask);
		return SYSERR;
	}
	if(locktab[ldes].lstate == LFREE) {
		kprintf("need to create lock first\n");
		restore(mask);
		return SYSERR;
	}
	if(isbadlock(ldes)) {
		kprintf("invalid lid\n");
		restore(mask);
		return SYSERR;
	}
	int shouldPutInWait = 0;
	lptr = &locktab[lockid];
	switch(type) {
		case READ:
			switch(lptr->lstate) {
				case LUSED:
					shouldPutInWait = 0;
					break;
				case READ_LOCKED:
					if(priority > lptr->maxWritePrio) {
						shouldPutInWait = 0;
					} else {
						shouldPutInWait = 1;
					}
					break;
				default:
					shouldPutInWait = 1;
			}
			break;
		case WRITE:
			switch(lptr->lstate) {
				case LUSED:
					shouldPutInWait = 0;
					break;
				default:
					shouldPutInWait = 1;
					if(priority > lptr->maxWritePrio) {
						lptr->maxWritePrio = priority;
					}
			}
			break;
	}
	if(shouldPutInWait) {
		waitLock(lockid, currpid, priority, type); 
		restore(mask);
		return prptr->pwaitret;
	} else {
		if(type == READ) {
			lptr->lstate = READ_LOCKED;
		} else {
			lptr->lstate = WRITE_LOCKED;
		}
		(prptr = &proctab[currpid])->locks[lockid] = 1;
		prptr->nlocks++;
		lptr->plist[currpid] = 1;
		restore(mask);
		return OK;
	}
}
int max(int a, int b) {
	return (a>b)?a:b;
}
