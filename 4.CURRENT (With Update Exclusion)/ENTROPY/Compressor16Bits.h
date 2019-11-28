#ifndef COMPRESSOR16BITS_H
#define COMPRESSOR16BITS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fse.h"
#include "fseU16.h"

#ifndef BLOCKSIZE
#define BLOCKSIZE 600000
#endif

//BLOCK COMPRESSOR
void C16BBlockCompress(unsigned short *InData, unsigned char *OutData, unsigned int ReadSize, unsigned int MaxSymbol, FILE *Output);
void C16BBlockEnd(FILE *Output);
//BLOCK DECOMPRESSOR
unsigned int C16BBlockDecompress(unsigned short *OutData, FILE *Input);

#endif
