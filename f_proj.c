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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "MESIF.h"
#include "cache.h"
#include "LRU.h"

//#define SILENT
#define TEST_FILE
#define DEBUG

/*TRACE RESULTS:*/
#define READ_DATA_L1  0
#define WRITE_DATA_L1 1
#define READ_INS_L1   2
#define SNOOP_INV     3
#define SNOOP_READ    4
#define SNOOP_WRITE   5
#define SNOOP_RWI     6
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

typedef struct TLINE{
	char * snooped;
	char * address;
	int * index;
	int * dec_addr;
	uint8_t  dec_op;
}TLINE;

/* Trace related functions */
TLINE * get_line(int * index);
void decode_trace(uint8_t trace_op, uint32_t address);
uint8_t check_tags(uint16_t tag, cache_set* set);

/* CPU reads and writes */
void L1_read_or_write(uint8_t CPU_op, uint16_t tag, uint32_t address, uint8_t tag_matched_line, cache_set* indexed_set);
void fill_invalid_line(uint8_t CPU_op, uint16_t tag, uint32_t address, cache_set* set);
void fill_valid_line(uint8_t CPU_op, uint16_t tag, uint32_t address, cache_set* set);

/* Functions that break down the address */
uint8_t extract_byte_select(uint32_t address);
uint16_t extract_index(uint32_t address);
uint16_t extract_tag(uint32_t address);

/* Global values */
FILE *trace;
TLINE trace_line;
cache_set sets[NUM_SETS];	//8k sets in the cache - see cache.h

/* Sean's test main */
int main() {
	uint32_t address = 0x60680000;
	int set_index, line_index;

	/*INITIALIZE CACHE ARRAY*/
	for(set_index = 0; set_index < NUM_SETS; set_index++) {
		for(line_index = 0; line_index < WAYS; line_index++){
			sets[set_index].psuedo_LRU = 0;
			sets[set_index].valid_ways = 0;

			/* Init values in each line */
			sets[set_index].line[line_index].tag = 0;
			sets[set_index].line[line_index].MESIF = INVALID;

		}/*END INNER FOR*/

	}/*END OUTER FOR*/

	
	for(set_index = 0; set_index < 20; set_index++) {
		decode_trace(WRITE_DATA_L1, address);
		printf("Lines = %d\n\n", sets[0].valid_ways);
		address += 0x01000000;
	}
		address -= 0x01000000;
	decode_trace(READ_DATA_L1, address);

	return 0;
}

/******************************************************************************
 * 	MAIN LOOP: 
 * 	-GET NEXT LINE OF TRACE FILE
 * 	-SPLIT LINE INTO PARTS
 * 	-COMPARE TAG
 * 	-UPDATE STATUS
 *****************************************************************************/

//int main (int argc, char * argv[])
//{
//	int index=0;
//	int bufx=2;
//	//char * next_line;
//	TLINE * next_line;
//	char * SnoopResult;
//	uint32_t Address;
//
//	int set_index, line_index;
//
//	time_t t;
//	srand(time(&t));
//
//	#ifdef TEST_FILE
//		if(argc>1){
//			trace = fopen(argv[1], "r");
//		}else{
//			printf("\nplease specify trace file: \"./a.out <file name> \"\n\n");
//			exit(-1);
//		}
//	#else
//		trace = fopen("cc1.din", "r");
//	#endif
//		/*INITIALIZE CACHE ARRAY*/
//		for(set_index = 0; set_index < NUM_SETS; set_index++) {
//			for(line_index = 0; line_index < WAYS; line_index++){
//				sets[set_index].psuedo_LRU = 0;
//				sets[set_index].valid_ways = 0;
//
//				/* Init values in each line */
//				sets[set_index].line[line_index].tag = 0;
//				sets[set_index].line[line_index].MESIF = INVALID;
//
//				#ifdef WARM_CACHE
//					sets[set_index].line[line_index].MESIF = (rand()%4);
//					sets[set_index].line[line_index].tag = (rand()%exp(16,32));
//				#endif
//			}/*END INNER FOR*/
//
//		}/*END OUTER FOR*/
//
//		if(trace == NULL){ 		/*FILE FAILED TO OPEN*/
//			printf("UNABLE TO OPEN FILE\n");
//			exit(-1); 		/*EXIT WITH ERROR*/
//		}else printf("FILE opened successfully!\n");
//
//		do{ 	/*DONT WANT TO CHECK PRIOR TO */
//			next_line = get_line(&index);	
//			++index;
//		
//			#ifndef SILENT
//				printf("CONTROL: %c\n", next_line->snooped[0]);	
//				printf("STRING RESULT from MAIN: %s\n", next_line->snooped);	
//				printf("MAIN next_line->address: %s\n", next_line->address);
//			#endif
//		//	while((bufx < strlen(next_line->snooped)) && isalnum(next_line->snooped[bufx])){
//		//		next_line->address[bufx] = next_line->snooped[bufx];
//		//	printf("MAIN next_line->address[bufx]: %c\n", next_line->address[bufx]);
//		//	++bufx;
//		//	}
//			//sscanf(next_line->address, "%s", &next_line->dec_addr);
//			//next_line->dec_addr = (long int)strtol(next_line->address, NULL, 0);
//			#ifndef SILENT
//			//	printf("decoded address: 0x%x\n",next_line->dec_addr);
//			#endif
//			
//			//trace_line.dec_op = GetSnoopResult(next_line);
//			//check_tag(next_line);
//			//BusOperation(next_line->snooped[0], *next_line->dec_addr);	
//
//		}while(*next_line->snooped != EOF);
//	return 0;
//}/*END MAIN*/

/******************************************************************************
 * USED TO GET NEXT LINE FROM TRACE FILE 
 * FIRST SINGLE DIGIT (COLUMN 0) IS BUS OP
 * REMAINDER OF TRACE FILE  ENTRY IS ADDRESS
 *****************************************************************************/
TLINE * get_line(int * index) {
char next_char;
char buffer[11];
volatile int bufx=0;

#ifndef SILENT
	printf("line index: %d\n", *index);
#endif
	for(bufx; bufx<strlen(buffer); ++bufx) buffer[bufx]='Z';
	bufx=0;	
	do{ 	buffer[bufx] = fgetc(trace);
		++bufx;
	}while((buffer[bufx-1] != '\n') && (bufx<10));
	
	trace_line.snooped = (char*)malloc((sizeof(char)*(strlen(buffer))+1));
	strcpy(trace_line.snooped, buffer);
	for(bufx=0; bufx<strlen(buffer); ++bufx) buffer[bufx]='Z';
	bufx=0;
	do{	buffer[bufx] = trace_line.snooped[bufx+2];
		++bufx;
	}while(isalnum(buffer[bufx]));
	
	trace_line.address = (char*)malloc((sizeof(char)*(strlen(buffer))-2));
	strcpy(trace_line.address, buffer);
return &trace_line;
}/**END GET LINE*/


/******************************************************************************
* FUNCTION TO DECODE TRACE OPERATION
******************************************************************************/

/* Send in the trace op and the address */
void decode_trace(uint8_t trace_op, uint32_t address) {
	const uint8_t no_match = 0xFF;
	uint16_t index, tag;
	uint8_t byte_select, line_filled;
	cache_set* indexed_set;
	uint8_t tag_matched_line;

	/* Decode the address */
	byte_select = extract_byte_select(address);
	index = extract_index(address);
	tag = extract_tag(address);

	/* Decode the trace operation */
	indexed_set = &sets[index];
	
	/* First, check for our CPU or snooping operations */
	if( trace_op < CLEAR_RESET ) {
		tag_matched_line = check_tags(tag, indexed_set);	//Will return 0xFF

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
		}//End switch
	} //End if
}

/******************************************************************************
 * FUNCTION THAT CHECKS FOR A MATCHING TAG WITHIN THE SET
 * RETURNS 0xFF IF NO TAG MATCHES
 *****************************************************************************/
uint8_t check_tags(uint16_t tag, cache_set* set) {
	uint8_t index;
	uint8_t match_index = 0xFF;

	/* Check the lines in the set for matching tag */
	for(index = 0; index < WAYS; index++) {
		if( set->line[index].MESIF != INVALID) {
			if( tag == set->line[index].tag ) {
				match_index = index;
				#ifdef DEBUG
					printf("Tag 0x%X found at index %d\n", tag, index);
				#endif
				break;
			}
		}
	}

	return match_index;
}/*END CHECK TAG BIT*/



/******************************************************************************
 * DETERMINES WHAT TO DO ON A CPU READ OR WRITE DEPENDING ON IF THE LINE IS
 * ALREADY IN THE CACHE OR NOT
 *****************************************************************************/
void L1_read_or_write(uint8_t CPU_op, uint16_t tag, uint32_t address, 
							 uint8_t tag_matched_line, cache_set* indexed_set) { 
	const uint8_t no_match = 0xFF;

	if( tag_matched_line == no_match ) {	/* Not in the cache already */
		if( indexed_set-> valid_ways < WAYS) { /* There is an invalid line */
			fill_invalid_line(CPU_op, tag, address, indexed_set);
		}
		else { /* All lines are currently valid */
			fill_valid_line(CPU_op, tag, address, indexed_set);
		}
	}
	else {	/* In the cache already */
		indexed_set->psuedo_LRU = update_LRU(tag_matched_line, indexed_set->psuedo_LRU);
		CPU_operation(CPU_op, tag, &indexed_set->line[tag_matched_line]);
	}
}

/******************************************************************************
 * FUNCTION THAT FILLS AN INVALID CACHE LINE
 * HANDLE UPDATING MESIF and PLRU in here as well
 *****************************************************************************/
void fill_invalid_line(uint8_t CPU_op, uint16_t tag, uint32_t address, cache_set* set) {
	uint8_t line_filled;
	uint8_t line_index;

	/* Replace TAG in an invalid line */
	for(line_index = 0; line_index < WAYS; line_index++) {
		if( set->line[line_index].MESIF == INVALID ) {
			set->line[line_index].tag = tag;
			set->valid_ways++;
			line_filled = line_index;
			break;
		}
	}

	/* Update the LRU bits and MESIF state */
	set->psuedo_LRU = update_LRU(line_filled, set->psuedo_LRU);
	CPU_operation(CPU_op, address, &set->line[line_index]);
}

/******************************************************************************
 * FUNCTION THAT FILLS A LINE WHEN ALL LINES ARE VALID
 * HANDLE UPDATING MESIF and PLRU in here as well
 *****************************************************************************/
void fill_valid_line(uint8_t CPU_op, uint16_t tag, uint32_t address, cache_set* set) {
	uint8_t line_to_evict;

	/* Find the line to evict and change the MESIF state to INVALID */
	line_to_evict = LeastUsed(set->psuedo_LRU);
	set->line[line_to_evict].MESIF = INVALID;
	#ifdef DEBUG
		printf("Index of line to evict = %d\n", line_to_evict);
	#endif

	/* Update the tag */
	set->line[line_to_evict].tag = tag;

	/* Update the LRU bits and MESIF state */
	set->psuedo_LRU = update_LRU(line_to_evict, set->psuedo_LRU);
	CPU_operation(CPU_op, address, &set->line[line_to_evict]);
}


/******************************************************************************
 * EXTRACTS THE BYTE SELECT BITS FROM THE ADDRESS
 *****************************************************************************/
 uint8_t extract_byte_select(uint32_t address) {
	uint32_t byte_select_mask;
 	uint8_t byte_select_bits;
	uint8_t index;

	byte_select_mask = 0;

	/* Generate the byte select mask */
	for(index = 0; index < BYTE_SELECT_BITS; index++) {
		byte_select_mask |= 1 << index;
	}

	/* Extract byte select bits */
	byte_select_bits = (address & byte_select_mask) & 0xFF;
	#ifdef DEBUG
		printf("Byte select bits = 0x%X\n", byte_select_bits);
	#endif

	return byte_select_bits;
}

/******************************************************************************
 * EXTRACTS THE INDEX BITS FROM THE ADDRESS
 *****************************************************************************/
 uint16_t extract_index(uint32_t address) {
 	uint32_t index_mask;
 	uint16_t index_bits;
	uint8_t index;

	index_mask = 0;

	/* Generate the index mask */
	for(index = BYTE_SELECT_BITS; index < (BYTE_SELECT_BITS + INDEX_BITS); index++) {
		index_mask |= 1 << index;
	}

	/* Extract index bits */
	index_bits = ((address & index_mask) >> BYTE_SELECT_BITS) & 0xFFFF;	//Shift into the LSB
	#ifdef DEBUG
		printf("Index bits = 0x%X\n", index_bits);
	#endif

	return index_bits;
}

/******************************************************************************
 * EXTRACTS THE TAG BITS FROM THE ADDRESS
 *****************************************************************************/
 uint16_t extract_tag(uint32_t address) {
 	uint32_t tag_mask;
 	uint16_t tag_bits;
	uint8_t index;

	tag_mask = 0;

	/* Generate the tag mask */
	for(index = (BYTE_SELECT_BITS + INDEX_BITS); index < ADDRESS_BITS; index++) {
		tag_mask |= 1 << index;
	}

	/* Extract tag bits */
	tag_bits = ((address & tag_mask) >> (BYTE_SELECT_BITS + INDEX_BITS)) & 0xFFFF;	//Shift into the LSB
	#ifdef DEBUG
		printf("Tag bits = 0x%X\n", tag_bits);
	#endif

	return tag_bits;
}

/******************************************************************************
 * EOF
 *****************************************************************************/
