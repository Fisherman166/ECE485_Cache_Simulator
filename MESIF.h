/******************************************************************************
 ******************************************************************************
 * J.S. PEIRCE       jpei2@pdx.edu
 * SEAN KOPPENHAFER  koppen2@pdx.edu
 * LUIS SANTIAGO     lsantiag@pdx.edu
 *
 * 9 NOVEMBER 2014
 * ECE485 FINAL PROJECT
 * 
 *	MESIF.h
 ******************************************************************************
 *****************************************************************************/

#include "cache.h"
#include <string.h>

 #ifndef MESIF_H
 #define MESIF_H

/* Define the MESIF states */
#define INVALID 0
#define EXCLUSIVE 1
#define SHARED 2
#define FORWARD 3
#define MODIFIED 4

/* Define CPU operations */
#define CPU_READ 0
#define CPU_WRITE 1

/* Used to simulate a bus operation and to capture the snoop results of the last
** level caches of other processors
*/
uint8_t bus_operation(uint8_t bus_op, uint32_t address);

/* Simulate snoop results from other processors */
uint8_t get_snoop_result(uint32_t address);

/* Return our snoop result from another processors operation */
void put_snoop_result(uint32_t address, uint8_t snoop_result);

/* Used to simulate our processor accessing the cache.
** Changes the MESIF state accordingly
** CPU_op can be 0 or 1.  0 = CPU read. 1 = CPU write
*/
void CPU_operation(uint8_t CPU_op, uint32_t address, cache_line* line);

/* Used to simulate our processor snooping another processors cache
** access.
*/
void other_CPU_operation(uint8_t bus_op, uint32_t address, cache_line* line);

#endif
