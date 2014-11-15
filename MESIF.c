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
** snoop_result = 0xFF when our processor is snooping and it performs a bus operation
*/
void bus_operation(uint8_t bus_op, uint32_t address, uint8_t snoop_result) {
	char bus_op_text[10];
	char snoop_text[5];

	#ifndef SLIENT
	if( bus_op == READ ) strcpy(bus_op_text, "READ");
	else if( bus_op == WRITE ) strcpy(bus_op_text, "WRITE");
	else if( bus_op == INVALIDATE ) strcpy(bus_op_text, "INVALIDATE");
	else if( bus_op == RWIM ) strcpy(bus_op_text, "RWIM");
	else strcpy(bus_op_text, "ERROR");

	/* When our processor performs a bus_op while snooping, it passes 0xFF to snoop_result */
	if(snoop_result == 0xFF) {
		printf("Bus OP: %s, Address: %X\n", bus_op_text, address);
	}
	else {
		if( snoop_result == NOHIT ) strcpy(snoop_text, "NOHIT");
		else if( snoop_result == HIT ) strcpy(snoop_text, "HIT");
		else if( snoop_result == HITM ) strcpy(snoop_text, "HITM");
		else strcpy(snoop_text, "ERROR");

		printf("Bus OP: %s, Address: %X, Snoop Result: %s\n", bus_op_text, address, snoop_text);
	}
	#endif
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
	uint8_t snoop_result = get_snoop_result(address);
	uint8_t bus_op = 0xFF;	/* When no bus_op is set for the transistion */ 
	uint8_t MESIF_state = line->MESIF;

	#ifdef DEBUG
	if(MESIF_state == INVALID) printf("Initial MESIF state: INVALID\n");
	else if(MESIF_state == EXCLUSIVE) printf("Initial MESIF state: EXCLUSIVE\n");
	else if(MESIF_state == SHARED) printf("Initial MESIF state: SHARED\n");
	else if(MESIF_state == FORWARD) printf("Initial MESIF state: FORWARD\n");
	else if(MESIF_state == MODIFIED) printf("Initial MESIF state: MODIFIED\n");
	else printf("Initial MESIF state: ERROR\n");
	#endif

	/* Modify the MESIF state for the line */
	switch( MESIF_state ) {
		case INVALID:
			if( CPU_op == CPU_READ ) {
				bus_op = READ;
				bus_operation(bus_op, address, snoop_result);

				/* Hit goes high when HITM goes high */
				if( (snoop_result == HIT) || (snoop_result == HITM) ) MESIF_state = FORWARD;
				else MESIF_state = EXCLUSIVE;
			}
			else {
				bus_op = RWIM;
				bus_operation(bus_op, address, snoop_result);
				MESIF_state = MODIFIED;
			}
			break;

		case FORWARD:
			if( CPU_op == CPU_WRITE ) {
				bus_op = INVALIDATE;
				bus_operation(bus_op, address, snoop_result);
				MESIF_state = MODIFIED;
			}
			break;

		case SHARED:
			if( CPU_op == CPU_WRITE ) {
				bus_op = INVALIDATE;
				bus_operation(bus_op, address, snoop_result);
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

	/* Update data in the cache line */
	line->MESIF = MESIF_state;
	line->last_bus_op = bus_op;
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
			if( bus_op == RWIM ) {
				MESIF_state = INVALID;
				snoop_result = HIT;
				message_to_L2_cache(READ, address);	/* Update our value in cache before we forward */
				bus_operation(WRITE, address, SNOOPING);
				message_to_L2_cache(INVALIDATE, address);
			}
			else if( bus_op == INVALIDATE ) {
				MESIF_state = INVALID;
				snoop_result = NOHIT;
				message_to_L2_cache(INVALIDATE, address);	
			}
			else if( bus_op == READ ) {
				MESIF_state = SHARED;
				snoop_result = HIT;
				message_to_L2_cache(READ, address);	/* Update our value in cache before we forward */
				bus_operation(WRITE, address, SNOOPING);
			}
			break;

		case SHARED:
			snoop_result = HIT;
			if( (bus_op == RWIM) || (bus_op == INVALIDATE) ) {
				MESIF_state = INVALID;
				message_to_L2_cache(INVALIDATE, address);	
			}
			break;

		case EXCLUSIVE:
			if( bus_op == RWIM ) {
				MESIF_state = INVALID;
				snoop_result = HIT;
				message_to_L2_cache(READ, address);	
				bus_operation(WRITE, address, SNOOPING);
				message_to_L2_cache(INVALIDATE, address);	
			}
			else if( bus_op == INVALIDATE ) {
				MESIF_state = INVALID;
				snoop_result = NOHIT;
				message_to_L2_cache(INVALIDATE, address);	
			}
			else if( bus_op == READ ) {
				MESIF_state = SHARED;
				snoop_result = HIT;
				message_to_L2_cache(READ, address);	
				bus_operation(WRITE, address, SNOOPING);
			}
			break;

		case MODIFIED:
			if( (bus_op == RWIM) || (bus_op == INVALIDATE) ) {
				MESIF_state = INVALID;
				snoop_result = HITM;
				message_to_L2_cache(READ, address);	
				bus_operation(WRITE, address, SNOOPING);
				message_to_L2_cache(INVALIDATE, address);	
			}
			else if( bus_op == READ ) {
				MESIF_state = SHARED;
				snoop_result = HITM;
				message_to_L2_cache(READ, address);	
				bus_operation(WRITE, address, SNOOPING);
			}
			break;
	}

	put_snoop_result(address, snoop_result);
	line->MESIF = MESIF_state;
	line->last_snoop_result = snoop_result;

	#ifdef DEBUG
	if(MESIF_state == INVALID) printf("Modified MESIF state: INVALID\n");
	else if(MESIF_state == EXCLUSIVE) printf("Modified MESIF state: EXCLUSIVE\n");
	else if(MESIF_state == SHARED) printf("Modified MESIF state: SHARED\n");
	else if(MESIF_state == FORWARD) printf("Modified MESIF state: FORWARD\n");
	else if(MESIF_state == MODIFIED) printf("Modified MESIF state: MODIFIED\n");
	else printf("Modified MESIF state: ERROR\n");
	#endif
}

/* Used to pass a message to the L2 cache */
void message_to_L2_cache( uint8_t bus_op, uint32_t address) {
	char bus_op_text[10];

	#ifndef SLIENT
		if( bus_op == READ ) strcpy(bus_op_text, "READ");
		else if( bus_op == WRITE ) strcpy(bus_op_text, "WRITE");
		else if( bus_op == INVALIDATE ) strcpy(bus_op_text, "INVALIDATE");
		else if( bus_op == RWIM ) strcpy(bus_op_text, "RWIM");
		else strcpy(bus_op_text, "ERROR");

		printf("L2 bus_op: %s, Address: %X\n", bus_op_text, address);
	#endif
}

/********************************************************************************
** DEBUG FUNCTIONS
*******************************************************************************/
/* Test F states in our CPU */
void F_CPU(cache_line line) {
	line.MESIF = FORWARD;
	printf("***********************************************************************************\n");
	
	/* Test all read conditions in F */
	printf("\nRead in F, NOHIT\n");
	CPU_operation(CPU_READ, 0x0, &line);
	assert( line.MESIF == FORWARD );

	printf("\nRead in F, HIT\n");
	CPU_operation(CPU_READ, 0x1, &line);
	assert( line.MESIF == FORWARD );

	printf("\nRead in F, HITM\n");
	CPU_operation(CPU_READ, 0x2, &line);
	assert( line.MESIF == FORWARD );

	/* Test F -> M with all three hit possibilities */
	printf("\nF -> M, NOHIT\n");
	CPU_operation(CPU_WRITE, 0x0, &line);
	assert( line.MESIF == MODIFIED );
	assert( line.last_bus_op == INVALIDATE );

	line.MESIF = FORWARD;
	printf("\nF -> M, HIT\n");
	CPU_operation(CPU_WRITE, 0x1, &line);
	assert( line.MESIF == MODIFIED );
	assert( line.last_bus_op == INVALIDATE );

	line.MESIF = FORWARD;
	printf("\nF -> M, HITM\n");
	CPU_operation(CPU_WRITE, 0x2, &line);
	assert( line.MESIF == MODIFIED );
	assert( line.last_bus_op == INVALIDATE );
}

/* Test E states in our CPU */
void E_CPU(cache_line line) {
	line.MESIF = EXCLUSIVE;
	printf("***********************************************************************************\n");

	/* Test all read conditions in E */
	printf("\nRead in E, NOHIT\n");
	CPU_operation(CPU_READ, 0x0, &line);
	assert( line.MESIF == EXCLUSIVE );

	printf("\nRead in E, HIT\n");
	CPU_operation(CPU_READ, 0x1, &line);
	assert( line.MESIF == EXCLUSIVE );

	printf("\nRead in E, HITM\n");
	CPU_operation(CPU_READ, 0x2, &line);
	assert( line.MESIF == EXCLUSIVE );

	/* Test E.M with all three hit possibilities */
	printf("\nE -> M, NOHIT\n");
	CPU_operation(CPU_WRITE, 0x0, &line);
	assert( line.MESIF == MODIFIED );

	line.MESIF = EXCLUSIVE;
	printf("\nE -> M, HIT\n");
	CPU_operation(CPU_WRITE, 0x1, &line);
	assert( line.MESIF == MODIFIED );

	line.MESIF = EXCLUSIVE;
	printf("\nE -> M, HITM\n");
	CPU_operation(CPU_WRITE, 0x2, &line);
	assert( line.MESIF == MODIFIED );
}

/* Test S states in our CPU */
void S_CPU(cache_line line) {
	line.MESIF = SHARED;
	printf("***********************************************************************************\n");

	/* Test all read conditions in E */
	printf("\nRead in S, NOHIT\n");
	CPU_operation(CPU_READ, 0x0, &line);
	assert( line.MESIF == SHARED );

	printf("\nRead in S, HIT\n");
	CPU_operation(CPU_READ, 0x1, &line);
	assert( line.MESIF == SHARED );

	printf("\nRead in S, HITM\n");
	CPU_operation(CPU_READ, 0x2, &line);
	assert( line.MESIF == SHARED );

	/* Test E.M with all three hit possibilities */
	printf("\nS -> M, NOHIT\n");
	CPU_operation(CPU_WRITE, 0x0, &line);
	assert( line.MESIF == MODIFIED );
	assert( line.last_bus_op == INVALIDATE );

	line.MESIF = SHARED;
	printf("\nS -> M, HIT\n");
	CPU_operation(CPU_WRITE, 0x1, &line);
	assert( line.MESIF == MODIFIED );
	assert( line.last_bus_op == INVALIDATE );

	line.MESIF = SHARED;
	printf("\nS -> M, HITM\n");
	CPU_operation(CPU_WRITE, 0x2, &line);
	assert( line.MESIF == MODIFIED );
	assert( line.last_bus_op == INVALIDATE );
}

/* Test I states in our CPU */
void I_CPU(cache_line line) {
	line.MESIF = INVALID;
	printf("***********************************************************************************\n");

	/* Check all reads */
	printf("\nI -> E, NOHIT\n");
	CPU_operation(CPU_READ, 0x0, &line);
	assert( line.MESIF == EXCLUSIVE );
	assert( line.last_bus_op == READ );
	
	line.MESIF = INVALID;
	printf("\nI -> F, HIT\n");
	CPU_operation(CPU_READ, 0x1, &line);
	assert( line.MESIF == FORWARD );
	assert( line.last_bus_op == READ );

	line.MESIF = INVALID;
	printf("\nI -> F, HITM\n");
	CPU_operation(CPU_READ, 0x2, &line);
	assert( line.MESIF == FORWARD );
	assert( line.last_bus_op == READ );

	/* Check all writes */
	line.MESIF = INVALID;
	printf("\nI -> M, NOHIT\n");
	CPU_operation(CPU_WRITE, 0x0, &line);
	assert( line.MESIF == MODIFIED );
	assert( line.last_bus_op == RWIM );

	line.MESIF = INVALID;
	printf("\nI -> M, HIT\n");
	CPU_operation(CPU_WRITE, 0x1, &line);
	assert( line.MESIF == MODIFIED );
	assert( line.last_bus_op == RWIM );

	line.MESIF = INVALID;
	printf("\nI -> M, HITM\n");
	CPU_operation(CPU_WRITE, 0x2, &line);
	assert( line.MESIF == MODIFIED );
	assert( line.last_bus_op == RWIM );
}

/* Test M states in our CPU */
void M_CPU(cache_line line) {
	line.MESIF = MODIFIED;
	printf("***********************************************************************************\n");

	/* Check all reads */
	printf("\nREAD in M, NOHIT\n");
	CPU_operation(CPU_READ, 0x0, &line);
	assert( line.MESIF == MODIFIED );

	printf("\nREAD in M, HIT\n");
	CPU_operation(CPU_READ, 0x1, &line);
	assert( line.MESIF == MODIFIED );

	printf("\nREAD in M, HITM\n");
	CPU_operation(CPU_READ, 0x2, &line);
	assert( line.MESIF == MODIFIED );
	
	/* Check all writes */
	printf("\nWRITE in M, NOHIT\n");
	CPU_operation(CPU_WRITE, 0x0, &line);
	assert( line.MESIF == MODIFIED );

	printf("\nWRITE in M, HIT\n");
	CPU_operation(CPU_WRITE, 0x1, &line);
	assert( line.MESIF == MODIFIED );

	printf("\nWRITE in M, HITM\n");
	CPU_operation(CPU_WRITE, 0x2, &line);
	assert( line.MESIF == MODIFIED );	
}

/* Test our CPU snooping in forward */
void F_SNOOP(cache_line line) {
	line.MESIF = FORWARD;
	printf("***********************************************************************************\n");

	/* Test all 4 bus operation that could be snooped */
	printf("\nSnoop READ in F\n");
	other_CPU_operation(READ, 0x0, &line);
	assert( line.MESIF == SHARED );	
	assert( line.last_snoop_result == HIT );

	line.MESIF = FORWARD;
	printf("\nSnoop WRITE in F\n");
	other_CPU_operation(WRITE, 0x0, &line);
	assert( line.MESIF == FORWARD );

	line.MESIF = FORWARD;
	printf("\nSnoop RWIM in F\n");
	other_CPU_operation(RWIM, 0x0, &line);
	assert( line.MESIF == INVALID );	
	assert( line.last_snoop_result == HIT );

	line.MESIF = FORWARD;
	printf("\nSnoop INVALIDATE in F\n");
	other_CPU_operation(INVALIDATE, 0x0, &line);
	assert( line.MESIF == INVALID );	
	assert( line.last_snoop_result == NOHIT );
}

/* Test our CPU snooping in exclusive */
void E_SNOOP(cache_line line) {
	line.MESIF = EXCLUSIVE;
	printf("***********************************************************************************\n");

	/* Test all 4 bus operation that could be snooped */
	printf("\nSnoop READ in E\n");
	other_CPU_operation(READ, 0x0, &line);
	assert( line.MESIF == SHARED );	
	assert( line.last_snoop_result == HIT );

	line.MESIF = EXCLUSIVE;
	printf("\nSnoop WRITE in E\n");
	other_CPU_operation(WRITE, 0x0, &line);
	assert( line.MESIF == EXCLUSIVE );

	line.MESIF = EXCLUSIVE;
	printf("\nSnoop RWIM in E\n");
	other_CPU_operation(RWIM, 0x0, &line);
	assert( line.MESIF == INVALID );	
	assert( line.last_snoop_result == HIT );

	line.MESIF = EXCLUSIVE;
	printf("\nSnoop INVALIDATE in E\n");
	other_CPU_operation(INVALIDATE, 0x0, &line);
	assert( line.MESIF == INVALID );	
	assert( line.last_snoop_result == NOHIT );
}

/* Test our CPU snooping in shared */
void S_SNOOP(cache_line line) {
	line.MESIF = SHARED;
	printf("***********************************************************************************\n");

	/* Test all 4 bus operation that could be snooped */
	printf("\nSnoop READ in S\n");
	other_CPU_operation(READ, 0x0, &line);
	assert( line.MESIF == SHARED );	

	line.MESIF = SHARED;
	printf("\nSnoop WRITE in S\n");
	other_CPU_operation(WRITE, 0x0, &line);
	assert( line.MESIF == SHARED );

	line.MESIF = SHARED;
	printf("\nSnoop RWIM in S\n");
	other_CPU_operation(RWIM, 0x0, &line);
	assert( line.MESIF == INVALID );	

	line.MESIF = SHARED;
	printf("\nSnoop INVALIDATE in S\n");
	other_CPU_operation(INVALIDATE, 0x0, &line);
	assert( line.MESIF == INVALID );	
}

/* Test our CPU snooping in shared */
void I_SNOOP(cache_line line) {
	line.MESIF = INVALID;
	printf("***********************************************************************************\n");

	/* Test all 4 bus operation that could be snooped */
	printf("\nSnoop READ in I\n");
	other_CPU_operation(READ, 0x0, &line);
	assert( line.MESIF == INVALID );	

	line.MESIF = INVALID;
	printf("\nSnoop WRITE in I\n");
	other_CPU_operation(WRITE, 0x0, &line);
	assert( line.MESIF == INVALID );

	line.MESIF = INVALID;
	printf("\nSnoop RWIM in I\n");
	other_CPU_operation(RWIM, 0x0, &line);
	assert( line.MESIF == INVALID );	

	line.MESIF = INVALID;
	printf("\nSnoop INVALIDATE in I\n");
	other_CPU_operation(INVALIDATE, 0x0, &line);
	assert( line.MESIF == INVALID );	
}

/* Test our CPU snooping in forward */
void M_SNOOP(cache_line line) {
	line.MESIF = MODIFIED;
	printf("***********************************************************************************\n");

	/* Test all 4 bus operation that could be snooped */
	printf("\nSnoop READ in M\n");
	other_CPU_operation(READ, 0x0, &line);
	assert( line.MESIF == SHARED );	
	assert( line.last_snoop_result == HITM );

	line.MESIF = MODIFIED;
	printf("\nSnoop WRITE in M\n");
	other_CPU_operation(WRITE, 0x0, &line);
	assert( line.MESIF == MODIFIED );

	line.MESIF = MODIFIED;
	printf("\nSnoop RWIM in M\n");
	other_CPU_operation(RWIM, 0x0, &line);
	assert( line.MESIF == INVALID );	
	assert( line.last_snoop_result == HITM );

	line.MESIF = MODIFIED;
	printf("\nSnoop INVALIDATE in M\n");
	other_CPU_operation(INVALIDATE, 0x0, &line);
	assert( line.MESIF == INVALID );	
}

/* MESIF unit test - can uncomment for debug */
//int main() {
//	cache_line line;
//
//	line.MESIF = INVALID;
//	line.tag = 0;
//
//	/* Check our CPU doing operations */
//	printf("***********************************************************************************\n");
//	printf("Our CPU accessing memory\n");
//	printf("***********************************************************************************\n");
//	F_CPU(line);	
//	E_CPU(line);
//	S_CPU(line);
//	I_CPU(line);
//	M_CPU(line);
//
//	/* Check us snooping other CPUs */
//	printf("\n\n***********************************************************************************\n");
//	printf("Our CPU snooping\n");
//	printf("***********************************************************************************\n");
//	F_SNOOP(line);	
//	E_SNOOP(line);
//	S_SNOOP(line);
//	I_SNOOP(line);
//	M_SNOOP(line);
//
//	return 0;
//}

