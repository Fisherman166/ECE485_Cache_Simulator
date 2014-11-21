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
//#include "LRU.h"
#include <math.h>
#include <assert.h>
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,n) ((var) |= (1 << (n)))
#define CLR_BIT(var,n) ((var) &= ~(1 << (n)))



typedef struct {
    uint8_t i;
    uint16_t victim, levels;
    int MIDPOINT;
    int Bits;
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
 *********************************************************  */
int FindVictim(uint64_t);
void decode_LRU(LRU_Data*);
void update_LRU(uint8_t line, LRU_Data*);
void GoRightSide(int line, LRU_Data*, int n);

<<<<<<< Updated upstream
extern uint32_t WAYS;

// used to update the LRU
// input Set Pseudo_LRU and the index of the line
// on te set used.
// reurns an updated LRU value
uint16_t update_LRU( uint8_t index, uint16_t LRU)
{
    // update to least recently used
    // 0 left
    // 1 right
    /*    L3                  _______ b14________
                           /	                 \
     L2               ____b6___                 ___b13_____
                     /        \                /         \
     L1            b2         b5             b9           b12
                  /  \       /   \          /  \       /       \
     L0         b0   b1    b3    b4       b7     b8    b10      b11
               / \   / \   / \  /  \     / \   /  \   /   \    /   \
DECODE        0  1  2  3  4  5  6  7    8  9  10  11  12  13  14  15
     *********************************************************  */
 //****************************************     Decode


int main()
{
    int line;
    test_set.pseudo_LRU=0;
    cache_set test_set;
    LRU_Data *packLRU;
    LRU_Data myStruct;
    packLRU= & myStruct;
    
    
    
   // do
    //  {
         
  
          // just a test
          
          printf( "Read CACHE ways: %d\n",WAYS);
          packLRU->levels=log2(WAYS);
          
          // could be and should be done somewhere else
         assert((pow(2,packLRU->levels))==WAYS);    // ensures it is a power of 2
          printf( "Enter LRU value: ");
          scanf("%lx", &test_set.pseudo_LRU);
         FindVictim(packLRU);
          printf( "Enter line to update: ");
         scanf("%x", &line);
    //    update_LRU(line, WAYS);
    // }while (WAYS <65);
    
    return 0;
}



int FindVictim (LRU_Data *packLRU)
{
    
     packLRU->Bits=0;
    
   

    
     packLRU->victim=0;
    
    
    decode_LRU(packLRU);
   
}

void decode_LRU(LRU_Data* packLRU)
{
    // check bit
    if (CHECK_BIT(packLRU->LRU,packLRU->Bits)) {
        packLRU->Bits=(packLRU->Bits*2)+2;
        printf("True LRU: %x next bit to check: %d \n",packLRU->LRU,packLRU->Bits);
        --packLRU->levels;        //indicate next lower level
         printf("Bits to check: %d \n",packLRU->levels);
        if (packLRU->levels>0)
        {
            packLRU->victim = packLRU->victim<<1;
            SET_BIT(packLRU->victim,0);
            decode_LRU( packLRU);
        }else{
            packLRU->victim = packLRU->victim<<1;
            packLRU->victim |= 1 << 0;
            printf("Victim Value: %d \n",packLRU->victim);
            return;
        }
    }else{
        packLRU->Bits=(packLRU->Bits*2)+1;
        printf("False LRU: %x next bit to check:%d \n",packLRU->LRU,packLRU->Bits);
        --packLRU->levels;  //indicate next lower level
        printf("Bits to check: %d \n",packLRU->levels);
        if (packLRU->levels>0)
        {
            packLRU->victim=packLRU->victim<<1;
            decode_LRU(packLRU);
        }else{
            packLRU->victim=packLRU->victim<<1;
            printf("Victim Value: %d \n",packLRU->victim);
            return;
        }
    }
}


 void update_LRU(uint8_t line, LRU_Data* packLRU )
{
    int n=1;
    int MIDPOINT;
    int levels, bit;
    packLRU->levels=log2 (WAYS);
    packLRU->MIDPOINT=WAYS/(pow(2,n));
    
    if (line< packLRU->MIDPOINT) {
        // set bit 0
        ++n;
        packLRU->MIDPOINT=WAYS/(pow(2,n));
       
        printf("Go to Left leftSide\n");
        
    }else {
        // clear bit 0
        ++n;
        packLRU->MIDPOINT=WAYS/(pow(2,n));
        printf("Go to Right side\n",packLRU);
        GoRightSide(line, packLRU, n);
        printf("MIDPOINT=%d", packLRU->MIDPOINT);
        printf("\n\n");
    }
}

void GoRightSide(int line ,LRU_Data* packLRU, int n)

{
    packLRU->MIDPOINT=+(WAYS/(pow(2,n)));
    if(line< packLRU->MIDPOINT&& packLRU->levels>=0)
    {
       printf("MIDPOINT=%d", packLRU->MIDPOINT);
    }
    else if(packLRU->levels>=0){
        printf("MIDPOINT=%d", packLRU->MIDPOINT);
    }
}


