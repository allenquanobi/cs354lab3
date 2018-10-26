/*	linit.c	- linit	initialize lock system */
#include <xinu.h>

/* Lab 3: Complete this function */

// declare any global variables here
struct lockent locktab[NLOCKS];
int nextlock;
void linit(void) {
	struct lockent *lptr;
	int i, j;
	nextlock = NLOCKS - 1;
	for(i = 0; i < NLOCKS; i++) {
		lptr = &locktab[i];
		lptr->lstate = LFREE;
		lptr->lqhead = newqueue();
		lptr->lprio = -1;
		lptr->maxWritePrio = -1;
	}
}
