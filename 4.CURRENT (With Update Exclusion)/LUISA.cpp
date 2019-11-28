#include "Model.hpp"

inline void Encode(char *InputFileName, char *OutputFileName, unsigned char EntropyCoder, unsigned char ContextAmount, unsigned char MemoryAmount, unsigned int BlockBytes, MR_METHOD MemoryMethod){

    //===================== ETAPA DE AJUSTE DO CODIFICADOR =====================
    if(access(InputFileName, F_OK) == -1){
        exit(-1);
    }
    FILE *InputFile = fopen(InputFileName, "rb");
    setvbuf(InputFile, NULL, _IOFBF, 64*1024);

    if(access(OutputFileName, W_OK) != -1){
        remove(OutputFileName);
    }
    FILE* OutputFile = fopen(OutputFileName, "a+b");
    setvbuf(OutputFile, NULL, _IOFBF, 64*1024);

    fputc(EntropyCoder, OutputFile);
    fputc(ContextAmount, OutputFile);
    fputc(MemoryAmount, OutputFile);
    fputc(MemoryMethod, OutputFile);
    fputc(BlockBytes, OutputFile);
    fputc((BlockBytes>>8), OutputFile);
    fputc((BlockBytes>>16), OutputFile);
    fputc((BlockBytes>>24), OutputFile);
    printf("SUMMARY: (-c) (-o %d) (-m %d) (-r %d) (-e %d) (-b %d)", ContextAmount, MemoryAmount, MemoryMethod, EntropyCoder, BlockBytes);
    //============================================================================

    //===================== ETAPA DE EXECUÇÃO DA CODIFICAÇÃO =====================
    StartSubAllocator(MemoryAmount);
    EncodeFile(OutputFile, InputFile, (int) EntropyCoder, (int) ContextAmount, (int) BlockBytes, MemoryMethod);
    //============================================================================

    //===================== ETAPA DE CONCLUSÃO DA CODIFICAÇÃO ====================
    fclose(InputFile);
    fclose(OutputFile);
    //============================================================================
}

inline void Decode(char *InputFileName, char *OutputFileName){

    //===================== ETAPA DE AJUSTE DO DECODIFICADOR =====================
    if(access(InputFileName, F_OK) == -1){
        exit(-1);
    }
    FILE *InputFile = fopen(InputFileName, "rb");
    setvbuf(InputFile, NULL, _IOFBF, 64*1024);

    unsigned char EntropyCoder = getc(InputFile);
    unsigned char ContextAmount = getc(InputFile);
    unsigned char MemoryAmount = getc(InputFile);
    MR_METHOD MemoryMethod = (MR_METHOD) getc(InputFile);
    unsigned int BlockBytes = (((getc(InputFile) | (getc(InputFile) << 8)) | (getc(InputFile) << 16)) | (getc(InputFile) << 24));
    printf("SUMMARY: (-d) (-o %d) (-m %d) (-r %d) (-e %d) (-b %d)", ContextAmount, MemoryAmount, MemoryMethod, EntropyCoder, BlockBytes);

    if(access(OutputFileName, W_OK) != -1){
        remove(OutputFileName);
    }
    FILE* OutputFile = fopen(OutputFileName, "wb");
    setvbuf(OutputFile, NULL, _IOFBF, 64*1024);
    //============================================================================

    //===================== ETAPA DE EXECUÇÃO DA DECODIFICAÇÃO =====================
    StartSubAllocator(MemoryAmount);
    DecodeFile(OutputFile, InputFile, (int) EntropyCoder, (int) ContextAmount, (int) BlockBytes, MemoryMethod);
    //============================================================================

    //===================== ETAPA DE CONCLUSÃO DA DECODIFICAÇÃO ====================
    fclose(InputFile);
    fclose(OutputFile);
    //============================================================================
}

void Help(){
    printf("\n========================================================================\n");
    printf("                                    HELP                                \n");
    printf("========================================================================\n");
    printf("Coding: *.exe -c input output (optional)                                \n");
    printf("\toptional:                                                             \n");
    printf("\t\t-o orders -> Number of context orders [1..255 - std:4]              \n");
    printf("\t\t-m memory -> Maximum allowed memory in MB [1..255 - std: 10]        \n");
    printf("\t\t-r type -> Context tree overflow rescue method [std: Restart]       \n");
    printf("\t\t  0 -> Restart                                                      \n");
    printf("\t\t  1 -> Remodel                                                      \n");
    printf("\t\t  2 -> Freeze                                                       \n");
    printf("\t\t-e coder -> Entropy encoder algorithm [std: FSE]                    \n");
    printf("\t\t  0 -> FSE                                                          \n");
    printf("\t\t  1 -> Huff0                                                        \n");
    printf("\t\t-b block -> Entropy encoder block in bytes [1..1KK - std: 125000]   \n");
    printf("Decoding: *.exe -d input output                                         \n");
    printf("========================================================================\n");
}

int main(int argc, char *argv[]){
    unsigned char EntropyCoder = 0;
    unsigned char ContextAmount = 4;
    unsigned char MemoryAmount = 10;
    unsigned int BlockBytes = 125000;
    MR_METHOD MemoryMethod = MRM_RESTART;

    unsigned char flags[5] = {0, 0, 0, 0, 0};
    if (argc < 4){ Help(); return -1; }
    if ((argc > 12) || ((argc > 4) && (strcmp(argv[1], "-d") == 0))){ Help(); return -1; }
    if ((strcmp(argv[1], "-c") != 0) && (strcmp(argv[1], "-d") != 0)){ Help(); return -1; }
    if (strcmp(argv[1], "-c") == 0){
        if (argc % 2 == 1){ Help(); return -1; }
        for (int i = 4; i < argc; i+=2){
            if (strcmp(argv[i], "-o") == 0){
                if (flags[0]){ Help(); return -1; }
                ContextAmount = (unsigned char) atoi(argv[i+1]);
                if ((ContextAmount < 1) || (ContextAmount > 255)){ Help(); return -1; }
                flags[0] = 1;
                continue;
            }
            if (strcmp(argv[i], "-m") == 0){
                if (flags[1]){ Help(); return -1; }
                MemoryAmount = (unsigned char) atoi(argv[i+1]);
                if ((MemoryAmount < 1) || (MemoryAmount > 255)){ Help(); return -1; }
                flags[1] = 1;
                continue;
            }
            if (strcmp(argv[i], "-b") == 0){
                if (flags[2]){ Help(); return -1; }
                BlockBytes = (unsigned int) atoi(argv[i+1]);
                if ((BlockBytes < 1) || (BlockBytes > 1000000)){ Help(); return -1; }
                flags[2] = 1;
                continue;
            }
            if (strcmp(argv[i], "-r") == 0){
                if (flags[3]){ Help(); return -1; }
                unsigned char MemoryMethodID = (unsigned char) atoi(argv[i+1]);
                if ((MemoryMethodID < 0) || (MemoryMethodID > 2)){ Help(); return -1; }
                if (MemoryMethodID == 1) MemoryMethod = MRM_CUT_OFF;
                else if (MemoryMethodID == 2) MemoryMethod = MRM_FREEZE;
                flags[3] = 1;
                continue;
            }
            if (strcmp(argv[i], "-e") == 0){
                if (flags[4]){ Help(); return -1; }
                EntropyCoder = (unsigned char) atoi(argv[i+1]);
                if ((EntropyCoder < 0) || (EntropyCoder > 1)){ Help(); return -1; }
                flags[4] = 1;
                continue;
            }
            Help();
            return -1;
        }
    }

    if (strcmp(argv[1], "-c") == 0){
        Encode(argv[2], argv[3], EntropyCoder, ContextAmount, MemoryAmount, BlockBytes, MemoryMethod);
    }
    else{
        Decode(argv[2], argv[3]);
    }

}
