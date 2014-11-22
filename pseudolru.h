//
//  pseudolru.h
//  
//
//  Created by LUIS SANTIAGO on 11/21/14.
//
//

#ifndef ____pseudolru__
#define ____pseudolru__

// used to update the LRU
// input Set Pseudo_LRU and the index of the line
// on te set used.
// reurns an updated LRU value
uint64_t update_LRU( uint8_t index, uint64_t LRU);

// returns the least recently used line on the set
// using P_LRU
uint8_t FindVictim(uint64_t LRU);


#endif /* defined(____pseudolru__) */
