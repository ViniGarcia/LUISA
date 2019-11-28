#ifndef COMPRESSOR8BITS_H
#define COMPRESSOR8BITS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fse.h"

#ifndef BLOCKSIZE
#define BLOCKSIZE 131072
#endif

//FILE COMPRESSOR
void FSEC8BFileCompress(char *InputName, char *OutputName);
//FILE DECOMPRESSOR
void FSEC8BFileDecompress(char *InputName, char *OutputName);
//BLOCK COMPRESSOR
void FSEC8BBlockCompress(unsigned char *InData, unsigned int ReadSize, const unsigned int BlockSize, FILE *Output);
void FSEC8BBlockEnd(FILE *Output, unsigned int BlockSize);
//BLOCK DECOMPRESSOR
unsigned int FSEC8BBlockDecompress(unsigned char *OutData, const unsigned int BlockSize, FILE *Input);

#endif
