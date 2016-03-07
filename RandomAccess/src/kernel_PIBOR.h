


/*-----------------------------------------------------------------------*/
//Documentation:
//Use the following env vars for PIBOR config
//export NX_ARGS="--smp-workers=16 --enable-lazy-privatization=1"
//export PER_THREAD_STORAGE_SIZE=x (x=size in bytes)
//export NUM_REGIONS=y (y=number of regions)
//export export BUFFER_SIZE=z (z=size in bytes)
/*-----------------------------------------------------------------------*/

#include "PIBOR/pibor.h"

using namespace redompss;

typedef PIBOR <INT_TYPE, char> Pibor_t;
typedef Implementation < INT_TYPE, char> RED_t;

void R ( INT_TYPE * a, INT_TYPE * b ) {*a ^= *b;}
void I ( INT_TYPE * a ) { *a = 0; }

inline void op(INT_TYPE * adr, INT_TYPE * RHS)
{
	*adr ^= *RHS;
}

void reducer (RED_t * out, RED_t * in)
{
	in->reduce();
}

void init(RED_t * priv, RED_t * orig)
{
	Pibor_t  * p = new Pibor_t (orig, op, new Settings(Settings::AUTOSET_BUF_SIZE));
	priv->init(p, orig);
}

#pragma omp declare reduction(plus: RED_t : reducer(&omp_out,&omp_in)) initializer(init(&omp_priv, &omp_orig))

INT_TYPE
RA(INT_TYPE *Table) {
	version = 3;
	INT_TYPE i, j, pos, block, block_mod;
	//defines work per task
	TILESIZE (NUPDATE, NT, block, block_mod);

	//create seeds at n-th position
	INT_TYPE * ran = (INT_TYPE * ) calloc (sizeof(INT_TYPE), NT) ;
	for (j=0; j<NT; j++){
		ran[j] = HPCC_starts_LCG ( block * j);
	}

	//wrapper object: target address, size, op and global data storage size
	RED_t & r = * new RED_t(Table, tableSize * sizeof(INT_TYPE), 1024 * sizeof(char));

	//kernel
	for(j = 0; j < NT; j++) {
		INT_TYPE seed = ran[j];
		if ( j == NT-1 ) block += block_mod;
		#pragma omp task reduction (plus:r) firstprivate(block, seed)
		{
			INT_TYPE tmp;
			for( INT_TYPE i = 0; i < block; i++ ) {
				seed = LCG_MUL64 * seed + LCG_ADD64;
				pos = seed >> (bitSize - logTableSize );
				//Table[pos] ^= seed;

				//array update via redirection to wrapper object
				r.op(&Table[pos], &seed);
			}
		}
	}
	#pragma omp taskwait
	return verifyResults(Table);
}
