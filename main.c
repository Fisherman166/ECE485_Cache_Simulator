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
 *
 * Main.c
 ******************************************************************************
 *****************************************************************************/

#include "cache.h"
#include "MESIF.h"

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

