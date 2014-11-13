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
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<ctype.h>
#include "MESIF.h"
#include "cache.h"

//#define SILENT
//#define TEST_FILE
#define WAYS 16

/*BUS OPS:*/
#define READ 		1
#define WRITE 		2
#define INVALIDATE 	3
#define RWIM 		4

/*SNOOP RESULTS:*/
#define NOHIT 		0
#define HIT 		1
#define HITM 		2

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

#define TAG 9
#define INDEX 9

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

/* Functions */
TLINE * get_line(int * index);
void decode_trace(uint8_t trace_op, uint32_t address, cache_set* set);
uint32_t check_tag(TLINE * traceline);


FILE *trace;
TLINE trace_line;

/******************************************************************************
 * 	MAIN LOOP: 
 * 	-GET NEXT LINE OF TRACE FILE
 * 	-SPLIT LINE INTO PARTS
 * 	-COMPARE TAG
 * 	-UPDATE STATUS
 *****************************************************************************/

int main (int argc, char * argv[])
{
	int index=0;
	int bufx=2;
	//char * next_line;
	TLINE * next_line;
	char * SnoopResult;
	uint32_t Address;

	/* Added by Sean */
	cache_set sets[NUM_SETS];	//8k sets in the cache - see cache.h
	int set_index, line_index;

	time_t t;
	srand(time(&t));

	#ifdef TEST_FILE
		if(argc>1){
			trace = fopen(argv[1], "r");
		}else{
			printf("\nplease specify trace file: \"./a.out <file name> \"\n\n");
			exit(-1);
		}
	#else
		trace = fopen("cc1.din", "r");
	#endif
		/*INITIALIZE CACHE ARRAY*/
		for(set_index = 0; set_index < NUM_SETS; set_index++) {
			for(line_index = 0; line_index < WAYS; line_index++){
				sets[set_index].psuedo_LRU = 0;
				sets[set_index].valid_ways = 0;

				/* Init values in each line */
				sets[set_index].line[line_index].tag = 0;
				sets[set_index].line[line_index].MESIF = INVALID;

				#ifdef WARM_CACHE
					sets[set_index].line[line_index].MESIF = (rand()%4);
					sets[set_index].line[line_index].tag = (rand()%exp(16,32));
				#endif
			}/*END INNER FOR*/

		}/*END OUTER FOR*/

		if(trace == NULL){ 		/*FILE FAILED TO OPEN*/
			printf("UNABLE TO OPEN FILE\n");
			exit(-1); 		/*EXIT WITH ERROR*/
		}else printf("FILE opened successfully!\n");

		do{ 	/*DONT WANT TO CHECK PRIOR TO */
			next_line = get_line(&index);	
			++index;
		
			#ifndef SILENT
				printf("CONTROL: %c\n", next_line->snooped[0]);	
				printf("STRING RESULT from MAIN: %s\n", next_line->snooped);	
				printf("MAIN next_line->address: %s\n", next_line->address);
			#endif
		//	while((bufx < strlen(next_line->snooped)) && isalnum(next_line->snooped[bufx])){
		//		next_line->address[bufx] = next_line->snooped[bufx];
		//	printf("MAIN next_line->address[bufx]: %c\n", next_line->address[bufx]);
		//	++bufx;
		//	}
			//sscanf(next_line->address, "%s", &next_line->dec_addr);
			//next_line->dec_addr = (long int)strtol(next_line->address, NULL, 0);
			#ifndef SILENT
			//	printf("decoded address: 0x%x\n",next_line->dec_addr);
			#endif
			
			//trace_line.dec_op = GetSnoopResult(next_line);
			//check_tag(next_line);
			//BusOperation(next_line->snooped[0], *next_line->dec_addr);	

		}while(*next_line->snooped != EOF);
	return 0;
}/*END MAIN*/

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

/* Send in the trace op, the address, and the set that the address indexes to */
void decode_trace(uint8_t trace_op, uint32_t address, cache_set* set) {
	/* psuedo code for this part
	**
	** if(trace = 0-6) {
	**		check cache lines for tag match
	**		if tags match and L1 read
	**			call CPU_operation(CPU_READ, address, &set->line[index]);
	**		if tags match and L1 write
	**			call CPU_operation(CPU_WRITE, address, &set->line[index];
	**		if tags match and snoop from other cache
	**			call other_CPU_operation(bus_op, address, &set->line[index];
	**
	**		and more, I'm just too lazy right now
	*/
}

/******************************************************************************
 * FUNCTION TO CHECK THE TAG BIT
 *****************************************************************************/
uint32_t check_tag(TLINE * traceline){

}/*END CHECK TAG BIT*/


/******************************************************************************
 * EOF
 *****************************************************************************/
