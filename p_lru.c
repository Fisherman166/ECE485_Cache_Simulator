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
    
    // Update to point to leat recently used
    // each case must update 4 bit values
    switch (index){
        case 0:
            // none set to zero all set to 1
            LRU|=(0x4045);  // bit, 14,6,2,0 are set to 1
            break;
            
        case 1:
            LRU &= ~(1 << 0);   // Clear bit 0
            LRU|=(0x4044);      // set bit, 2,6,14 to 1
            break;
            
        case 2:
            LRU &= ~(1 << 2) ;   // Clear bit 2
            LRU |=(0x4042);      // set bits 1,6,14 to 1
            break;
        case 3:
            LRU &= 0xFFF9;       // clear 1,2 equal to 0
            LRU |=(0x4040);      // set bit, 14,6 set to 1
            break;
        case 4:
            LRU &= ~(1 << 6);     // clear bit 6
            LRU |= 0x4028;       // set bit, 3,5,14 to 1
            break;
            
        case 5:
            LRU &= 0xFFB7;       // clear bit 3,6
            LRU |= 0x4020;     // set bit, 5,14
            break;
            
        case 6:
            LRU &= 0xFF9F;       // clear bit 5,6
            LRU |=0x4010 ;      // set bit, 4,14
            break;
            
        case 7:
            LRU &= 0xFF8F;      // cleart bit 4,5,6
            LRU |=(0x4000);    // set bit, 14 is set to 0
            break;
            //******************* Right side
        case 8:
            LRU &= ~(1 << 14);   // Clear bit 14
            LRU |= 0x2280;      // set bit, 7,9,13
            break;
            
        case 9:
            LRU &= 0xBF7F;     // Clear bit 7,14
            LRU |= 0x2200;      // set bit 9,13
            break;
            
        case 10:
            LRU &= 0xBDFF ;       // Clear bit 9,14
            LRU |=0x2100;      // set bits 8,13
            break;
        case 11:
            LRU &= 0xBCFF;       // clear 8,9,14
            LRU |=0x2000;      // set bit 13
            break;
        case 12:
            LRU &= 0x9FFF;         // clear bit 13,14
            LRU |= 0x1400;       // set bit, 10,12,
            break;
            
        case 13:
            LRU &= 0x9BFF;       // clear bit 10,13,14
            LRU |= 0x1000;     // set bit 12
            break;
            
        case 14:
            LRU &= 0x8FFF;       // clear bit 12,13,14
            LRU |=0x4010 ;      // set bit, 11
            break;
            
        case 15:
            LRU &= 0x87FF;      // cleart bit 11,12,13,14
            break;
            
            
            
    }
    return LRU;
}

//****************************************     Decode

uint8_t check_B0 (uint16_t LRU)
{
    if ((LRU & (1 << 0)))
    {
        return 1;
    }else{
        return 0;
    }
}


uint8_t check_B1 (uint16_t LRU)
{
    if ((LRU & (1 << 1 )))
    {
        return 3;
    }else{
        return 2;
    }
}

uint8_t check_B3 (uint16_t LRU)
{
    if ((LRU & (1 << 3)))
    {
        return 5;
    }else{
        return 4;
    }
}

uint8_t check_B4 (uint16_t LRU)
{
    if ((LRU & (1 << 4)))
    {
        return 7;
    }else{
        return 6;
    }
}

uint8_t check_B7 (uint16_t LRU)
{
    if ((LRU & (1 << 7)))
    {
        return 9;
    }else{
        return 8;
    }
}

uint8_t check_B8 (uint16_t LRU)
{
    if ((LRU & (1 << 8)))
    {
        return 11;
    }else{
        return 10;
    }
}

uint8_t check_B10 (uint16_t LRU)
{
    if ((LRU & (1 << 10)))
    {
        return 13;
    }else{
        return 12;
    }
}

uint8_t check_B11 (uint16_t LRU)
{
    if ((LRU & (1 << 11)))
    {
        return 15;
    }else{
        return 14;
    }
}

//********************************************* right side

uint8_t check_B12(uint16_t LRU)
{
    if ((LRU & (1 << 12)))
    {
        return check_B11(LRU);
    }else{
        return check_B10(LRU);
    }
}

uint8_t check_B9(uint16_t LRU)
{
    if ((LRU & (1 << 9)))
    {
        return check_B8(LRU);
    }else{
        return check_B7(LRU);
    }
}

uint8_t check_B13(uint16_t LRU)
{
    if ((LRU & (1 << 13)))
    {
        return check_B12(LRU);
    }else{
        return check_B9(LRU);
    }
}





// ***********************************  Left side

uint8_t check_B2(uint16_t LRU)
{
    if ((LRU & (1 << 2)))
    {
        return (uint8_t)check_B1(LRU);
    }else{
        return (uint8_t) check_B0(LRU);
    }
}

uint8_t check_B5(uint16_t LRU)
{
    if ((LRU & (1 << 5)))
    {
        return (uint8_t)check_B4(LRU);
    }else{
        return (uint8_t)check_B3(LRU);
    }
}

uint8_t check_B6(uint16_t LRU)
{
    if ((LRU & (1 << 6)))
    {
        return (uint8_t) check_B5(LRU);
    }else{
        return (uint8_t) check_B2(LRU);
    }
}







// *************************************top of tree

// Function decode the LRU bits and Reuturns 
// the index of the 
uint8_t LeastUsed(uint16_t LRU)
{
    
    if ((LRU & (1 << 14)))
    {
        return check_B13(LRU);
    }else{
        return check_B6(LRU);
    }
    
    
}



//int main()
//{
//    uint8_t line, i;
//    uint16_t LRU;
//    uint8_t victim;
//	 cache_set test_set;
//
//	 /* Init*/
//	 test_set.psuedo_LRU = 0;
//	 test_set.valid_ways = 0;
//
//	 /* Fill up all 16 ways */
//	 for(i = 0; i < WAYS; i++) {
//	 	test_set.psuedo_LRU = update_LRU(i, test_set.psuedo_LRU);
//		test_set.valid_ways++;
//		printf("On init %d, LRU = %X\n", i, test_set.psuedo_LRU);
//	}
//
//	/* Evict all lines in order */
//	for(i = 0; i < WAYS; i++) {
//		victim = LeastUsed(test_set.psuedo_LRU);
//		test_set.psuedo_LRU = update_LRU(victim, test_set.psuedo_LRU);
//		printf("Victim is index %d\n", victim);
//	}
//
//
//    /*printf( "Enter LRU value: ");
//    scanf("%ux", &LRU);
//    // just a test 
//    do
//      {
//    printf( "Enter line to be updated: ");
//    scanf("%d", &line);
//    LRU= update_LRU (line,LRU);
//    printf( "LRU:0x%x \t line: %u \n\n", LRU,  line);
//    victim= LeastUsed(LRU);
//    printf( "Last used: %u \n\n",  victim);
//      }while (line <16);*/
//    
//    return 0;
//}

