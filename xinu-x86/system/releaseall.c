/*	releaseall.c - releaseall	*/
#include <xinu.h>
/* Lab 3: Complete this function */
syscall releaseall (int32 numlocks, ...) {
	intmask mask;
	mask = disable();
	int ldes, flag = 0;
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
		if(nonempty(lptr->lqhead)) {
			do {
				flag = 0;
				if(firstType(lptr->lqhead) == READ) {
					lptr->lstate = READ_LOCKED;
					flag = 1;
				} else {
					lptr->lstate = WRITE_LOCKED;
				}
				proctab[queuetab[(lptr->lqhead)].qnext].plock = -1;
				proctab[queuetab[(lptr->lqhead)].qnext].nlocks--;
				pid32 tmp = getfirst(lptr->lqhead);
				ready(tmp);
			} while (flag && (firstType(lptr->lqhead) == READ));
		} else {
			int j = 0;
			while(i < NPROC) {
				if(proctab[i].locks[ldes] == 1) {
					j++;
				}
				i++;
			}
			if(j < 1) {
				lptr->lstate = LUSED;
			}
		}
	}
	resched();
	restore(mask);
	return returnValue;
}
