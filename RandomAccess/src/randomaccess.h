
/* -*- mode: C; tab-width: 2; indent-tabs-mode: nil; -*- */

/* Random number generator */

#pragma once

#ifdef LONG_IS_64BITS
#define POLY 0x0000000000000007UL
#define PERIOD 1317624576693539401L
#else
#define POLY 0x0000000000000007ULL
#define PERIOD 1317624576693539401LL
#endif

typedef unsigned long long u64Int;
typedef long long s64Int;

#define INT_TYPE u64Int

int  NT, version, sparsity, verify, logTableSize, bitSize;       //# tasks
INT_TYPE totalMem, tableSize;

uint64_t compute_log2(uint64_t in);
void init(INT_TYPE * Table);
void Usage(int argc, char**argv);
void Msg  ( char * s );
void Error ( char * s );
INT_TYPE RA(INT_TYPE *Table);
INT_TYPE verifyResults(INT_TYPE * Table);

#define NUPDATE (4 * tableSize)
#define TILESIZE( size, NT, block, block_mod ) block = (size) / (NT); \
		block_mod = (size) % (block);
#define CHECK_NULL( _check ) if ((_check)==NULL) Error("Null pointer allocating memory");
#define MALLOC(t,s) ((t*)malloc (sizeof(t)*(s)))
#define AL_MALLOC(p,a,t,s) (posix_memalign ((void**) p, a, sizeof(t)*(s)))


extern u64Int HPCC_starts (s64Int);
extern u64Int HPCC_starts_LCG (s64Int);


#define USE_MULTIPLE_RECV 1

#define LCG_MUL64 6364136223846793005ULL
#define LCG_ADD64 1
