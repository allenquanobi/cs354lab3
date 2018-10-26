/*	ldelete.c - ldelete 	*/
#include <xinu.h>

/* Lab 3: Complete this function */

syscall ldelete( 
		int32 ldesc	/* lock descriptor */
	)
{
	intmask mask;
	int pid, i, flag = 0;
	struct lockent *lptr;
	mask = disable();
	if(isbadlock(ldesc) || locks[ldesc].lstate == LFREE) {
		restore(mask);
		return SYSERR;
	}
	lptr = &locks[ldesc];
	lptr->lstate = LFREE;
	if(nonempty(lptr->lqhead)) {
		pid = getfirst(lptr->lqhead);
		while (pid != EMPTY) {
			proctab[pid].pwaitret = DELETED;
			proctab[pid].plock = -1;
			ready(pid);
			pid = getfirst(lptr->lqhead);
		}
		resched();
	}
	for(i = 0; i < NPROC; i++) {
		locktab[i][ldesc].time = -1;
		locktab[i][ldesc].type = NONE;
	}
	restore(mask);
	return OK;
}
