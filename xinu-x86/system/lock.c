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
	if(locks[ldes].lstate == LFREE) {
		kprintf("need to create lock first\n");
		restore(mask);
		return SYSERR;
	}
	if(isbadlock(ldes)) {
		kprintf("invalid lid\n");
		restore(mask);
		return SYSERR;
	}
	//kprintf("here\n");
	//kprintf("lock state: %d\n",locks[lockid].lstate);
	int shouldPutInWait = 0;
	lptr = &locks[lockid];
	if ( READ == type )
	{
		//kprintf("ENTERED READ TYPE\n");
		//kprintf("Entered lock read part of the LOCK func");
		if (lptr->lstate == LUSED)
		{
			//kprintf("locck acquired:%d\n", lockid);
			lptr->lstate= READ_LOCKED;
			// updating the Process table with the lock descriptor
			(prptr = &proctab[currpid])->locks[lockid] =1;
			prptr->nlocks++;
			//Indicates which process currently holds the lock
			lptr->procArray[currpid] = 1;
			//printf(" Acquired Read lock \n");
			restore(mask);
			return (OK);
		}
		else if ( lptr->lstate == READ_LOCKED)
		{
			// There is already a process which has acquired the lock, so move it to wait queue or put on the lock wait queue
			if ( priority > lptr->maxWritePrio)
			{
				// Give the process the lock
				//kprintf("lock acquired:%d\n",lockid);
				(prptr = &proctab[currpid])->locks[lockid] =1;
				prptr->nlocks++;
				lptr->procArray[currpid] = 1;	
				restore(mask);
				return (OK);
			}
			else
			{
				shouldPutInWait = 1;
			}
		}
		else // lock currently held by writer process, put in the lock queue
		{
			shouldPutInWait = 1;
		}

	}
	else  //type = WRITE
	{
		//kprintf("writer in lock\n");
		//printf("Trying to Acquire Write Lock, current state = %d \n", lptr->lstate);
		if (lptr->lstate == LUSED)
		{	
			//kprintf("here1\n");
			//kprintf("lock acquired: %d\n", lockid);
			lptr->lstate= WRITE_LOCKED;
			(prptr = &proctab[currpid])->locks[lockid] =1;
			prptr->nlocks++;
			lptr->procArray[currpid] = 1;
			restore(mask);
			return (OK);
		}
		else
		{
			shouldPutInWait = 1;
		}
		//writeLockAcquire(ldes1, priority);
	}
	if(shouldPutInWait) {
		(prptr = &proctab[currpid])->prstate = PR_WAIT;
		prptr->plock = lockid;
		lptr->lprio = max(lptr->lprio,proctab[currpid].pprio);
		swapPriority(lockid,currpid);
		insertlockq(currpid,lptr->lqhead,priority,type);
		prptr->pwaitret = OK;
		//kprintf("lptr->lqhead before resched in lock, should be 400: %d\n", lptr->lqhead);
		resched();
		//kprintf("lptr->lqhead after resched in lock, should be 400: %d\n", lptr->lqhead);
		lptr->procArray[currpid] = 1;
		proctab[currpid].locks[lockid] = 1;
		proctab[currpid].nlocks++;
		//kprintf("lptr->lqhead before swapPriority, should be 400:%d\n", lptr->lqhead);
		for(i=0;i<NPROC;i++)
		{
			if(proctab[i].plock == lockid)   //now update current process holding this lock, with this proc details
			{
				swapPriority(proctab[i].plock,i);
			}

		}
		//kprintf("lptr->lqhead after swapPriority, should be 400:%d\n", lptr->lqhead);
		restore(mask);
		return prptr->pwaitret;
	}
	restore(mask);
	return OK;
}
int max(int a, int b) {
	return (a>b)?a:b;
}
int insertlockq(int proc, int head, int key, int type) {
	int next;
	int prev;
	next = firstid(head);
	while(queuetab[next].qkey >= key) {
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
	struct lockent *lptr = &locks[ld];
	struct procent *prptr = &proctab[pid];
	int hprio = lptr->lprio;
	int cprio = prptr->pprio;
	if(hprio < cprio) {
		lptr->lprio = cprio;
		hprio = cprio;
	}
	int i = 0;
	for(i = 0; i < NPROC; i++) {
		if(locks[ld].procArray[i] > 0) {
			prptr = &proctab[i];
			int pprio = prptr->pprio;
			if(pprio < hprio) {
				if(prptr->changePrioFlag == 0) {
					prptr->changePrioFlag = 1;
					prptr->oprio = pprio;
				}
				prptr->pprio = hprio;
			}
			swapPriority(proctab[i].plock, i);
		}

	}
	restore(mask);
	return;
}
