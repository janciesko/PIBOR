/* -*- mode: C; tab-width: 2; indent-tabs-mode: nil; -*-
 *
 * This file provides utility functions for the RandomAccess benchmark suite.
 */

void Error( char * s ) { fprintf(stderr,"%s\n",s); exit(1);}
void Msg  ( char * s ) { fprintf(stderr,"%s\n",s);         }
void Usage(int argc, char**argv){fprintf(stderr,"Usage: %s  <TOTALMB> <NUMTASKS> <VERIFY>\n", argv[0]);}

uint64_t compute_log2(uint64_t in){
	uint64_t out;
	for (in *= 0.5, out = 0; in >= 1.0; in *= 0.5, out++)
		;

	return out;
}


INT_TYPE verifyResults(INT_TYPE * Table)
{
	INT_TYPE i, err = 0;
	if(verify)
	{
		INT_TYPE tmp = 1;
		for (i=0; i<NUPDATE; i++) {
			tmp = LCG_MUL64 * tmp + LCG_ADD64;
			Table[tmp >> (bitSize - logTableSize)] ^= tmp;
		}

		for (i=0; i<tableSize; i++)
			if (Table[i] != i)
				err++;

	} else err = -1;
	return err;
}

void init(INT_TYPE * Table)
{
	INT_TYPE i, j;
	INT_TYPE block, block_mod, start;
	//defines work per task
	TILESIZE (tableSize, NT, block, block_mod);

	start = 0;
	for(j = 0; j < NT; j++) {
		#ifdef PARALLEL_INIT
		#pragma omp task out(Table[start;block])
		#endif
		{
			for( INT_TYPE i = start; i <  start + block; i++ )
				Table[i] = i;
		}
		start += block;
	}
	#pragma omp taskwait noflush
}


/* Utility routine to start LCG random number generator at Nth step */
u64Int
HPCC_starts_LCG(s64Int n)
{
  u64Int mul_k, add_k, ran, un;

  mul_k = LCG_MUL64;
  add_k = LCG_ADD64;

  ran = 1;
  for (un = (u64Int)n; un; un >>= 1) {
    if (un & 1)
      ran = mul_k * ran + add_k;
    add_k *= (mul_k + 1);
    mul_k *= mul_k;
  }

  return ran;
}

