/*	releaseall.c - releaseall	*/
#include <xinu.h>
/* Lab 3: Complete this function */
syscall releaseall (int32 numlocks, ...) {
	intmask mask;
	mask = disable();
	int ldes;
	struct lockent *lptr;
	int i;
	int returnValue = OK;
	unsigned long *a = (unsigned long *)(&numlocks) + (numlocks);
	for(i = 0; i < numlocks; i++) {
		ldes = *a--;
		if(isbadlock(ldes) || (lptr = &locktab[ldes])->lstate == LFREE || proctab[currpid].locks[ldes] != 1) {
			if(proctab[currpid].pwaitret == DELETED) {
				returnValue = DELETED;
			} else {
				returnValue = SYSERR;
			}
			continue;
		}
		resetPrio(ldes, currpid);
		lptr = &locktab[ldes];
		lptr->lprio = maxWaitQueue(ldes);
		proctab[currpid].locks[ldes] = 0;
		lptr->plist[currpid] = 0;
		int j = 0;
		int stilluse = checkUse(ldes);
		if(stilluse == 0) {
			resched();
			continue;
		}
		emptyWaitQueue(ldes);
	}
	resched();
	restore(mask);
	return returnValue;
}
