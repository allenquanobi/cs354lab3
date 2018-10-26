/*	linit.c	- linit	initialize lock system */
#include <xinu.h>

/* Lab 3: Complete this function */

// declare any global variables here
struct lockent locks[NLOCKS];
int nextlock;
int refNum;
void linit(void) {
	struct lockent *lptr;
	int i, j;
	nextlock = NLOCKS - 1;
	refNum = 0;
	for(i = 0; i < NLOCKS; i++) {
		lptr = &locks[i];
		lptr->lstate = LFREE;
		lptr->lqhead = newqueue();
		lptr->lrefNum = -1;
		lptr->lprio = -1;
		lptr->maxWritePrio = -1;
	}
}
