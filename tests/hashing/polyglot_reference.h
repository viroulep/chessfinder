#ifndef PG_REFERENCE_H
#define PG_REFERENCE_H
#include <stdint.h>
/*typedef unsigned long long int uint64;*/
typedef uint64_t uint64;

uint64 pg_ref_hash(const char *fen);
#endif
