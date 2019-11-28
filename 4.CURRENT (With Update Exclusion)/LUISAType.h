#ifndef LUISATYPE_H
#define LUISATYPE_H

#define _WIN32_ENVIRONMENT_
#include <windows.h>

#include <stdio.h>
#define _USE_PREFETCHING

#define _FASTCALL __fastcall
#define _STDCALL  __stdcall

#if defined(__GNUC__)
#define _PACK_ATTR __attribute__ ((packed))
#else
#define _PACK_ATTR
#endif

typedef int   BOOL;
#define FALSE 0
#define TRUE  1
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned int   UINT;

const DWORD PPMdSignature=0x84ACAF8F, Variant='I';
const int MAX_O=16;

#endif
