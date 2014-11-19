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
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifndef _CACHE_H
#define _CACHE_H

/* General defines for all functions */
//#define SILENT
#define TEST_FILE
#define DEBUG
#define PRETTY_OUTPUT

/* Cache size definitions */
#define ADDRESS_BITS 32

/*BUS OPS:*/
#define READ 			1
#define WRITE 			2
#define INVALIDATE 	3
#define RWIM 			4
#define SNOOPING		0xFF

/*SNOOP RESULTS:*/
#define NOHIT 		0
#define HIT 		1
#define HITM 		2

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
	cache_line* line;
} cache_set;

#endif

