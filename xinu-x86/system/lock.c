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
	if ( READ == type )
	{
		if (lptr->lstate == LUSED)
		{
			shouldPutInWait = 0;
		}
		else if ( lptr->lstate == READ_LOCKED)
		{
			if ( priority > lptr->maxWritePrio)
			{
				shouldPutInWait = 0;
			}
			else
			{
				shouldPutInWait = 1;
			}
		}
		else
		{
			shouldPutInWait = 1;
		}

	}
	else
	{
		if (lptr->lstate == LUSED)
		{
			shouldPutInWait = 0;	
		}
		else
		{
			shouldPutInWait = 1;
			if(priority > lptr->maxWritePrio) {
				lptr->maxWritePrio = priority;
			}
		}
	}
	if(shouldPutInWait) {
		(prptr = &proctab[currpid])->prstate = PR_WAIT;
		prptr->plock = lockid;
		lptr->lprio = max(lptr->lprio,proctab[currpid].prprio);
		swapPriority(lockid,currpid);
		insertlockq(currpid,lptr->lqhead,priority,type);
		prptr->pwaitret = OK;
		resched();
		lptr->procArray[currpid] = 1;
		proctab[currpid].locks[lockid] = 1;
		proctab[currpid].nlocks++;
		for(i=0;i<NPROC;i++)
		{
			if(proctab[i].plock == lockid)
			{
				swapPriority(proctab[i].plock,i);
			}

		}
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
		lptr->procArray[currpid] = 1;
		restore(mask);
		return OK;
	}
}
int max(int a, int b) {
	return (a>b)?a:b;
}
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
		if(locktab[ld].procArray[i] > 0) {
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
