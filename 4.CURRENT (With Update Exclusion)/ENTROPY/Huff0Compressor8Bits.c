#include "Huff0Compressor8Bits.h"

void Huff0C8BBlockLenghtBurn(unsigned char *OutData, unsigned int Lenght){
    OutData[0] = (unsigned char) Lenght;
    OutData[1] = (unsigned char)(Lenght>>8);
    OutData[2] = (unsigned char)(Lenght>>16);
    OutData[3] = (unsigned char)(Lenght>>24);
}

//================================================ FILE COMPRESSOR ================================================

void Huff0C8BFileCompress(char *InputName, char *OutputName){
    FILE *Input, *Output;
    unsigned char InData[BLOCKSIZE], OutData[BLOCKSIZE+8];
    unsigned int BlockLenght, ReadSize;

    Input = fopen(InputName, "rb");
    Output = fopen(OutputName, "wb+");

    for (ReadSize = fread(InData, 1, BLOCKSIZE, Input); ReadSize > 0; ReadSize = fread(InData, 1, BLOCKSIZE, Input)){
        BlockLenght = HUF_compress(&OutData[8], BLOCKSIZE, InData, ReadSize);
        Huff0C8BBlockLenghtBurn(OutData, BlockLenght);
        Huff0C8BBlockLenghtBurn(&OutData[4], ReadSize);

        switch(BlockLenght){
        case 0:
            fwrite(OutData, 1, 8, Output);
            fwrite(InData, 1, ReadSize, Output);
            break;

        case 1:
            OutData[8] = InData[0];
            fwrite(OutData, 1, 9, Output);
            break;

        default:
            fwrite(OutData, 1, BlockLenght + 8, Output);
            break;
        }
    }
    Huff0C8BBlockLenghtBurn(OutData, BLOCKSIZE+1);
    fwrite(OutData, 1, 4, Output);

    fclose(Input);
    fclose(Output);
}

//=============================================== FILE DECOMPRESSOR ===============================================

void Huff0C8BFileDecompress(char *InputName, char *OutputName){
    FILE *Input, *Output;
    unsigned char InData[BLOCKSIZE], OutData[BLOCKSIZE];
    unsigned int BlockLenght, DecompressedSize;

    Input = fopen(InputName, "rb");
    Output = fopen(OutputName, "wb+");

    for(fread(InData, 1, 8, Input); 1; fread(InData, 1, 8, Input)){
        BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));
        DecompressedSize = ((((InData[7] << 24) | (InData[6] << 16)) | (InData[5] << 8)) | (InData[4]));

        switch(BlockLenght){
        case BLOCKSIZE+1:
            goto END;

        case 0:
            fread(InData, 1, DecompressedSize, Input);
            fwrite(InData, 1, DecompressedSize, Output);
            break;

        case 1:
            fread(InData, 1, 1, Input);
            memset(OutData, InData[0], DecompressedSize);
            fwrite(OutData, 1, DecompressedSize, Output);
            break;

        default:
            fread(InData, 1, BlockLenght, Input);
            HUF_decompress(OutData, DecompressedSize, InData, BlockLenght);
            fwrite(OutData, 1, DecompressedSize, Output);
            break;
        }
    }

    END:
    fclose(Input);
    fclose(Output);
}

//=============================================== BLOCK COMPRESSOR ================================================

//InData -> (Size: BLOCKSIZE) Array with data to compress.
//ReadSize -> Many InData positions with data.
//Output -> Compressed file to burn data.
void Huff0C8BBlockCompress(unsigned char *InData, unsigned int ReadSize, const unsigned int BlockSize, FILE *Output){
    unsigned char OutData[BlockSize+8];
    unsigned int BlockLenght;

    BlockLenght = HUF_compress(&OutData[8], BlockSize, InData, ReadSize);
    Huff0C8BBlockLenghtBurn(OutData, BlockLenght);
    Huff0C8BBlockLenghtBurn(&OutData[4], ReadSize);

    switch(BlockLenght){
    case 0:
        fwrite(OutData, 1, 8, Output);
        fwrite(InData, 1, ReadSize, Output);
        return;

    case 1:
        OutData[8] = InData[0];
        fwrite(OutData, 1, 9, Output);
        return;

    default:
        fwrite(OutData, 1, BlockLenght + 8, Output);
        return;
    }
}

void Huff0C8BBlockEnd(FILE *Output, const unsigned int BlockSize){
    unsigned char OutData[4];

    Huff0C8BBlockLenghtBurn(OutData, BlockSize+1);
    fwrite(OutData, 1, 4, Output);
}

//============================================== BLOCK DECOMPRESSOR ===============================================

//OutData -> Clean array to save original data.
//Input -> Compressed file to get information.
unsigned int Huff0C8BBlockDecompress(unsigned char *OutData, const unsigned int BlockSize, FILE *Input){
    unsigned char InData[BlockSize];
    unsigned int BlockLenght, DecompressedSize;

    fread(InData, 1, 8, Input);
    BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));
    DecompressedSize = ((((InData[7] << 24) | (InData[6] << 16)) | (InData[5] << 8)) | (InData[4]));

    if (BlockLenght == BlockSize + 1) return -1;
    switch(BlockLenght){

    case 0:
        fread(OutData, 1, DecompressedSize, Input);
        return DecompressedSize;

    case 1:
        fread(InData, 1, 1, Input);
        memset(OutData, InData[0], DecompressedSize);
        return DecompressedSize;

    default:
        fread(InData, 1, BlockLenght, Input);
        HUF_decompress(OutData, DecompressedSize, InData, BlockLenght);
        return DecompressedSize;
    }
}

