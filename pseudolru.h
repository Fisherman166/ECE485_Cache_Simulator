//
//  pseudolru.h
//  
//
//  Created by LUIS SANTIAGO on 11/21/14.
//
//

#ifndef ____pseudolru__
#define ____pseudolru__

#include "cache.h"

/* Internal datatype */
typedef struct {
    uint32_t victim;        // initiate to 0 and it is used to generate
                            // binary value of Least recently used
    uint32_t levels;        // saves Log2(Ways) used to count leves
                            // on tree and determine when done
    uint32_t MIDPOINT;      // Value to determine walk direction
    uint8_t Bits;          // index of bits in LRU
    uint64_t LRU;           // Stores passed LRU (only for simplicity)
    uint8_t n;             // used to calculate ways
}LRU_Data;

/* Global functions */

// Used to update the LRU bits
// Input set Pseudo_LRU and the index of the line
// used referenced in the set
// Returns an updated LRU value
uint64_t update_LRU(uint8_t index, uint64_t LRU);

// returns the least recently used line in the set
// using P_LRU
uint8_t FindVictim(uint64_t LRU);


/* Internal functions */
void decode_LRU(LRU_Data* packLRU);
void GoRight(LRU_Data* packLRU);
void GoLeft(LRU_Data* packLRU);
void Check_Tree(LRU_Data* packLRU, uint32_t line);

#endif /* defined(____pseudolru__) */
