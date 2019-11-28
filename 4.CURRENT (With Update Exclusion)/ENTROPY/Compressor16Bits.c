#include "Compressor16Bits.h"

void C16BBlockLenghtBurn(unsigned char *OutData, unsigned int Lenght){
    OutData[0] = (unsigned char) Lenght;
    OutData[1] = (unsigned char)(Lenght>>8);
    OutData[2] = (unsigned char)(Lenght>>16);
    OutData[3] = (unsigned char)(Lenght>>24);
}

//=============================================== BLOCK COMPRESSOR ================================================

//InData -> (Size: BLOCKSIZE) Array with data to compress.
//ReadSize -> Many InData positions with data.
//Output -> Compressed file to burn data.
//To increase the dictionary size, you must change FSEU16_MAX_SYMBOL_VALUE in fseU16.h
//and change the 5º argument in FSE_compressU16 here.
void C16BBlockCompress(unsigned short *InData, unsigned char *OutData, unsigned int ReadSize, unsigned int MaxSymbol, FILE *Output){
    unsigned int BlockLenght;

    BlockLenght = FSE_compressU16(&OutData[4], BLOCKSIZE, InData, ReadSize, MaxSymbol, 0);
    C16BBlockLenghtBurn(OutData, BlockLenght);

    switch(BlockLenght){
    case 0:
        C16BBlockLenghtBurn(&OutData[4], ReadSize);
        fwrite(OutData, 1, 8, Output);
        fwrite(InData, 2, ReadSize, Output);
        return;

    case 1:
        C16BBlockLenghtBurn(&OutData[4], ReadSize);
        OutData[8] = InData[0];
        fwrite(OutData, 1, 9, Output);
        return;

    default:
        fwrite(OutData, 1, BlockLenght + 4, Output);
        return;
    }
}

void C16BBlockEnd(FILE *Output){
    unsigned char OutData[4];

    C16BBlockLenghtBurn(OutData, BLOCKSIZE+1);
    fwrite(OutData, 1, 4, Output);
}

//============================================== BLOCK DECOMPRESSOR ===============================================

//OutData -> (Size: BLOCKSIZE) Clean array to save original data.
//Input -> Compressed file to get information.
unsigned int C16BBlockDecompress(unsigned short *OutData, FILE *Input){
    unsigned char InData[BLOCKSIZE];
    unsigned int BlockLenght, DecompressedSize;

    fread(InData, 1, 4, Input);
    BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));

    switch(BlockLenght){
    case BLOCKSIZE+1:
        return -1;

    case 0:
        fread(InData, 1, 4, Input);
        BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));
        fread(OutData, 2, BlockLenght, Input);
        return BlockLenght;

    case 1:
        fread(InData, 1, 4, Input);
        BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));
        fread(InData, 1, 1, Input);
        memset(OutData, InData[0], BlockLenght);
        return BlockLenght;

    default:
        fread(InData, 1, BlockLenght, Input);
        DecompressedSize = FSE_decompressU16(OutData, BLOCKSIZE, InData, BlockLenght);
        return DecompressedSize;
    }
}
