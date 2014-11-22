/******************************************************************************
 ******************************************************************************
 * J.S. PEIRCE       jpei2@pdx.edu
 * SEAN KOPPENHAFER  koppen2@pdx.edu
 * LUIS SANTIAGO     lsantiag@pdx.edu
 *
 *
 * ECE485 FINAL PROJECT
 *
 *   LRU.c
 ******************************************************************************
 *****************************************************************************/
//
//  This File Created by LUIS SANTIAGO on 11/11/14.
//
//

#include "cache.h"
#include "LRU.h"
#include <math.h>
#include <assert.h>
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,n) ((var) |= (1 << (n)))
#define CLR_BIT(var,n) ((var) &= ~(1 << (n)))



typedef struct {
    uint32_t victim;        // initiate to 0 and it is used to generate
                            // binary value of Least recently used
    uint32_t levels;        // saves Log2(Ways) used to count leves
                            // on tree and determine when done
    uint32_t MIDPOINT;      // Value to determine walk direction
    uint8_t Bits;          // index of bits in LRU
    uint64_t LRU;           // Stores passed LRU (only for simplicity)
    uint8_t n;             //
}LRU_Data ;





// update to least recently used
// 0 left
// 1 right
/*    L3                  _______ b0________
                        /	                 \
 L2               ____b1___                 ___b2_____
                 /         \               /          \
 L1            b3          b4             b5           b6
              /  \       /   \          /  \        /      \
 L0         b7   b8    b9    b10     b11   b12     b13     b14
            / \   / \   / \  /  \     / \   /  \   /   \   /   \
 DECODE    0  1  2  3  4  5  6  7    8  9  10  11  12  13 14  15
 *********************************************************
*/
uint8_t WAYS;







void decode_LRU(LRU_Data* packLRU )
{
    /* check LRU bits and use them as flag to get the next bit
     to read in order to find the direction at the next lower level
     When travering each level it creates the binary value of victim
     -Bit is defined in find victim as Bit=0 which is the
     root of tree.
     -Compute next bit by:
     - when going right (Bit*2)+2
     - going left (Bit*2)+1
     */
    
    if (CHECK_BIT(packLRU->LRU,packLRU->Bits)) {
        
        //Update to indicate indicate next lower level
        --packLRU->levels;
        printf("True: \tbit checked: %u \tlevels left to check:%u\n",packLRU->Bits, packLRU->levels);
        
        
        packLRU->Bits=(packLRU->Bits*2)+2;
        if (packLRU->levels>0)
        {
            packLRU->victim = packLRU->victim<<1;
            SET_BIT(packLRU->victim,0);
            decode_LRU( packLRU);
        }else{
            packLRU->victim = packLRU->victim<<1;
            packLRU->victim |= 1 << 0;
            return;
        }
    }else{
        
        --packLRU->levels;  //indicate next lower level
        
        printf("False:\tbit checked:%u \tlevels left to check:%u\n",packLRU->Bits,packLRU->levels );
        packLRU->Bits=(packLRU->Bits*2)+1;
      
        if (packLRU->levels>0)
        {
            packLRU->victim=packLRU->victim<<1;
            decode_LRU(packLRU);
        }else{
            packLRU->victim= packLRU->victim<<1;
            return;
        }
    }
}






//  Takes the  LRU Value from the main program in order to fint the victim
// input uint64_t

uint8_t FindVictim (uint64_t LRU)
{
    
    LRU_Data myStruct;
    LRU_Data *packLRU;
    packLRU= & myStruct;
    
    packLRU->Bits=0;
    packLRU->victim=0;
    packLRU->LRU=LRU;
    // levels determine how many iteration are needed based on WAYS
    packLRU->levels=log2(WAYS);
    assert((pow(2,packLRU->levels))==WAYS);    // ensures it is a power of 2
    
    decode_LRU(packLRU );
    return packLRU->victim;
}


void GoRight(LRU_Data* packLRU)

{
    uint32_t midpoint = packLRU->MIDPOINT;
    
    // first clear bit
    CLR_BIT(packLRU->LRU , packLRU->Bits);
    
    printf("Clear bit: %u \t at level:%u \t", packLRU->Bits, packLRU->levels);
    --packLRU->levels;
    
    // calculate and Store next bit to check
    packLRU->Bits=(packLRU->Bits*2)+2;
    
    // update N in order to correctly update midpoint
    ++packLRU->n;
    
    // update midpoint and store back into struct
    packLRU->MIDPOINT= midpoint + WAYS/(pow(2,packLRU->n));
    printf("Midpoint: %u\n",packLRU->MIDPOINT);
}


void GoLeft(LRU_Data* packLRU)
{
    uint32_t midpoint = packLRU->MIDPOINT;
    
    // first clear bit
    SET_BIT(packLRU->LRU , packLRU->Bits);
    
    printf("SET bit: %u \t at level:%u \t", packLRU->Bits, packLRU->levels);
    --(packLRU->levels);
    
    // calculate and Store next bit to check
    packLRU->Bits=(packLRU->Bits*2)+1;
    
    // update N in order to correctly update midpoint
    ++packLRU->n;
    
    // update midpoint and store back into struct
    packLRU->MIDPOINT= midpoint - WAYS/(pow(2,packLRU->n));
    printf("Midpoint: %u\n",packLRU->MIDPOINT);
}



/* Check line value given against the root of the tree
 next it will create a subtree of either left or right side and
 and repeat, in the process it will update the LRU bits
 accordinly to update for the MOST recently used
 */
void Check_Tree(LRU_Data* packLRU, uint32_t line)
{
 /* midpoint originally was set by update LRU
   Each desition acter this must update midpoint
  */
    if (line < packLRU->MIDPOINT) {
        
        if (packLRU->levels > 0)
        {
            GoLeft(packLRU);
            Check_Tree (packLRU, line);
        }
    }
    else {
        if (packLRU->levels > 0)
        {
             GoRight(packLRU);
            Check_Tree (packLRU, line);
        }
    
    }
}

uint64_t update_LRU(uint8_t line, uint64_t LRU)
{
 // creates a struct and saves the pointer
 // Struct contains some if the elements needed to
 // compute and update LRU
    
    LRU_Data myStruct;
    LRU_Data *packLRU;
    packLRU= & myStruct;
    
    packLRU->Bits=0;
    packLRU->victim=0;
    packLRU->LRU=LRU;
    packLRU->n=1;
   
    packLRU->levels=log2(WAYS) ;
    packLRU->MIDPOINT= WAYS/(pow(2,packLRU->n));
    
    Check_Tree(packLRU, line);
 
    return packLRU->LRU;
}



int main()
{
    uint8_t i, victim;
    char cont;
    
    cache_set test_set;
    test_set.pseudo_LRU=0;
    LRU_Data *packLRU;
    LRU_Data myStruct;
    packLRU= & myStruct;
    
     printf( "Enter Cache Set ways:\n");
     scanf("%u", &WAYS);
    
     //Fill up all entered ways
    
     for(i = 0; i < WAYS; i++) {
     test_set.pseudo_LRU = update_LRU(i, test_set.pseudo_LRU);
     test_set.valid_ways++;
     printf("On init %d, LRU = 0X%llx\n\n", i, test_set.pseudo_LRU);
     }
    
    printf( "\n**************************************\n");
    printf( "\nPress y (lower case) with Victim test:\n");
    scanf(" %c", &cont);
    
    if(cont=='y'){
    //  Evict all lines in order
     for(i = 0; i < WAYS; i++) {
     victim = FindVictim(test_set.pseudo_LRU);
     test_set.pseudo_LRU = update_LRU(victim, test_set.pseudo_LRU);
     printf("Victim is index %d\n\n", victim);
     }
    }
    
    return 0;
}

