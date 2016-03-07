INT_TYPE
RA(INT_TYPE *Table) {
	version = 3;
	INT_TYPE i, j, pos, start;
	INT_TYPE block, block_mod, datablock, databock_mod;
	//defines work per task
	TILESIZE (NUPDATE, NT, block, block_mod);
	TILESIZE (tableSize, NT, datablock, databock_mod);

	//create seeds at n-th position
	INT_TYPE * ran = (INT_TYPE * ) calloc (sizeof(INT_TYPE), NT) ;
	for (j=0; j<NT; j++){
		ran[j] = HPCC_starts_LCG ( block * j);
	}

	start = 0;
	for(j = 0; j < NT; j++) {
		INT_TYPE seed = ran[j];
		if ( j == NT-1 ) { block += block_mod;}
		#pragma omp task inout (Table[start;datablock])
		{
			for( INT_TYPE i = 0; i < block; i++ ) {
				seed = LCG_MUL64 * seed + LCG_ADD64;
				pos = seed >> (bitSize - logTableSize);
				#pragma omp atomic
				Table[pos] ^= seed;
			}
		}
		start+=datablock;
	}
	#pragma omp taskwait
	return verifyResults(Table);
}
