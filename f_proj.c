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
FILE *trace;

typedef struct TLINE{
	char * snooped;
	char * address;
	int * index;
	int * dec_addr;
	uint8_t  dec_op;
}TLINE;
TLINE trace_line;
uint8_t GetSnoopResult(TLINE *); /*function prototype*/
/******************************************************************************
 * USED TO GET NEXT LINE FROM TRACE FILE 
 * FIRST SINGLE DIGIT (COLUMN 0) IS BUS OP
 * REMAINDER OF TRACE FILE  ENTRY IS ADDRESS
 *****************************************************************************/
TLINE * get_line(int * index){
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
 * FUNCTION TO CHECK THE TAG BIT
 *****************************************************************************/
uint32_t check_tag(TLINE * traceline){

}/*END CHECK TAG BIT*/
/******************************************************************************
 * THIS STRUCTURE REPRESENTS A SINGLE CACHE LINE, SINCE NO DATA, ONLY
 * HOLDS A TAG BIT...
 *****************************************************************************/
typedef struct{
	uint16_t *tag;

}line;
/******************************************************************************
 * THIS UNION REPRESENTS AN ENTIRE SET OF CACHE LINES, SINCE 16-WAY
 * ASSOCIATIVITY, AN ARRAY HOLDS THE 16 WAYS, EACH WITH A UNIQUE TAG
 * THEN ALSO WITHIN THE UNION ARE THE MESIF BITS 
 *****************************************************************************/
typedef union{
	struct{
		line * lines[15];
		uint16_t *MESIF;
	}set;
}LINE;/*end set struct definition*/

/******************************************************************************
 * USED TO SIMULATE A BUS OPERATION AND TO CAPTURE THE SNOOP RESULTS OF
 * LAST LEVEL CACHES OF OTHER PROCESSORS:
 *****************************************************************************/
void BusOperation(char BusOp, int Address){
	
	uint8_t temp_op = BusOp-'0';
	
#ifndef SILENT
	printf("BusOp: %d, Address: 0x%x\n", temp_op, Address);
#endif
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
	switch(temp_op){
		case READ_DATA_L1 :
			//check tag, check MESIF status, if exclusive, FWD
			//check snoop result			
			//else, cache miss
		break;
		case WRITE_DATA_L1 :
			//check tag, check MESIF status, if exists and EXC PWB
			//if exists but INVALIDATED, other forwards
			//if exists and shared, put INVALIDATE on line
			//else READ op
		break;
		case READ_INS_L1 :
			//change MESIF bits, mark line for eviction
		break;
		case SNOOP_INV :
			//check tag, check MESIF status
			//check snoop result, perhaps other FORWARDS 
			//post INVALIDATE
		break;
		case SNOOP_READ:
		
		
		break;
		case SNOOP_WRITE:
		

		break;
		case SNOOP_RWI:
		

		break;
		case CLEAR_RESET:
		

		break;
		case PRINT_STATES:
		
		break;
		default:
		break;
	}//END SWITCH
}/*end BusOperation*/

/******************************************************************************
 * USED TO SIMULATE THE REPORTING OF SNOOP RESULTS BY CACHES:
 *****************************************************************************/
uint8_t GetSnoopResult(TLINE * to_snoop)
{
uint8_t BusOp=0;
	
return BusOp;	
}/*end GetSnoopResult*/

/******************************************************************************
 * USED TO REPORT SNOOP RESULT FOR SNOOPING CACHES:
 *****************************************************************************/
void PutSnoopResult(unsigned int Address, char SnoopResult){

#ifndef SILENT
	printf("SnoopResult: Address %h. SnoopResult: %d\n", Address, SnoopResult);
#endif

}/*end PutSnoopResult*/

/******************************************************************************
 * USED TO SIMULATE COMMUNICATION TO OUR UPPER LEVEL CACHE
 * -verifies behavior of response
 *****************************************************************************/
void MessageToL2Cache( char BusOp, unsigned int Address)
{
#ifndef SILENT
	printf("L2: %d %h\n", BusOp, Address);
#endif

/******************************************************************************
 * 	MAIN LOOP: 
 * 	-GET NEXT LINE OF TRACE FILE
 * 	-SPLIT LINE INTO PARTS
 * 	-COMPARE TAG
 * 	-UPDATE STATUS
 *****************************************************************************/

}/*end Message ToL2Cache*/

int main (int argc, char * argv[])
{
int index=0;
int bufx=2;
//char * next_line;
TLINE * next_line;
char * SnoopResult;
uint32_t Address;

int width, height;
width=height=0;

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
	for(height; height<8000; ++height){
		for(width; width<WAYS; ++width){
			LINE *entry = malloc(sizeof(LINE));		
			entry->set.MESIF = malloc(sizeof(uint16_t));
			entry->set.lines[width] = malloc(sizeof(line));
			entry->set.lines[width]->tag = malloc(sizeof(uint8_t));
			#ifdef WARM_CACHE
				entry->MESIF = (rand()%4);
				entry->lines[width]->tag = (rand()%exp(16,32));
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
 * EOF
 *****************************************************************************/
