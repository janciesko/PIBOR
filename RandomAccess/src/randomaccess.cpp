//2015, Ciesko J., Mateo S., Teruel X.

/* -*- mode: C; tab-width: 2; indent-tabs-mode: nil; -*- */

/*
* This code has been contributed by the DARPA HPCS program.  Contact
* David Koester <dkoester@mitre.org> or Bob Lucas <rflucas@isi.edu>
* if you have questions.
*
* GUPS (Giga UPdates per Second) is a measurement that profiles the memory
* architecture of a system and is a measure of performance similar to MFLOPS.
* The HPCS HPCchallenge RandomAccess benchmark is intended to exercise the
* GUPS capability of a system, much like the LINPACK benchmark is intended to
* exercise the MFLOPS capability of a computer.  In each case, we would
* expect these benchmarks to achieve close to the "peak" capability of the
* memory system. The extent of the similarities between RandomAccess and
* LINPACK are limited to both benchmarks attempting to calculate a peak system
* capability.
*
* GUPS is calculated by identifying the number of memory locations that can be
* randomly updated in one second, divided by 1 billion (1e9). The term "randomly"
* means that there is little relationship between one address to be updated and
* the next, except that they occur in the space of one half the total system
* memory.  An update is a read-modify-write operation on a table of 64-bit words.
* An address is generated, the value at that address read from memory, modified
* by an integer operation (add, and, or, xor) with a literal value, and that
* new value is written back to memory.
*
* We are interested in knowing the GUPS performance of both entire systems and
* system subcomponents --- e.g., the GUPS rating of a distributed memory
* multiprocessor the GUPS rating of an SMP node, and the GUPS rating of a
* single processor.  While there is typically a scaling of FLOPS with processor
* count, a similar phenomenon may not always occur for GUPS.
*
* For additional information on the GUPS metric, the HPCchallenge RandomAccess
* Benchmark,and the rules to run RandomAccess or modify it to optimize
* performance -- see http://icl.cs.utk.edu/hpcc/
*
*/

/*
* This file contains the computational core of the single cpu version
* of GUPS.  The inner loop should easily be vectorized by compilers
* with such support.
*
* This core is used by both the single_cpu and star_single_cpu tests.
*/
#include <sys/time.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <malloc.h>
#include "randomaccess.h"
#include "timers.h"
#include "utils.h"
#include <assert.h>

//*************************************************************|
//CONTROL PARAMS FOR VERSION SELECT                            |
//*************************************************************|
#define DEFAULT
//#define ATOMIC
//#define UDR
//#define pibor
//-------------------------------------------------------------
#define PARALLEL_INIT
//-------------------------------------------------------------

#ifdef DEFAULT
#include "kernel_default.h"
#endif

#ifdef ATOMIC
#include "kernel_atomic.h"
#endif

#ifdef UDR
#include "kernel_UDR.h"
#endif

#ifdef pibor
#include "kernel_PIBOR.h"
#endif

using namespace std;

//*************************************************************|
// MAIN                                                        |
//*************************************************************|

int main( int argc, char** argv )
{
	INT_TYPE i, *Table;
	int err;
	double cputime;               /* CPU time to update table */
	double realtime;              /* Real time to update table */
	double GUPs;

	if(argc != 4)
	{
		Usage(argc, argv);
		exit(1);
	}

	totalMem = atoi( argv[1] ); //reduction array (MB)
	NT       = atoi( argv[2] ); //number of tasks
	verify   = atoi( argv[3] ); //verify restuls
	version  = 0;
	bitSize = sizeof(INT_TYPE) * 8 ;

	/* calculate local memory per node for the update table */
	totalMem <<= 20; //convert MB to byte
	totalMem /= sizeof(INT_TYPE); //elements

	logTableSize = compute_log2 (totalMem);
	tableSize = 1 << logTableSize;

	//Table = MALLOC( INT_TYPE, tableSize );
	AL_MALLOC(&Table, 4096, INT_TYPE, tableSize);
	CHECK_NULL(Table);

	/* Initialize main table */
	init(Table);
	timer_start(0);
	err = RA(Table);
	/* End timed section */
	timer_stop(0);

	/* make sure no division by zero */
	GUPs = (timer_read(1)) > 0.0 ? 1.0 / timer_read(0) : -1.0;
	GUPs *= 1e-9*NUPDATE;
	/* Print timing results */

	fprintf( stdout, "%i, %.6f, %lld, %i, %i, %s \n",
			version,
			timer_read(0),
			/*GUPs*/
			tableSize * sizeof(INT_TYPE) / 1024 / 1024,
			omp_get_num_threads(),
			err,
			(err == 0 || err == -1) ? "passed" : "failed");
	free( Table );
	return 0;
}
