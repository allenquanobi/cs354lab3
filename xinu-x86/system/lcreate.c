/*	lcreate.c - lcreate	*/
#include <xinu.h>

/* Lab 3: Complete this function */
local int newlock();
int32 lcreate() {
	intmask mask;
	int lid;
	mask = disable();
	if((lid = newlock()) == SYSERR) {
		restore(mask);
		return SYSERR;
	}
	restore(mask);
	return lid;
}
local int newlock() {
	int lock;
	int i;
	int j;
	for(i = 0; i < NLOCKS; i++) {
		lock = nextlock--;
		if(nextlock < 0)
			nextlock = NLOCKS - 1;
		if(locktab[lock].lstate == LFREE){
			locktab[lock].lstate = LUSED;
			locktab[lock].lprio = -1;
			for(j = 0; j < NPROC; j++) {
				locktab[lock].plist[j] = 0;
			}
			return lock;
		}
	}
	kprintf("could not find a free lock\n");
	return SYSERR;
}
