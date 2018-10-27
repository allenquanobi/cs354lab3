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
		lptr->lprio = maxWaitQueue();
		proctab[currpid].locks[ldes] = 0;
		lptr->procArray[currpid] = 0;
		int j = 0;
		int stilluse = checkUse(ldes);
		if(stilluse) {
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
			for(i = 0; i < NPROC; i++) {
				if(proctab[i].locks[ldes] == 1) {
					j++;
				}
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
int checkUse(int ldes) {
	struct lockent *lptr = &locktab[ldes];
	int j = 0;
	for(j = 0; j < NPROC; j++) {
		if(lptr->procArray[j] == 1) {
			return 1;
		}
	}
	return 0;
}
int maxWaitQueue(int ldes) {
	struct lockent *lptr;
	int maxprio = -1;
	int i;
	lptr = &locktab[ldes];
	i = queuetab[lptr->lqhead].qnext;
	if(nonempty(lptr->lqhead)) {
		int test = firstkey(lptr->lqhead);
		if(test > maxprio) {
			return test;
		}
	}
	return maxprio;
}
void resetPrio(int ld, int pid) {
	int nlocks = -1;
	int tmplid = -1;
	int hprio = -1;
	int maxprio = -1;
	int iflag = 0;
	struct procent *prptr = NULL;
	intmask mask;
	mask = disable();
	if(isbadlock(ld) || isbadpid(pid)) {
		restore(mask);
		return;
	}
	prptr = &proctab[pid];
	if(prptr->changePrioFlag == 0) {
		restore(mask);
		return;
	}
	if(prptr->nlocks == 0) {
		prptr->prprio = prptr->oprio;
		prptr->oprio = 0;
		prptr->changePrioFlag = 0;
		prptr->plock = -1;
		restore(mask);
		return;
	}
	maxprio = prptr->prprio;
	for(tmplid = 0; tmplid < NLOCKS; tmplid++) {
		if(locktab[tmplid].procArray[pid] > 0) {
			if(maxprio < locktab[tmplid].lprio) {
				maxprio = locktab[tmplid].lprio;
				iflag = 1;
			}
		}
	}
	if(iflag == 1) {
		prptr->prprio = maxprio;
		prptr->changePrioFlag = 1;
	}
	restore(mask);
	return;
}
