/******************************************************************************
 ******************************************************************************
 * J.S. PEIRCE       jpei2@pdx.edu
 * SEAN KOPPENHAFER  koppen2@pdx.edu
 * LUIS SANTIAGO     lsantiag@pdx.edu
 *
 *
 * ECE485 FINAL PROJECT
 *
 *   LRU.h
 ******************************************************************************
 *****************************************************************************/
//
//  This file Created by LUIS SANTIAGO on 11/11/14.
//
//

#ifndef _LRU_h
#define _LRU_h

// used to update the LRU
// input Set Pseudo_LRU and the index of the line
// on te set used.
// reurns an updated LRU value
uint16_t update_LRU( uint8_t index, uint16_t LRU);

// returns the leat recently used line on the set
// using P_LRU
uint8_t LeastUsed(uint16_t LRU);


#endif
