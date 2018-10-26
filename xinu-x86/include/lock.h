/*  lock.h	*/


// declare variables, #defines, 
#define NLOCKS 50
#define NONE -1
#define READ 1
#define WRITE 2
#define DELETED -20
#define LFREE '\01'
#define LUSED '\02'
#define READ_LOCKED '\03'
#define WRITE_LOCKED '\04'
struct	lockent {
	//struct members
	char lstate;
	qid16 lqhead;
	int ltype;
	int lrefNum;
	int lprio;
	int procArray[NPROC];
	int maxWritePrio;
};

struct lstat {
	int type;
	long int time;
};


/* Lab 3 lock table */
extern struct lockent locks[];
extern struct lstat locktab[][NLOCKS];
extern int nextlock;
extern int refNum;
#define firstType(head) (queuetab[queuetab[(head)].qnext].qtype)
#define isbadlock(l) (l < 0 || l >= NLOCKS)

