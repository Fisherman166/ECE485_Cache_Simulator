/******************************************************************************
 ******************************************************************************
 * J.S. PEIRCE       jpei2@pdx.edu
 * SEAN KOPPENHAFER  koppen2@pdx.edu
 * LUIS SANTIAGO     lsantiag@pdx.edu
 *
 * 9 NOVEMBER 2014
 * ECE485 FINAL PROJECT
 * 
 *	MESIF.c
 ******************************************************************************
 *****************************************************************************/

#include "MESIF.h"
#define DEBUG

/* Used to simulate a bus operation and to capture the snoop results of the last
** level caches of other processors
*/
uint8_t bus_operation(uint8_t bus_op, uint32_t address) {
	uint8_t snoop_result = get_snoop_result(address);
	char bus_op_text[10];
	char snoop_text[5];

	#ifndef SLIENT
	if( bus_op == READ ) strcpy(bus_op_text, "READ");
	else if( bus_op == WRITE ) strcpy(bus_op_text, "WRITE");
	else if( bus_op == INVALIDATE ) strcpy(bus_op_text, "INVALIDATE");
	else if( bus_op == RWIM ) strcpy(bus_op_text, "RWIM");
	else strcpy(bus_op_text, "ERROR");

	if( snoop_result == NOHIT ) strcpy(snoop_text, "NOHIT");
	else if( snoop_result == HIT ) strcpy(snoop_text, "HIT");
	else if( snoop_result == HITM ) strcpy(snoop_text, "HITM");
	else strcpy(snoop_text, "ERROR");

	printf("Bus OP: %s, Address: %X, Snoop Result: %s\n", bus_op_text, address, snoop_text);
	#endif

	return snoop_result;
}

/* Simulate snoop results from other processors. 
**	Use low order two bits to determine which snoop
** result to return
**	00 = NOHIT, 01 = HIT, 10 = HITM, 11 = UNUSED
*/
uint8_t get_snoop_result(uint32_t address) {
	uint8_t low_order_bits = address & 0x3;
	uint8_t retval;

	if( low_order_bits == 0x2 ) {
		retval = HITM;
	}
	else if( low_order_bits == 0x1 ) {
		retval = HIT;
	}
	else {
		retval = NOHIT;
	}

	return retval;
}
		
/* Used to report the result of our snooping bus operations by other caches */
void put_snoop_result(uint32_t address, uint8_t snoop_result) {
	char snoop_text[5];

	#ifndef SILENT
	if( snoop_result == NOHIT ) strcpy(snoop_text, "NOHIT");
	else if( snoop_result == HIT ) strcpy(snoop_text, "HIT");
	else if( snoop_result == HITM ) strcpy(snoop_text, "HITM");
	else strcpy(snoop_text, "ERROR");

	printf("Address: %X, Snoop Result: %s\n", address, snoop_text);
	#endif
}

/* Used to simulate our processor accessing the cache.
** Changes the MESIF state accordingly.
** CPU_op can be 0 or 1.  0 = CPU read. 1 = CPU write
*/
void CPU_operation(uint8_t CPU_op, uint32_t address, cache_line* line) {
	uint8_t snoop_result, bus_op; 
	uint8_t MESIF_state = line->MESIF;

	#ifdef DEBUG
	if(MESIF_state == INVALID) printf("Initial MESIF state: INVALID\n");
	else if(MESIF_state == EXCLUSIVE) printf("Initial MESIF state: EXCLUSIVE\n");
	else if(MESIF_state == SHARED) printf("Initial MESIF state: SHARED\n");
	else if(MESIF_state == FORWARD) printf("Initial MESIF state: FORWARD\n");
	else if(MESIF_state == MODIFIED) printf("Initial MESIF state: MODIFIED\n");
	else printf("Initial MESIF state: ERROR\n");
	#endif

	/* Generate the bus operation */
	if( CPU_op == CPU_READ ) bus_op = READ;
	else bus_op = RWIM;

	snoop_result = bus_operation(bus_op, address);

	/* Modify the MESIF state for the line */
	switch( MESIF_state ) {
		case INVALID:
			if( CPU_op == CPU_READ ) {
				if( snoop_result == HIT ) MESIF_state = FORWARD;
				else MESIF_state = EXCLUSIVE;
			}
			else {
				MESIF_state = MODIFIED;
			}
			break;

		case FORWARD:
			if( CPU_op == CPU_WRITE ) {
				MESIF_state = MODIFIED;
			}
			break;

		case SHARED:
			if( CPU_op == CPU_WRITE ) {
				MESIF_state = MODIFIED;
			}
			break;

		case EXCLUSIVE:
			if( CPU_op == CPU_WRITE ) {
				MESIF_state = MODIFIED;
			}
			break;

		case MODIFIED:
			MESIF_state = MODIFIED;
			break;
	}

	#ifdef DEBUG
	if(MESIF_state == INVALID) printf("Modified MESIF state: INVALID\n");
	else if(MESIF_state == EXCLUSIVE) printf("Modified MESIF state: EXCLUSIVE\n");
	else if(MESIF_state == SHARED) printf("Modified MESIF state: SHARED\n");
	else if(MESIF_state == FORWARD) printf("Modified MESIF state: FORWARD\n");
	else if(MESIF_state == MODIFIED) printf("Modified MESIF state: MODIFIED\n");
	else printf("Modified MESIF state: ERROR\n");
	#endif

	line->MESIF = MESIF_state;
}

/* Used to simulate our processor snooping another processors cache
** access.
*/
void other_CPU_operation(uint8_t bus_op, uint32_t address, cache_line* line) {
	uint8_t snoop_result;
	uint8_t MESIF_state = line->MESIF;

	#ifdef DEBUG
	if(MESIF_state == INVALID) printf("Initial MESIF state: INVALID\n");
	else if(MESIF_state == EXCLUSIVE) printf("Initial MESIF state: EXCLUSIVE\n");
	else if(MESIF_state == SHARED) printf("Initial MESIF state: SHARED\n");
	else if(MESIF_state == FORWARD) printf("Initial MESIF state: FORWARD\n");
	else if(MESIF_state == MODIFIED) printf("Initial MESIF state: MODIFIED\n");
	else printf("Initial MESIF state: ERROR\n");
	#endif

	switch( MESIF_state ) {
		case INVALID:
			snoop_result = NOHIT;
			break;

		case FORWARD:
			if( (bus_op == RWIM) || (bus_op == INVALIDATE) ) {
				MESIF_state = INVALID;
				snoop_result = HIT;
			}
			else if( bus_op == READ ) {
				MESIF_state = SHARED;
				snoop_result = HIT;
			}
			break;

		case SHARED:
			snoop_result = HIT;
			if( (bus_op == RWIM) || (bus_op == INVALIDATE) ) {
				MESIF_state = INVALID;
			}
			break;

		case EXCLUSIVE:
			if( (bus_op == RWIM) || (bus_op == INVALIDATE) ) {
				MESIF_state = INVALID;
				snoop_result = HIT;
			}
			else if( bus_op == READ ) {
				MESIF_state = SHARED;
				snoop_result = HIT;
			}
			break;

		case MODIFIED:
			if( (bus_op == RWIM) || (bus_op == INVALIDATE) ) {
				MESIF_state = INVALID;
				snoop_result = HITM;
			}
			else if( bus_op == READ ) {
				MESIF_state = SHARED;
				snoop_result = HITM;
			}
			break;
	}

	put_snoop_result(address, snoop_result);

	#ifdef DEBUG
	if(MESIF_state == INVALID) printf("Modified MESIF state: INVALID\n");
	else if(MESIF_state == EXCLUSIVE) printf("Modified MESIF state: EXCLUSIVE\n");
	else if(MESIF_state == SHARED) printf("Modified MESIF state: SHARED\n");
	else if(MESIF_state == FORWARD) printf("Modified MESIF state: FORWARD\n");
	else if(MESIF_state == MODIFIED) printf("Modified MESIF state: MODIFIED\n");
	else printf("Modified MESIF state: ERROR\n");
	#endif
}


/* To test the MESIF functionality by itself */
int main() {
	cache_line line;

	line.MESIF = INVALID;
	line.tag = 0;

	/* Check our CPU doing operations */
	/* I -> E -> M -> I */
	printf("Invalid to exclusive\n");
	CPU_operation(CPU_READ, 0x0, &line);
	printf("\n\n");

	printf("Read in exclusive \n");
	CPU_operation(CPU_READ, 0x0, &line);
	printf("\n\n");

	printf("Exclusive to modified \n");
	CPU_operation(CPU_WRITE, 0x0, &line);
	printf("\n\n");

	printf("Read in modified\n");
	CPU_operation(CPU_READ, 0x0, &line);
	printf("\n\n");

	printf("Write in modified \n");
	CPU_operation(CPU_WRITE, 0x0, &line);
	printf("\n\n");

	printf("M -> I\n");
	other_CPU_operation(RWIM, 0x0, &line);
	printf("\n\n");

	return 0;
}

