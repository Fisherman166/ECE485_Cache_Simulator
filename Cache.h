/* Header for internal cache structure
** ECE 485/585 Cache Simulation Project
** Fall 2014
** Luis Santiago, Sean Koppenhafer, Steven Pierce
*/

#include <stdint.h>

#ifndef CACHE_H
#define CACHE_H

#define ways 16

/* Holds all variables stored in a cache line */
struct cache_line {
	uint16_t tag;
	uint8_t MESIF;
};

/* Holds all variables stored in a set */
struct cache_set {
	uint16_t psuedo_LRU;
	cache_line set_line[ways];
};
