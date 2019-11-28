#include "FSECompressor8Bits.h"

void FSEC8BBlockLenghtBurn(unsigned char *OutData, unsigned int Lenght){
    OutData[0] = (unsigned char) Lenght;
    OutData[1] = (unsigned char)(Lenght>>8);
    OutData[2] = (unsigned char)(Lenght>>16);
    OutData[3] = (unsigned char)(Lenght>>24);
}

//================================================ FILE COMPRESSOR ================================================

void FSEC8BFileCompress(char *InputName, char *OutputName){
    FILE *Input, *Output;
    unsigned char InData[BLOCKSIZE], OutData[BLOCKSIZE+4];
    unsigned int BlockLenght, ReadSize;

    Input = fopen(InputName, "rb");
    Output = fopen(OutputName, "wb+");

    for (ReadSize = fread(InData, 1, BLOCKSIZE, Input); ReadSize > 0; ReadSize = fread(InData, 1, BLOCKSIZE, Input)){
        BlockLenght = FSE_compress(&OutData[4], BLOCKSIZE, InData, ReadSize);
        FSEC8BBlockLenghtBurn(OutData, BlockLenght);

        switch(BlockLenght){
        case 0:
            FSEC8BBlockLenghtBurn(&OutData[4], ReadSize);
            fwrite(OutData, 1, 8, Output);
            fwrite(InData, 1, ReadSize, Output);
            break;

        case 1:
            FSEC8BBlockLenghtBurn(&OutData[4], ReadSize);
            OutData[8] = InData[0];
            fwrite(OutData, 1, 9, Output);
            break;

        default:
            fwrite(OutData, 1, BlockLenght + 4, Output);
            break;
        }
    }

    FSEC8BBlockLenghtBurn(OutData, BLOCKSIZE+1);
    fwrite(OutData, 1, 4, Output);

    fclose(Input);
    fclose(Output);
}

//=============================================== FILE DECOMPRESSOR ===============================================

void FSEC8BFileDecompress(char *InputName, char *OutputName){
    FILE *Input, *Output;
    unsigned char InData[BLOCKSIZE], OutData[BLOCKSIZE];
    unsigned int BlockLenght, DecompressedSize;

    Input = fopen(InputName, "rb");
    Output = fopen(OutputName, "wb+");

    for(fread(InData, 1, 4, Input); 1; fread(InData, 1, 4, Input)){
        BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));

        switch(BlockLenght){
        case BLOCKSIZE+1:
            goto END;

        case 0:
            fread(InData, 1, 4, Input);
            BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));
            fread(InData, 1, BlockLenght, Input);
            fwrite(InData, 1, BlockLenght, Output);
            break;

        case 1:
            fread(InData, 1, 4, Input);
            BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));
            fread(InData, 1, 1, Input);
            memset(OutData, InData[0], BlockLenght);
            fwrite(OutData, 1, BlockLenght, Output);
            break;

        default:
            fread(InData, 1, BlockLenght, Input);
            DecompressedSize = FSE_decompress(OutData, BLOCKSIZE, InData, BlockLenght);
            fwrite(OutData, 1, DecompressedSize, Output);
            break;
        }
    }

    END:
    fclose(Input);
    fclose(Output);
}

//=============================================== BLOCK COMPRESSOR ================================================

//InData -> Array with data to compress.
//ReadSize -> Many InData positions with data.
//Output -> Compressed file to burn data.
void FSEC8BBlockCompress(unsigned char *InData, unsigned int ReadSize, const unsigned int BlockSize, FILE *Output){
    unsigned char OutData[BlockSize+4];
    unsigned int BlockLenght;

    BlockLenght = FSE_compress(&OutData[4], BlockSize, InData, ReadSize);
    FSEC8BBlockLenghtBurn(OutData, BlockLenght);

    switch(BlockLenght){
    case 0:
        FSEC8BBlockLenghtBurn(&OutData[4], ReadSize);
        fwrite(OutData, 1, 8, Output);
        fwrite(InData, 1, ReadSize, Output);
        return;

    case 1:
        FSEC8BBlockLenghtBurn(&OutData[4], ReadSize);
        OutData[8] = InData[0];
        fwrite(OutData, 1, 9, Output);
        return;

    default:
        fwrite(OutData, 1, BlockLenght + 4, Output);
        return;
    }
}

void FSEC8BBlockEnd(FILE *Output, unsigned int BlockSize){
    unsigned char OutData[4];

    FSEC8BBlockLenghtBurn(OutData, BlockSize+1);
    fwrite(OutData, 1, 4, Output);
}

//============================================== BLOCK DECOMPRESSOR ===============================================

//OutData -> Clean array to save original data.
//Input -> Compressed file to get information.
unsigned int FSEC8BBlockDecompress(unsigned char *OutData, const unsigned int BlockSize, FILE *Input){
    unsigned char InData[BlockSize];
    unsigned int BlockLenght, DecompressedSize;

    fread(InData, 1, 4, Input);
    BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));

    if (BlockLenght == BlockSize + 1) return -1;
    switch(BlockLenght){

    case 0:
        fread(InData, 1, 4, Input);
        BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));
        fread(OutData, 1, BlockLenght, Input);
        return BlockLenght;

    case 1:
        fread(InData, 1, 4, Input);
        BlockLenght = ((((InData[3] << 24) | (InData[2] << 16)) | (InData[1] << 8)) | (InData[0]));
        fread(InData, 1, 1, Input);
        memset(OutData, InData[0], BlockLenght);
        return BlockLenght;

    default:
        fread(InData, 1, BlockLenght, Input);
        DecompressedSize = FSE_decompress(OutData, BlockSize, InData, BlockLenght);
        return DecompressedSize;
    }
}

