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
	if(isbadlock(ldesc) || locktab[ldesc].lstate == LFREE) {
		restore(mask);
		return SYSERR;
	}
	lptr = &locktab[ldesc];
	lptr->lstate = LFREE;
	lptr->ltype = -1;
	lptr->lprio = -1;
	lptr->maxWritePrio = -1;
	for(i = 0; i < NPROC; i++) {
		lptr->procArray[i] = 0;
	}
	for(i = 0; i < NLOCKS; i++) {
		proctab[i].locks[ldesc] = 0;
	}
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
	restore(mask);
	return OK;
}
