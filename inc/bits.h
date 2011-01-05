//	designer - third party architectural design utility for 'Dwarf Fortress'
//	Copyright (C) 2010 Edward Cree (see top of src/designer.c for license details)
//	 bits.h - header for miscellaneous utility functions
#pragma once

#include <stdio.h>
#include <stdlib.h>

#define max(a,b)	((a)>(b)?(a):(b))
#define min(a,b)	((a)<(b)?(a):(b))

const char **xatiles; // populated with array of 128 unicode strings representing the high half of 'codepage 437'

void init437(void); // fill out xatiles
char * getl(FILE *); // get a line of string data, realloc()ing as it goes, so you don't need to make a buffer for it, nor must thee fret thyself about overruns!
