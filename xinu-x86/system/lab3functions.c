#include <xinu.h>
int insertlockq(int proc, int head, int key, int type) {
	int next;
	int prev;
	next = firstid(head);
	while(queuetab[next].qkey >= key && (queuetab[next].qtype == READ)) {
		next = queuetab[next].qnext;
	}
	prev = queuetab[next].qprev;
	queuetab[proc].qnext = next;
	queuetab[proc].qprev = prev;
	queuetab[proc].qkey = key;
	queuetab[proc].qtype = type;
	queuetab[prev].qnext = proc;
	queuetab[next].qprev = proc;
	return OK;	
}
void swapPriority(int ld, int pid) {
	intmask mask;
	mask = disable();
	if(isbadlock(ld) || isbadpid(pid)) {
		restore(mask);
		return;
	}
	struct lockent *lptr = &locktab[ld];
	struct procent *prptr = &proctab[pid];
	int hprio = lptr->lprio;
	int cprio = prptr->prprio;
	if(hprio < cprio) {
		lptr->lprio = cprio;
		hprio = cprio;
	}
	int i = 0;
	for(i = 0; i < NPROC; i++) {
		if(locktab[ld].plist[i] > 0) {
			prptr = &proctab[i];
			int prprio = prptr->prprio;
			if(prprio < hprio) {
				if(prptr->changePrioFlag == 0) {
					prptr->changePrioFlag = 1;
					prptr->oprio = prprio;
				}
				prptr->prprio = hprio;
			}
			swapPriority(proctab[i].plock, i);
		}

	}
	restore(mask);
	return;
}
int checkUse(int ldes) {
	struct lockent *lptr = &locktab[ldes];
	int j = 0;
	while(j < NPROC) {
		if(lptr->plist[j] == 1) {
			return 0;
		}
		j++;
	}
	return 1;
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
		if(locktab[tmplid].plist[pid] > 0) {
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
void waitLock(int lid, int pid, int priority, int type) {
	struct procent *prptr = &proctab[pid];
	prptr->prstate = PR_WAIT;
	prptr->plock = lid;
	struct lockent *lptr = &locktab[lid];
	lptr->lprio = max(lptr->lprio, proctab[pid].prprio);
	swapPriority(lid, pid);
	insertlockq(pid, lptr->lqhead, priority, type);
	prptr->pwaitret = OK;
	resched();
	lptr->plist[pid] = 1;
	proctab[pid].locks[lid] = 1;
	proctab[pid].nlocks++;
	int i;
	for(i = 0; i < NPROC; i++) {
		if(proctab[i].plock == lid) {
			swapPriority(proctab[i].plock, i);
		}
	}
}
void emptyWaitQueue(int lid){
	struct lockent *lptr = &locktab[lid];
	int flag = 0;
	int i = 0;
	int j = 0;
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
		j = 0;
		while(i < NPROC) {
			if(proctab[i].locks[lid] == 1) {
				j++;
			}
			i++;
		}
		if(j < 1) {
			lptr->lstate = LUSED;
		}
	}
}
