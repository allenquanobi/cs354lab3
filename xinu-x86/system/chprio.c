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
	if(prptr->changePrioFlag == 0) {
		prptr->prprio = newprio;
	} else {
		int oprio = prptr->oprio;
		int prprio = prptr->prprio;
		prptr->oprio = newprio;
		if(prprio < newprio) {
			prptr->prprio = newprio;
		}
	}
	swapPriority(prptr->plock, pid);
	if(prptr->prstate == PR_READY) {
		resched();
	}
	restore(mask);
	return oldprio;
}
