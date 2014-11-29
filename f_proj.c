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
 * -MESIF COHERENCE PROTOCOL
 * -PSEUDO LRU
 ******************************************************************************
 *****************************************************************************/

#include "MESIF.h"
#include "cache.h"
#include "pseudolru.h"

/* Defines for write buffer */
#define WB_SIZE 10		//Defines the size of the posted write buffer
#define WB_COUNT 5		//Number of traces before the write buffer writes to memory
#define SLOT_VALID 1
#define SLOT_INVALID 0

/*TRACE RESULTS:*/
#define READ_DATA_L1  0
#define WRITE_DATA_L1 1
#define READ_INS_L1   2
#define SNOOP_INV     3
#define SNOOP_READ    4
#define SNOOP_WRITE   5
#define SNOOP_RWIM    6
#define CLEAR_RESET   8
#define PRINT_STATES  9

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

typedef struct {
	uint8_t operation;
	uint32_t address;
} trace_line;

typedef struct {
	uint32_t buffer_slot[WB_SIZE];
	uint8_t slot_valid[WB_SIZE];
	uint8_t head, tail, full;
	uint8_t trace_counter;
	uint8_t tag_index;		/* The index of the most recenetly matched tag */
	uint32_t truncate_mask;
} posted_write_buffer;

/* Trace related functions */
void decode_trace(uint8_t trace_op, uint32_t address);
uint8_t check_tags(uint32_t tag, cache_set* set);
void init_cache(void);
void reset_cache(void);
void print_cache(void);
void print_statistics(void);
void cleanup(FILE*);

/* CPU reads and writes */
void L1_read_or_write(uint8_t CPU_op, uint32_t tag, uint32_t address, uint8_t tag_matched_line, cache_set* indexed_set);
void fill_invalid_line(uint8_t CPU_op, uint32_t tag, uint32_t address, cache_set* set);
void fill_valid_line(uint8_t CPU_op, uint32_t tag, uint32_t address, cache_set* set);

/* Functions that break down the address */
uint32_t extract_byte_select(uint32_t address);
uint32_t extract_index(uint32_t address);
uint32_t extract_tag(uint32_t address);

/* Posted Write Buffer */
posted_write_buffer write_buffer;
void add_to_write_buffer(uint32_t address);
uint8_t search_write_buffer(uint32_t address);
void writeback_line_write_buffer(void);
void clear_write_buffer(void);

/* Global values */
cache_set *sets;
uint32_t NUM_SETS; 
uint32_t WAYS;
uint32_t BYTES_PER_LINE;
uint32_t TAG_BITS; 
uint32_t INDEX_BITS; 
uint32_t BYTE_SELECT_BITS; 

/* Cache statistics */
static uint32_t cache_reads;
static uint32_t cache_writes;
static uint32_t cache_hits;
static uint32_t cache_misses;


/******************************************************************************
 * 	MAIN LOOP: 
 * 	-GET NEXT LINE OF TRACE FILE
 * 	-SPLIT LINE INTO PARTS
 * 	-COMPARE TAG
 * 	-UPDATE STATUS
 *****************************************************************************/

int main (int argc, char * argv[])
{
	const uint8_t buffer_len = 20;
	FILE *trace_file;
	trace_line trace;
	time_t t;
	srand(time(&t));
	char buffer[buffer_len];
	char *operation_ptr, *address_ptr;
	char operation_text[30];

	/* Get cache specifications from user */
	printf("Enter the number of sets in the cache (must be a power of 2): ");
	scanf("%u", &NUM_SETS);
	printf("Enter the number of ways in the cache (must be power of 2 less than 128): ");
	scanf("%u", &WAYS);
	printf("Enter the number of bytes in a line (must be a power of 2): ");
	scanf("%u", &BYTES_PER_LINE);

	/* Calculate cache variables from user input */
	BYTE_SELECT_BITS = log(BYTES_PER_LINE) / log(2);
	INDEX_BITS = log(NUM_SETS) / log(2);
	TAG_BITS = ADDRESS_BITS - (BYTE_SELECT_BITS + INDEX_BITS);

	#ifdef DEBUG
	printf("\nByte select bits = %u\n", BYTE_SELECT_BITS);
	printf("Index bits = %u\n", INDEX_BITS);
	printf("Tag bits = %u\n\n", TAG_BITS);
	#endif

	/* Check for an illegal cache configuration */
	if( (BYTE_SELECT_BITS + INDEX_BITS) >= ADDRESS_BITS ) {
		printf("The entered cache configuration is too large to fit within a 32 bit address.\n");
		exit(-2);
	}

	#ifdef TEST_FILE
		if(argc>1){
			trace_file = fopen(argv[1], "r");
		}else{
			printf("\nplease specify trace file: \"./a.out <file name> \"\n\n");
			exit(-1);
		}/*end else*/
	#else
		trace_file = fopen("cc1.din", "r");
	#endif

	if(trace_file == NULL) {
		printf("Could not open the specified file\n");
		exit(-1);
	}/*end if*/

	/* If all checks pass above, allocate the sets */
	sets = (cache_set*)malloc(sizeof(cache_set) * NUM_SETS);	//Malloc however many sets we need
	init_cache();

//	#ifdef WARM_CACHE
//				sets[set_index].line[line_index].MESIF = (rand()%4);
//				sets[set_index].line[line_index].tag = (rand()%exp(16,32));
//	#endif

	/* Get and decode each operation in the trace file */
	while( fgets(buffer, buffer_len, trace_file) != NULL ) {
		if( feof(trace_file) ) { /* EOF has been reached */
			break;
		}
		else {
			operation_ptr = strtok(buffer, " ");	//Look for the space
			address_ptr = operation_ptr + 2;	//The address is two characters higher

			trace.operation = (uint8_t)atoi(operation_ptr);
			trace.address = (uint32_t)strtol(address_ptr, NULL, 16);	//Convert hex string to int

			#ifdef DEBUG
			#ifdef PRETTY_OUTPUT
				if(trace.operation == READ_DATA_L1) strcpy(operation_text, "READ_DATA_L1");
				else if(trace.operation == WRITE_DATA_L1) strcpy(operation_text, "WRITE_DATA_L1");
				else if(trace.operation == READ_INS_L1) strcpy(operation_text, "READ_INS_L1");
				else if(trace.operation == SNOOP_INV) strcpy(operation_text, "SNOOP_INV");
				else if(trace.operation == SNOOP_READ) strcpy(operation_text, "SNOOP_READ");
				else if(trace.operation == SNOOP_WRITE) strcpy(operation_text, "SNOOP_WRITE");
				else if(trace.operation == SNOOP_RWIM) strcpy(operation_text, "SNOOP_RWIM");
				else if(trace.operation == CLEAR_RESET) strcpy(operation_text, "CLEAR_RESET");
				else if(trace.operation == PRINT_STATES) strcpy(operation_text, "PRINT_STATES");
				else strcpy(operation_text, "ERROR");
				printf("Trace op = %s, Trace address = 0x%X\n", operation_text, trace.address);
			#else
				printf("Trace op = %d, Trace address = 0x%X\n", trace.operation, trace.address);
			#endif
			#endif

			decode_trace(trace.operation, trace.address);
			#ifdef DEBUG
				printf("\n");
			#endif			
		}
	}

	/* Cleanup */
	cleanup(trace_file);
		
	return 0;
}/*END MAIN*/


/******************************************************************************
* FUNCTION TO DECODE TRACE OPERATION
******************************************************************************/

/* Send in the trace op and the address */
void decode_trace(uint8_t trace_op, uint32_t address) {
	const uint8_t no_match = 0xFF;
	uint32_t index, tag;
	uint32_t byte_select, dirty_line_address;
	cache_set* indexed_set;
	uint8_t tag_matched_line;

	/* Update the trace counter if the buffer has something in it */
	if(write_buffer.head != write_buffer.tail) {
		write_buffer.trace_counter++;
	}

	/* Decode the address */
	byte_select = extract_byte_select(address);
	index = extract_index(address);
	tag = extract_tag(address);

	/* Decode the trace operation */
	indexed_set = &sets[index];
	
	/* First, check for our CPU or snooping operations */
	if( trace_op < CLEAR_RESET ) {
		tag_matched_line = check_tags(tag, indexed_set);	//Will return 0xFF if there is no match

		switch( trace_op ) {
			case READ_DATA_L1:
				L1_read_or_write(CPU_READ, tag, address, tag_matched_line, indexed_set);
				break;

			case WRITE_DATA_L1:
				L1_read_or_write(CPU_WRITE, tag, address, tag_matched_line, indexed_set);
				break;

			case READ_INS_L1:
				L1_read_or_write(CPU_READ, tag, address, tag_matched_line, indexed_set);
				break;

			case SNOOP_INV:
				if(tag_matched_line != no_match) {
					/* If line is modified, it must be put into the write buffer */
					if(indexed_set->line[tag_matched_line].MESIF == MODIFIED) {
						dirty_line_address |= (indexed_set->line[tag_matched_line].tag << (BYTE_SELECT_BITS + INDEX_BITS)) | (index << BYTE_SELECT_BITS);
					add_to_write_buffer(dirty_line_address);
					}

					other_CPU_operation(INVALIDATE, address, &indexed_set->line[tag_matched_line]);
					indexed_set->valid_ways--;
				}
				break;

			case SNOOP_READ:
				if(tag_matched_line != no_match) {
					other_CPU_operation(READ, address, &indexed_set->line[tag_matched_line]);
				}
				break;

			case SNOOP_WRITE:
				if(tag_matched_line != no_match) {
					other_CPU_operation(WRITE, address, &indexed_set->line[tag_matched_line]);
				}
				break;

			case SNOOP_RWIM:
				if(tag_matched_line != no_match) {
					other_CPU_operation(RWIM, address, &indexed_set->line[tag_matched_line]);
					indexed_set->valid_ways--;
				}
				break;
				default:
					printf("ERROR: INVALID TRACE OPERATION REQUESTED: %d\n", trace_op);
				break;
		}//End switch
	} //End if
	else if( trace_op == CLEAR_RESET ) {	/*Clear the cache and reset the state */
		reset_cache();
	}/*end else if*/
	else if( trace_op == PRINT_STATES) { /* Print out all valid cache lines */
		print_cache();
	}/*end else if*/
	else {
		printf("Not a trace operation\n");
	}/*end else*/

	if(write_buffer.trace_counter == WB_COUNT) { /* Need to write a line back to memory */
		writeback_line_write_buffer();
		write_buffer.trace_counter = 0;
	}
}

/******************************************************************************
 * FUNCTION THAT CHECKS FOR A MATCHING TAG WITHIN THE SET
 * RETURNS 0xFF IF NO TAG MATCHES AND THE LINE INDEX IF THERE IS A MATCH
 *****************************************************************************/
uint8_t check_tags(uint32_t tag, cache_set* set) {
	uint32_t index;
	uint8_t match_index = 0xFF;

	/* Check the lines in the set for matching tag */
	for(index = 0; index < WAYS; index++) {
		if( set->line[index].MESIF != INVALID) {
			if( tag == set->line[index].tag ) {
				match_index = index;
				#ifdef DEBUG
					printf("Tag 0x%X found at line %d\n", tag, index);
				#endif
				break;
			}/*End if*/
		}/*End if*/
	}/*End for*/

	return match_index;
}/*END CHECK TAG BIT*/


/******************************************************************************
 * INIT THE CACHE
 *****************************************************************************/
void init_cache(void) {
	int set_index, line_index;

	clear_write_buffer();

	/* Generate the truncate mask for the posted write buffer */
	for(set_index = 0; set_index < BYTE_SELECT_BITS; set_index++) {
		write_buffer.truncate_mask |= 1 << set_index;
	}

	/*INITIALIZE CACHE ARRAY*/
	for(set_index = 0; set_index < NUM_SETS; set_index++) {
		sets[set_index].pseudo_LRU = 0;
		sets[set_index].valid_ways = 0;
		sets[set_index].line = (cache_line*)malloc(sizeof(cache_line) * WAYS);

		for(line_index = 0; line_index < WAYS; line_index++){
			/* Init values in each line */
			sets[set_index].line[line_index].tag = 0;
			sets[set_index].line[line_index].MESIF = INVALID;
		}/*end inner for*/
	}/*end outer for*/
}

/******************************************************************************
 * RESETS THE CACHE
 *****************************************************************************/
void reset_cache(void) {
	int set_index, line_index;

	clear_write_buffer();
	
	/* Reset cache statistics */
	cache_hits = 0;
	cache_misses = 0;
	cache_reads = 0;
	cache_writes = 0;

	/* Reset cache sets */
	for(set_index = 0; set_index < NUM_SETS; set_index++) {
		sets[set_index].pseudo_LRU = 0;
		sets[set_index].valid_ways = 0;

		for(line_index = 0; line_index < WAYS; line_index++){
			sets[set_index].line[line_index].tag = 0;
			sets[set_index].line[line_index].MESIF = INVALID;
		}/*end inner for*/
	}/*end outer for*/
}/*end reset_cache*/

/******************************************************************************
 * PRINTS OUT ALL VALID CACHE LINES
 *****************************************************************************/
void print_cache(void) {
	int set_index, line_index;
	char MESIF_text[10];
	uint32_t tag_bits;
	uint8_t MESIF_state;

	printf("\n/****************************************************************\n");
	printf("** PRINTING VALID LINES IN CACHE\n");
	printf("*****************************************************************/\n");

	for(set_index = 0; set_index < NUM_SETS; set_index++) {
		for(line_index = 0; line_index < WAYS; line_index++){
			if(sets[set_index].line[line_index].MESIF != INVALID ) {
				tag_bits = sets[set_index].line[line_index].tag; 
				MESIF_state = sets[set_index].line[line_index].MESIF;

				#ifdef PRETTY_OUTPUT
				if(MESIF_state == EXCLUSIVE) strcpy(MESIF_text, "EXCLUSIVE");
				else if(MESIF_state == SHARED) strcpy(MESIF_text, "SHARED");
				else if(MESIF_state == FORWARD) strcpy(MESIF_text, "FORWARD");
				else if(MESIF_state == MODIFIED) strcpy(MESIF_text, "MODIFED");
				else strcpy(MESIF_text, "ERROR");
				
				printf("Set number: 0x%X, Line number: %d, MESIF state: %s, Tag bits: 0x%X\n", set_index, line_index, MESIF_text, tag_bits);
				#else
				printf("Set number: 0x%X, Line number: %d, MESIF state: %d, Tag bits: 0x%X\n", set_index, line_index, MESIF_state, tag_bits);
				#endif
			}/*end inner if*/
		} //End inner for
	} //End outer for

	printf("\n/****************************************************************\n");
	printf("** PRINTING VALID LINES IN WRITE BUFFER\n");
	printf("*****************************************************************/\n");

	for(set_index = 0; set_index < WB_SIZE; set_index++) {
		if(write_buffer.slot_valid[set_index]) {
			printf("Write buffer slot %d holds cache line: 0x%X\n", set_index, write_buffer.buffer_slot[set_index]);
		}
	}

	printf("\n");

}/*end print_cache*/

/******************************************************************************
 * PRINTS CACHE STATISTICS
 *****************************************************************************/
void print_statistics(void) {
	double hit_ratio = (double)cache_hits / ((double)cache_hits + (double)cache_misses);

	printf("\n/****************************************************************\n");
	printf("** PRINTING CACHE STATISTICS\n");
	printf("*****************************************************************/\n");
	printf("Cache reads: %u\n", cache_reads);
	printf("Cache writes: %u\n", cache_writes);
	printf("Cache hits: %u\n", cache_hits);
	printf("Cache misses: %u\n", cache_misses);
	printf("Cache hit ratio: %g\n", hit_ratio);
}/*end print_statistics*/


/******************************************************************************
 * CLEANUP FUNCTION
 *****************************************************************************/
void cleanup(FILE* trace_file) {
	uint32_t set_index;
	fclose(trace_file);

	/* Free all the lines */
	for(set_index = 0; set_index < NUM_SETS; set_index++) {
		free(sets[set_index].line);
	}

	free(sets);
	print_statistics();
}

/******************************************************************************
 * DETERMINES WHAT TO DO ON A CPU READ OR WRITE DEPENDING ON IF THE LINE IS
 * ALREADY IN THE CACHE OR NOT
 *****************************************************************************/
void L1_read_or_write(uint8_t CPU_op, uint32_t tag, uint32_t address, 
								uint8_t tag_matched_line, cache_set* indexed_set) { 
	const uint8_t no_match = 0xFF;

	/* Update cache statistics */
	if(CPU_op == READ_DATA_L1 || CPU_op == READ_INS_L1) cache_reads++;
	else cache_writes++;

	if( tag_matched_line == no_match ) {	/* Not in the cache already */

		cache_misses++;
		if( indexed_set->valid_ways < WAYS) { /* There is an invalid line */
			fill_invalid_line(CPU_op, tag, address, indexed_set);
		}
		else { /* All lines are currently valid */
			fill_valid_line(CPU_op, tag, address, indexed_set);
		}

	}/*end if*/
	else {	/* In the cache already */
		indexed_set->pseudo_LRU = update_LRU(tag_matched_line, indexed_set->pseudo_LRU);
		CPU_operation(CPU_op, tag, &indexed_set->line[tag_matched_line]);
		cache_hits++;
	}/*end else*/

	#ifdef DEBUG
	printf("After read/write, pseudo LRU bits = 0x%llX\n", (long long unsigned int)indexed_set->pseudo_LRU);
	#endif
}/*end L1_read_or_write*/

/******************************************************************************
 * FUNCTION THAT FILLS AN INVALID CACHE LINE
 * HANDLE UPDATING MESIF and PLRU in here as well
 *****************************************************************************/
void fill_invalid_line(uint8_t CPU_op, uint32_t tag, uint32_t address, cache_set* set) {
	uint8_t line_filled;
	uint8_t line_index, in_write_buffer;

	/* Search write buffer first */
	in_write_buffer = search_write_buffer(address);	/* Will return 1 if found */

	#ifndef SILENT
	if(in_write_buffer) printf("Read cache line from write buffer: 0x%X\n", address);
	else printf("Read cache line from DRAM: 0x%X\n", address);
	#endif

	/* Replace TAG in an invalid line */
	for(line_index = 0; line_index < WAYS; line_index++) {
		if( set->line[line_index].MESIF == INVALID ) {
			set->line[line_index].tag = tag;
			set->valid_ways++;
			line_filled = line_index;
			break;
		}/*end if*/
	}/*end for*/

	#ifdef DEBUG
		printf("Valid ways = %d\n", set->valid_ways);
	#endif

	/* Update the LRU bits and MESIF state */
	set->pseudo_LRU = update_LRU(line_filled, set->pseudo_LRU);
	CPU_operation(CPU_op, address, &set->line[line_index]);
}/*end fill_invalid*/

/******************************************************************************
 * FUNCTION THAT FILLS A LINE WHEN ALL LINES ARE VALID
 * HANDLE UPDATING MESIF and PLRU in here as well
 *****************************************************************************/
void fill_valid_line(uint8_t CPU_op, uint32_t tag, uint32_t address, cache_set* set) {
	uint8_t line_to_evict, in_write_buffer;
	uint32_t index_bits = extract_index(address);	//Needed to rebuild the address of an evicted dirty line
	uint32_t dirty_line_address = 0;

	/* Search write buffer first */
	in_write_buffer = search_write_buffer(address);	/* Will return 1 if found */

	#ifndef SILENT
	if(in_write_buffer) printf("Read cache line from write buffer: 0x%X\n", address);
	else printf("Read cache line from DRAM: 0x%X\n", address);
	#endif

	/* Find the line to evict and change the MESIF state to INVALID
	** IF evicted line was in the modified state, write it to write buffer */
	line_to_evict = FindVictim(set->pseudo_LRU);
	if(set->line[line_to_evict].MESIF == MODIFIED) {
		dirty_line_address |= (set->line[line_to_evict].tag << (BYTE_SELECT_BITS + INDEX_BITS)) | (index_bits << BYTE_SELECT_BITS);
		add_to_write_buffer(dirty_line_address);
	}

	set->line[line_to_evict].MESIF = INVALID;
	#ifdef DEBUG
		printf("Index of line to evict = %d\n", line_to_evict);
	#endif

	/* Update the tag */
	set->line[line_to_evict].tag = tag;

	/* Update the LRU bits and MESIF state */
	set->pseudo_LRU = update_LRU(line_to_evict, set->pseudo_LRU);
	CPU_operation(CPU_op, address, &set->line[line_to_evict]);
}/*end fill_valid_line*/


/******************************************************************************
 * EXTRACTS THE BYTE SELECT BITS FROM THE ADDRESS
 *****************************************************************************/
 uint32_t extract_byte_select(uint32_t address) {
	uint32_t byte_select_mask;
 	uint32_t byte_select_bits;
	uint8_t index;

	byte_select_mask = 0;

	/* Generate the byte select mask */
	for(index = 0; index < BYTE_SELECT_BITS; index++) {
		byte_select_mask |= 1 << index;
	}

	/* Extract byte select bits */
	byte_select_bits = (address & byte_select_mask);
	#ifdef DEBUG
		printf("Byte select bits = 0x%X\n", byte_select_bits);
	#endif

	return byte_select_bits;
}/*end extract_byte_select*/

/******************************************************************************
 * EXTRACTS THE INDEX BITS FROM THE ADDRESS
 *****************************************************************************/
 uint32_t extract_index(uint32_t address) {
 	uint32_t index_mask;
 	uint32_t index_bits;
	uint8_t index;

	index_mask = 0;

	/* Generate the index mask */
	for(index = BYTE_SELECT_BITS; index < (BYTE_SELECT_BITS + INDEX_BITS); index++) {
		index_mask |= 1 << index;
	}/*end for*/

	/* Extract index bits */
	index_bits = (address & index_mask) >> BYTE_SELECT_BITS;	//Shift into the LSB
	#ifdef DEBUG
		printf("Index bits = 0x%X\n", index_bits);
	#endif

	return index_bits;
}/*end extract_index*/

/******************************************************************************
 * EXTRACTS THE TAG BITS FROM THE ADDRESS
 *****************************************************************************/
 uint32_t extract_tag(uint32_t address) {
 	uint32_t tag_mask;
 	uint32_t tag_bits;
	uint8_t index;

	tag_mask = 0;

	/* Generate the tag mask */
	for(index = (BYTE_SELECT_BITS + INDEX_BITS); index < ADDRESS_BITS; index++) {
		tag_mask |= 1 << index;
	}

	/* Extract tag bits */
	tag_bits = (address & tag_mask) >> (BYTE_SELECT_BITS + INDEX_BITS);	//Shift into the LSB
	#ifdef DEBUG
		printf("Tag bits = 0x%X\n", tag_bits);
	#endif

	return tag_bits;
}/*end extract_tag*/

/******************************************************************************
 * ADDS DIRTY EVICTED CACHE LINE TO THE WRITE BUFFER
 *****************************************************************************/
void add_to_write_buffer(uint32_t address) {
	uint32_t line_address = address & ~write_buffer.truncate_mask;	//Set byte offset bits to 0
	uint8_t in_buffer = search_write_buffer(address);	/* Will return 1 if it is in the buffer */

	/* Buffer could be full, so stall and write to DRAM */
	if(write_buffer.full) {
		#ifdef DEBUG
		printf("Write buffer full - stalling processor to write to DRAM\n");
		#endif
		writeback_line_write_buffer();
	}

	if(in_buffer) { /* We already have the tag as a write in the buffer */
		/* Remove the old write */
		write_buffer.buffer_slot[write_buffer.tag_index] = 0;
		write_buffer.slot_valid[write_buffer.tag_index] = 0;
	}

	/* Add the new write */
	write_buffer.buffer_slot[write_buffer.tail] = line_address;
	write_buffer.slot_valid[write_buffer.tail] = 1;
	write_buffer.tail = (write_buffer.tail + 1) % WB_SIZE;	/* Wrap around the queue */
	if(write_buffer.tail == write_buffer.head)
		write_buffer.full = 1;
}

/******************************************************************************
 * SEARCHES THE WRITE BUFFER FOR AN EXISTING CACHE LINE
 * RETURNS 1 IF THE LINE IS IN THE BUFFER, 0 IF THE LINE IS NOT IN THE BUFFER
 *****************************************************************************/
uint8_t search_write_buffer(uint32_t address) {
	uint8_t index;
	uint8_t retval = 0;
	uint32_t line_address = address & ~write_buffer.truncate_mask;	//Set byte offset bits to 0

	#ifdef DEBUG
	printf("Search Write Buffer: Address: 0x%X, Line Address: 0x%X\n", address, line_address);
	#endif

	/* If the two positions are not equal, then the buffer is not empty */
	if(write_buffer.tail != write_buffer.head) {
		for(index = 0; index < WB_SIZE; index++) {

			if(write_buffer.buffer_slot[index] == line_address) {
				if(write_buffer.slot_valid[index]) { /* The line is in the write buffer */
					write_buffer.tag_index = index;
					retval = 1;
					break;
				} /* End slot valid */
			} 

		} /* End for */
	}

	return retval;
}

/******************************************************************************
 * WRITES A LINE BACK TO DRAM FROM THE WRITE BUFFER EVERY WB_COUNT TRACES
 *****************************************************************************/
void writeback_line_write_buffer(void) {
	/* Head may not be valid from a newer write to the same address. 
	** Keep moving forward until a valid line is found
	*/
	while(write_buffer.slot_valid[write_buffer.head] != SLOT_VALID) {
		write_buffer.head = (write_buffer.head + 1) % WB_COUNT;	/* Wrap around */
	}

	#ifndef SILENT
	printf("Write buffer Writeback to DRAM, Address: 0x%X\n", write_buffer.buffer_slot[write_buffer.head]);
	#endif

	write_buffer.buffer_slot[write_buffer.head] = 0;
	write_buffer.slot_valid[write_buffer.head] = 0;
	write_buffer.head = (write_buffer.head + 1) % WB_COUNT;	/* Wrap around */
}

/******************************************************************************
 * CLEARS THE WRITE BUFFER
 *****************************************************************************/
void clear_write_buffer(void) {
	int index;

	/* Clear write buffer */
	write_buffer.head = 0;
	write_buffer.tail = 0;
	write_buffer.full = 0;
	write_buffer.trace_counter = 0;
	write_buffer.tag_index = 0;
	for(index = 0; index < WB_SIZE; index++) {
		write_buffer.buffer_slot[index] = 0;
		write_buffer.slot_valid[index] = 0;
	}
}

/******************************************************************************
 * EOF
 *****************************************************************************/

