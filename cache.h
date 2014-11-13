/******************************************************************************
 ******************************************************************************
 * J.S. PEIRCE       jpei2@pdx.edu
 * SEAN KOPPENHAFER  koppen2@pdx.edu
 * LUIS SANTIAGO     lsantiag@pdx.edu
 *
 * 5 NOVEMBER 2014
 * ECE485 FINAL PROJECT
 * CACHE DESIGN SIMULATION PROGRAM
 * 
 *
 * cache.h General header 
 ******************************************************************************
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#ifndef _CACHE_H
#define _CACHE_H

#define WAYS 16
#define NUM_SETS 8192

/*BUS OPS:*/
#define READ 			1
#define WRITE 			2
#define INVALIDATE 	3
#define RWIM 			4

/*SNOOP RESULTS:*/
#define NOHIT 		0
#define HIT 		1
#define HITM 		2

/*TRACES: (N ADDRESS)
 *0 READ REQUEST FROM L1 FROM D CACHE
 *1 WRITE REQUEST FROM L1 FROM D CACHE
 *2 READ REQUSET FROM L1 INSTRUCTION CACHE
 *3 SNOOPED INVCALIDATE COMMAND
 *4 SNOOPED READ REQUEST
 *5 SNOOPED WRITE REQUEST
 *6 SNOOPED RFO
 *8 CLEAR CACHE AND RESET STATE (FLUSH OR EOF)
 *9 PRINT CONTENTS AND STATE OF EACH VALID LINE
 */

/* Holds all variables stored in a cache line */
typedef struct {
	uint16_t tag;
	uint8_t MESIF;
	uint8_t last_bus_op;		/* MESIF debug - our CPU accesses memory */
	uint8_t last_snoop_result;	/* MESIF debug - our CPU snooping - NOHIT, HIT or HITM */
} cache_line;

/* Holds all variables stored in a set */
typedef struct {
	uint16_t psuedo_LRU;
	uint8_t valid_ways;
	cache_line line[WAYS];
} cache_set;

#endif

