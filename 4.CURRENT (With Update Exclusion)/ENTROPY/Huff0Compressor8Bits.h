#ifndef HUFF0_COMPRESSOR8BITS_H
#define HUFF0_COMPRESSOR8BITS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huf.h"

#ifndef BLOCKSIZE
#define BLOCKSIZE 131072
#endif

//FILE COMPRESSOR
void Huff0C8BFileCompress(char *InputName, char *OutputName);
//FILE DECOMPRESSOR
void Huff0C8BFileDecompress(char *InputName, char *OutputName);
//BLOCK COMPRESSOR
void Huff0C8BBlockCompress(unsigned char *InData, unsigned int ReadSize, const unsigned int BlockSize, FILE *Output);
void Huff0C8BBlockEnd(FILE *Output, const unsigned int BlockSize);
//BLOCK DECOMPRESSOR
unsigned int Huff0C8BBlockDecompress(unsigned char *OutData, const unsigned int BlockSize, FILE *Input);

#endif
