void R ( INT_TYPE * a, INT_TYPE * b ) {*a ^= *b;}
void I ( INT_TYPE * a ) { *a = 0; }

typedef struct
{
	INT_TYPE * array;
}table_t;

void reducer(table_t * out, table_t * in)
{
	for (int i = 0; i < tableSize; i++)
		out->array[i] ^= in->array[i];

    free(in->array);
}

void init(table_t* priv, table_t * orig)
{
	priv->array = ( INT_TYPE *) malloc(sizeof(INT_TYPE) * tableSize);
    for (int i = 0; i < tableSize; i++)
    	priv->array[i] = 0;
}

#pragma omp declare reduction(myUDR :  table_t : \
		reducer(&omp_out,&omp_in)) \
		initializer(init(&omp_priv, &omp_orig))

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

	table_t table;
	table.array = Table;

	for(j = 0; j < NT; j++) {
		INT_TYPE seed = ran[j];
		if ( j == NT-1 ) block += block_mod;
		#pragma omp task reduction (myUDR:table) firstprivate(block, seed)
		{
			INT_TYPE tmp;
			for( INT_TYPE i = 0; i < block; i++ ) {
				seed = LCG_MUL64 * seed + LCG_ADD64;
				pos = seed >> (bitSize - logTableSize );
				table.array[pos] ^= seed;
			}
		}

	}
	#pragma omp taskwait
	return verifyResults(Table);
}
