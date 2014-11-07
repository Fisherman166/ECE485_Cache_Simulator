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

//#define SILENT

/*BUS OPS:*/
#define READ 		1
#define WRITE 		2
#define INVALIDATE 	3
#define RWIM 		4

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

char *GetSnoopResult(char BusOp, unsigned int Address); /*function prototype*/

typedef struct line{
	
	uint16_t tag;
	
}LINE;/*end line struct definition*/

typedef struct set{

	struct set * lines[15];
	uint16_t MESIF;

} SET;/*end set struct definition*/

/******************************************************************************
 * USED TO SIMULATE A BUS OPERATION AND TO CAPTURE THE SNOOP RESULTS OF
 * LAST LEVEL CACHES OF OTHER PROCESSORS:
 *****************************************************************************/
void BusOperation(char BusOp, unsigned int Address, char *SnoopResult){
SnoopResult = GetSnoopResult(BusOp, Address);

#ifndef SILENT
	printf("BusOp: %d, Address %h, Snoop Result %d\n", *SnoopResult);
#endif

}/*end BusOperation*/

/******************************************************************************
 * USED TO SIMULATE THE REPORTING OF SNOOP RESULTS BY CACHES:
 *****************************************************************************/
char *GetSnoopResult(char BusOp, unsigned int Address)
{

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


}/*end Message ToL2Cache*/
int main (int argc, char * argv[])
{
int index=0;

FILE *trace;

trace = fopen("cc1.din", "r");
if(trace ==NULL){
	 printf("UNABLE TO OPEN FILE\n");
	exit(-1);
}//end if
	
return 0;
}
/******************************************************************************
 * EOF
 *****************************************************************************/
