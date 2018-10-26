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
	//kprintf("here\n");
	//kprintf("lock state: %d\n",locks[lockid].lstate);
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
			if ( priority > maxwritepriority(lockid))
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
				// There is a higher priority write process waiting, put the lock in the queue
				//kprintf("wat\n");
				(prptr = &proctab[currpid])->prstate = PR_WAIT;
				prptr->plock = lockid;
				lptr->lprio = max(lptr->lprio,proctab[currpid].pprio);
				swapPriority(lockid,currpid);
				insertlock(currpid,lptr->lqhead,priority,type);
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
		}
		else // lock currently held by writer process, put in the lock queue
		{
			//kprintf("lptr->lstate: %d\n", lptr->lstate);
			//kprintf("lptr->lstate string: %s\n", lptr->lstate);
			//kprintf("wat part 2\n");
			(prptr = &proctab[currpid])->prstate = PR_WAIT;
			prptr->plock = lockid;
			lptr->lprio = max(lptr->lprio,proctab[currpid].pprio);
			swapPriority(lockid,currpid);
			insertlock(currpid,lptr->lqhead,priority,type);
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
			//kprintf("here2\n");
			// Lock already held, Add to Lock wait queue
			(prptr = &proctab[currpid])->prstate = PR_WAIT;
			prptr->plock = lockid;
			lptr->lprio = max(lptr->lprio,proctab[currpid].pprio);
			//printf("Lock already held,pptr->plock = %d , lptr->lprio =%d \n", pptr->plock,lptr->lprio);
			swapPriority(lockid,currpid);
			//printf("AFter Priority inheritance, process inherited prio = %d \n", pptr->pinh);
			//kprintf("lptr->lqhead: %d\n", lptr->lqhead);
			insertlock(currpid,lptr->lqhead,priority,type);
			//kprintf("lptr->lqhead after insertlock: %d\n", lptr->lqhead);
			if(priority > lptr->maxWritePrio) {
				lptr->maxWritePrio = priority;
			}
			prptr->pwaitret = OK;
			//kprintf("lptr->lqhead before resched in lock, should be 400: %d\n", lptr->lqhead);
			resched();
			//kprintf("lptr->lqhead after resched in lock, should be 400: %d\n", lptr->lqhead);
			lptr->procArray[currpid] = 1;
			proctab[currpid].locks[lockid] = 1;
			proctab[currpid].nlocks++;
			//kprintf("lptr->lqhead before swapPriority in lock, should be 400: %d\n", lptr->lqhead);
			for(i=0;i<NPROC;i++)
			{
				if(proctab[i].plock == lockid) {
					swapPriority(proctab[i].plock,i);
				}

			}
			//kprintf("lptr->lqhead after swapPriority in lock, should be 400: %d\n", lptr->lqhead);
			restore(mask);
			return prptr->pwaitret;		
		}
		//writeLockAcquire(ldes1, priority);
	}
	restore(mask);
	return OK;
}
int max(int a, int b) {
	return (a>b)?a:b;
}
int insertlock(int proc, int head, int key, int type) {
	int next;
	int prev;
	//kprintf("type: %d\n", type);
	//kprintf("procid: %d\n", proc);
	//kprintf("process name: %s\n", proctab[proc].prname);
	next = firstid(head);
	//kprintf("lptr->lqhead in insertlock: %d\n", head);
	while(queuetab[next].qkey >= key) {
		//kprintf("insertlockwhile\n");
		//kprintf("in lockwaitlist: \n");
		//kprintf("type: %d\n", queuetab[next].qtype);
		//kprintf("procid: %d\n", next);
		//kprintf("process name: %s\n", proctab[next].prname);
		next = queuetab[next].qnext;
	}
	//kprintf("gets past insertwhileloop\n");
	//while((queuetab[next].qtype == READ) && (queuetab[next].qkey == key))
		//next = queuetab[next].qnext;
	//queuetab[proc].qnext = next;
	prev = queuetab[next].qprev;
	queuetab[proc].qnext = next;
	queuetab[proc].qprev = prev;
	queuetab[proc].qkey = key;
	queuetab[proc].qtype = type;
	queuetab[prev].qnext = proc;
	queuetab[next].qprev = proc;
	//kprintf("lptr->lqhead after insertlock: %d\n", head);
	//kprintf("inserted noice\n");
	return OK;
}
int maxwritepriority(int ldesc) {
	/*int curr;
	struct lockent *lptr = &locks[ldesc];
	if(nonempty(lptr->lqhead)) {
		curr = queuetab[lptr->lqtail].qprev;
		while(queuetab[curr].qtype != WRITE) {
			//kprintf("this while loop\n");
			curr = queuetab[curr].qprev;
		}
		if(queuetab[curr].qtype == WRITE)
			return queuetab[curr].qkey;
		else 
			return 0;
	}*/
	struct lockent *lptr = &locks[ldesc];
	return lptr->maxWritePrio;
}
int getMax(int a, int b) {
	return (a>b)?a:b;
}
void priority_inheritance(int ldes, int pid)
{
	//kprintf("GETS TO HERE AND U SHOULD CHECK IF MAIN IS GETTING ITS PRIORITY CHANGED\n");
	// REMOVE
	//return;
	int i;
	if(ldes==-1)
	{
		//printf("priority_inheritance, LDES is -1 \n");
		return;
	}
	struct procent *prptr = &proctab[pid];
	int baseCase = 0;
	for(i = 0; i < NPROC; i++) {
		if(proctab[i].locks[ldes] > 0) {
			if(proctab[i].pprio > prptr->pprio) 
				baseCase = 1;
		}
	}
	if(baseCase == 0) {
		return;
	}
	chprio(pid, locks[ldes].lprio);
	for(i=0;i<NPROC;i++)
	{

		if(locks[ldes].procArray[i] > 0)
		{
			// this lock is held by some other process
			//process i holds the lock ldes, so see if you need to update it priority.

			if(proctab[pid].pinh > 0) //check if the current process waiting to acquire the lock already has some inherited priority
			{
				if(proctab[i].pinh > 0) // if the other process already has some inherited priority, comapre against that
				{
					if(proctab[pid].pinh > proctab[i].pinh)   // if your inherited priority is greater than his inherited prio, swap
						proctab[i].pinh = proctab[pid].pinh;
				}

				else // the other process does not have any inherited priority
				{
					if(proctab[pid].pinh > proctab[i].pprio)
						proctab[i].pinh = proctab[pid].pinh;

				}

			}
			else // ur current process does not have any inherited prioroty
			{
				if(proctab[i].pinh > 0) // if the other process has inherited prio, compare it with ur base prio and swap
				{
					if(proctab[pid].pprio > proctab[i].pinh)
						proctab[i].pinh = proctab[pid].pprio;
				}
				else
				{
					if(proctab[pid].pprio > proctab[i].pprio)
						proctab[i].pinh = proctab[pid].pprio;
				}

			}
			//printf("\nUpdated pinh for process %d is %d\n",i,proctab[i].pinh);
			priority_inheritance(proctab[i].plock,i);

		}

	} // End of for	
	//kprintf("proctab[pid] name: %s\n", proctab[pid].prname);
	//kprintf("proctab[pid] prio: %d\n", proctab[pid].pprio);
	//kprintf("proctab[pid] state: %d\n", proctab[pid].prstate);
	//printf ("Exiting Priority INHERITANCE \n");
	return ;

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
