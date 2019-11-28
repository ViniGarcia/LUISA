#ifndef MODEL_HPP
#define MODEL_HPP

#include <unistd.h>
#include <string.h>
#include "LUISAType.h"
extern "C"{
#include "ENTROPY/FSECompressor8Bits.h"
#include "ENTROPY/Huff0Compressor8Bits.h"
}

BOOL  _STDCALL StartSubAllocator(UINT SubAllocatorSize);
void  _STDCALL StopSubAllocator();
DWORD _STDCALL GetUsedMemory();

typedef enum MR_METHOD { MRM_RESTART, MRM_CUT_OFF, MRM_FREEZE } MR_METHOD;

void _STDCALL EncodeFile(FILE* EncodedFile, FILE* DecodedFile, int EntropyCoder, int MaxOrder, int BlockBytes, MR_METHOD MRMethod);
void _STDCALL DecodeFile(FILE* DecodedFile, FILE* EncodedFile, int EntropyCoder, int MaxOrder, int BlockBytes, MR_METHOD MRMethod);

#endif
