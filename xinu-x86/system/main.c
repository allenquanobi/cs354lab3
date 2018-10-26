#include <xinu.h>

int lck;

#ifndef NLOCKS
#define NLOCKS 50
#endif
#ifndef READ 
#define READ 0
#define READL READ
#endif
#ifndef WRITE 
#define WRITE 1
#define WRITEL WRITE
#endif
#ifndef DELETED
#define DELETED (-4)
#endif

int lcks [NLOCKS];

void reader1( int, int, int );
void writer1( int, int, int );
void loop(int);

void test0(void);
void test1(void);
void test2(void);
void test3(void);
void test4(void);
void test5(void);
void test6(void);

int main(int argc, char** argv) {
	kprintf("\n\nCS354 Lab3 \n\r");
	kprintf("\n\nRunning test 0\n\r");
	test0();
	kprintf("\n\nRunning test 1\n\r");
	test1();
	kprintf("\n\nRunning test 2\n\r");
	test2();
	kprintf("\n\nRunning test 3\n\r");
	test3();
	kprintf("\n\nRunning test 4\n\r");
	test4();
	kprintf("\n\nRunning test 5\n\r");
	test5();
	kprintf("\n\nRunning test 6\n\r");
	test6();
	return 0;
}


/*Test0	- Regression testing
 *Expected Output:
 *	TEST0 
 *	TEST0	DONE
 */

void test0()
{
	int i, j, a;

	for( j = 0; j < 4; j++ )	
	{
		for( i = 0; i < NLOCKS; i++ )
		{
			lcks[i] = lcreate();	
			if( lcks[i] == SYSERR )
			{
				kprintf(" TEST0: lcreate failed ..DONE\n\r");
				return;
			}
		}
		//kprintf("finished creating locks. starting lock acquires\n");
		for( i = 0; i < NLOCKS; i++ )
		{
			a = lock( lcks[i], READ, 0 );
			if( a == SYSERR )
			{
				kprintf(" TEST0: lock failed ..DONE: %d\n\r", a);
				return;
			}
		}
		//kprintf("locks have been acquired.  releasing them now\n");
		for( i = 0; i < NLOCKS; i++ )
		{
			a = releaseall( 1,lcks[i] );
			if( a == SYSERR )
			{
				kprintf(" TEST0: release failed ..DONE\n\r");
				return;
			}
		}
		//kprintf("locks have been released.  deleting them now\n");
		for( i = 0; i < NLOCKS; i++ )
		{
			a = ldelete( lcks[i] );
			if( a == SYSERR )
			{
				kprintf(" TEST0: release failed ..DONE\n\r");
				return;
			}
		}
		//kprintf("locks have been deleted\n");
	}
	kprintf(" TEST0	DONE \n\r");
}

/* Test 1 - Test for basic lcreate, lock, release ( read lock ) 
 *
 * Expected Output : 
 *	TEST1
 * 
 *
 *	Reader1: lock .. 
 *	Reader2: lock .. 
 *	Reader3: lock .. 
 *
 *	Reader1: Releasing .. 
 *	Reader2: Releasing .. 
 *	Reader3: Releasing .. 
 * 	( The order doesnt matter . The last 3  
 *	  statements will be printed after approx 3 seconds )
 *	TEST1: Completed
 *	       
 */

void test1()
{
	int  a;
	lck = lcreate();
	
	if( lck == SYSERR )
	{
	  kprintf(" TEST1: error in lcreate()..DONE\n\r");
	  return;
	}

	resume( create( reader1, 2000, 30, "reader", 3, lck, 1, 0 ));
	resume( create( reader1, 2000, 30, "reader", 3, lck, 2, 0 ));
	resume( create( reader1, 2000, 30, "reader", 3, lck, 3, 0 ));

	/* wait for them to complete and then delete the lock */
	sleep( 5 );
	kprintf(" TEST1: deleting lock\n");
	a = ldelete( lck );

	if( a != OK )
	{
		kprintf(" TEST1: error in ldelete()..DONE\n\r");
		return;
	}
	kprintf(" TEST1: DONE \n\r");
	

}

void reader1 ( int lck, int num, int prio )
{
	int a;
	//kprintf("getting lock: %d\n", num);
	a = lock( lck, READ, prio );
	if( a != OK )
	{
	  kprintf(" Reader%d: lock failed %d ..\n\r", num, a ); 
	  return;
	}

	kprintf(" Reader%d: Lock ..\n\r", num );
	sleep(3);
	kprintf(" Reader%d: Releasing ..\n\r", num );

	a = releaseall( 1,lck );
	if( a != OK )
		kprintf(" Reader%d: Lock release failed %d ..\n\r", num, a ); 
	else
		kprintf(" Reader%d: Lock release done ..\n\r", num ); 
}

/* Test2 - Test for basic lcreate, lock, release ( for a write lock ) 
 *
 * Expected output:
 *	Writer1: Lock ..
 *	Writer1: Releasing  ..
 *     	(after 3 seconds )
 *	Writer2: Lock ..
 *	Writer2: Releasing ..
 *     	(after 3 seconds )
 *	Writer3: Lock ..
 *	Writer3: Releasing ..
 *     The order is not important. What is important is that the lock/release 
 * for a process should not be interspersed with the lock/release for another process
 */


void test2()
{
	int a; 

	kprintf("	TEST2	\n\r\n\r");
	lck = lcreate();

	if( lck == SYSERR )
	{
	  kprintf(" TEST2: error in lcreate()..DONE\n\r");
	  return;
	}
	resume( create( writer1, 2000, 30, "writer", 3, lck, 1, 0 ));
	resume( create( writer1, 2000, 30, "writer", 3, lck, 2, 0 ));
	resume( create( writer1, 2000, 30, "writer", 3, lck, 3, 0 ));

	/* Wait for the writers to complete and then delete the lock */

	sleep(15);
	a = ldelete( lck );

	if( a != OK )
	  kprintf(" TEST2: error in ldelete()..\n\r");

	kprintf("	TEST2	DONE\n\r");
	
}

void writer1 ( int lck, int num, int prio )
{
	int a;
	a = lock( lck, WRITE, prio );
	if( a != OK )
	  {
	    kprintf(" Writer%d: lock failed %d..\n\r", num, a ); 
	    return;
	  }
	
	kprintf(" Writer%d: Lock ..\n\r", num );
	sleep(3);
	kprintf(" Writer%d: Releasing ..\n\r", num );

	a = releaseall(1, lck );
	if( a != OK )
	  kprintf(" Writer%d: Lock release failed %d ..\n\r", num,a ); 
	return;
}


/* Test3: Testing for SYSERRs in case of erroneous inputs 
 * Expected output - SYSERRs in all cases 
 */

void test3()
{
	/* Locking without creating */
	
	lck = 2;
	kprintf(" lock(2,READ) without lcreate() : %d \n\r", lock( lck, READ,0 ) ); 

	/* Locking with invalid id */

	lck = -2;
	kprintf(" lock(-2,READ)  : %d \n\r", lock( lck, READ,0 ) ); 


	/* Locking with an invalid mode */

	lck = lcreate();
	if( lck == SYSERR ) 
		kprintf(" TEST3: lcreate() failed .. DONE\n\r");

	kprintf(" lock(lck,INVALID) : %d \n\r", lock( lck, -9,0 ) ); 
	ldelete(lck);

	/* Deleting without creating  */

	lck = 10;
	kprintf(" ldelete(lck) without lcreate() : %d \n\r", ldelete( lck ) ); 


	/* Deleting with an invalid ID */

	lck = -1;
	kprintf(" ldelete(-1)  : %d \n\r", ldelete( lck ) ); 
}

/*Test4 - readers and writers of equal priority 
 *Expected Output: 
 *	Reader1: lock .. 
 *	Reader2: lock .. 
 *	Reader1: Releasing  .. 
 *	Reader2: Releasing  .. 
 *	Writer1: Lock ..
 *	Writer1: Releasing ..
 *	Writer2: Lock ..
 *	Writer2: Releasing ..
 *	Reader3: lock .. 
 *	Reader4: lock .. 
 *	Reader3: Releasing  .. 
 *	Reader4: Releasing  .. 
 * 	( previous 2 statements can be in any order )
 */

void test4()
{
	int a;
	
	kprintf("	TEST4	\n\r\n\r");
	
	lck = lcreate( );

	resume( create( reader1, 2000, 30, "reader", 3, lck, 1, 0 ));
	resume( create( reader1, 2000, 30, "reader", 3, lck, 2, 0 ));
	resume( create( writer1, 2000, 30, "writer", 3, lck, 1, 0 ));

	sleep(11);
	resume( create( writer1, 2000, 30, "writer", 3,lck, 2, 0 ));
	resume( create( reader1, 2000, 30, "reader", 3,lck, 3, 0 ));
	resume( create( reader1, 2000, 30, "reader", 3,lck, 4, 0 ));

	sleep(11);
	
	a = ldelete(lck);

	if( a != OK )
		kprintf(" TEST4: error in ldelete()..\n\r");

	kprintf("	TEST4 DONE	\n\r\n\r");
}

/* Test5
 * Testing priority inheritance on a single lock acquired by multiple processes  
 * Expected output: 
 * Reader1: Lock ..
 * Reader2: Lock..
 * Reader3: Lock..
 * pid of medium priority process = x
 * Reader1: Releasing
 * Reader2: Releasing
 * Reader3: Releasing
 * Writer1: Lock
 * Writer 1: Releasing
 * pid x exiting from loop
 * (The order of the readers acquiring / releasing the locks does not matter )
*/
void test5() {
  int lk = lcreate();
  int mpid;

  resume( create( reader1, 2000, 30, "reader", 3, lk, 1, 0 ));
  resume( create( reader1, 2000, 30, "reader", 3, lk, 2, 1 ));
  resume( create( reader1, 2000, 30, "reader", 3, lk, 3, 2 ));
  /* The 3 readers have acquired the lock */
  chprio(getpid(),60);
  resume(mpid = create(loop,2000,40,"loop",1,10));
  kprintf("pid of medium priority process = %d\n\r",mpid);
  resume(create(writer1,2000,50,"writer",3,lk,1,0));
  chprio(getpid(),20);
  sleep(10);
  if(ldelete(lk)!=OK) 
    kprintf("Test5 : Error while deleting lock\n\r");
  else
	  kprintf("deleted!!!\r\n");
  kprintf("Test 5 done..\n\r");
}

/*Test6 - DELETED test
*Points - 10
*Expected Output:
*      Reader1: Lock ..
*      Writer1: Lock failed (value of DELETED here)..
*      Writer2: Lock failed (value of DELETED here)..  (order of these 3 doesn't matter)
*      Reader2: Lock failed SYSERR..
*      Reader1: Releasing ..
*      Reader1: Lock Release failed SYSERR..
*      TEST7:  DONE
*
*/

void test6()
{
       int a;

       kprintf("       TEST6   \n\r\n\r");

       lck = lcreate();

       if( lck == SYSERR )
       {
               kprintf(" TEST7: lcreate failed ...DONE\n\r\n\r");
               return;
       }

       resume( create( reader1, 2000, 30, "reader", 3, lck, 1, 0 ));
       resume( create( writer1, 2000, 30, "writer", 3, lck, 1, 0 ));
       resume( create( writer1, 2000, 30, "writer", 3, lck, 2, 0 ));

       a = ldelete(lck);
       if( a != OK )
       {
               kprintf(" TEST6: ldelete failed ...\n\r\n\rDONE");
               return;
       }
       resume( create( reader1, 2000, 30, "reader", 3, lck, 2, 0 ));

       sleep(5);
       kprintf(" TEST6: DONE \n\r" );

}


void loop(int timetoloop) {
  kprintf("pid %d starting loop\n\r", getpid());
  const unsigned int start_time = clktime;
  while((clktime - start_time) <= timetoloop);
  kprintf("pid %d exiting from loop \n\r",getpid());
}

  
