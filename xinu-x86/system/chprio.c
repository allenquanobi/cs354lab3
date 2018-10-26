/* chprio.c - chprio */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  chprio  -  Change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
pri16	chprio(
	  pid32		pid,		/* ID of process to change	*/
	  pri16		newprio		/* New priority			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	pri16	oldprio;		/* Priority to return		*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return (pri16) SYSERR;
	}
	prptr = &proctab[pid];
	oldprio = prptr->pprio;
	prptr->pprio = newprio;
	if(prptr->plock > 0)
		swapPriority(prptr->plock, pid);
	int i = 0;
	int j = 0;
	for(i = 0; i < NLOCKS; i++) {
		if(locks[i].procArray[pid] > 0) {
			for(j = 0; j < NPROC; j++) {
				if(proctab[j].plock == i) {
					swapPriority(i, j);
				}
			}
		}
	}
	restore(mask);
	return oldprio;
}
