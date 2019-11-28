#pragma hdrstop
#include "Model.hpp"
#include "SubAlloc.hpp"

int xxx;

enum { UP_FREQ=5, INT_BITS=7, PERIOD_BITS=7, TOT_BITS=INT_BITS+PERIOD_BITS,
    INTERVAL=1 << INT_BITS, BIN_SCALE=1 << TOT_BITS, MAX_FREQ=124, O_BOUND=9 };

#pragma pack(1)
static struct LUISA_CONTEXT {
    BYTE NumStats, Flags;
    WORD SummFreq;
    struct STATE {
        BYTE Symbol, Freq;
        LUISA_CONTEXT* Successor;
    } _PACK_ATTR * Stats;
    LUISA_CONTEXT* Suffix;
    inline void LUencodeBinSymbol(int symbol, int *position, unsigned char *EncodedArray);
    inline void LUencodeSymbol1(int symbol, int *position, unsigned char *EncodedArray);
    inline bool LUencodeSymbol3a(int symbol);
    inline void LUencodeSymbol3b(int symbol);
    inline void LUencodeSymbol3c(int symbol, int *position, unsigned char *EncodedArray);
    inline void LUdecodeBinSymbol(int symbol, int *position, unsigned char *DecodedArray);
    inline void LUdecodeSymbol1(int symbol, int *position, unsigned char *DecodedArray);
    inline void LUdecodeSymbol2(int symbol, int *position, unsigned char *DecodedArray);
    inline void           update1(STATE* p);
    inline void           update2(STATE* p);
    void                          rescale();
    void      refresh(int OldNU,BOOL Scale);
    LUISA_CONTEXT*          cutOff(int Order);
    LUISA_CONTEXT*  removeBinConts(int Order);
    STATE& oneState() const { return (STATE&) SummFreq; }
} _PACK_ATTR* MaxContext;
#pragma pack()

static BYTE NS2BSIndx[256], QTable[260];
static LUISA_CONTEXT::STATE* FoundState;
static int  InitEsc, OrderFall, MaxOrder;
static BYTE CharMask[256], NumMasked, EscCount, PrintCount;
static MR_METHOD MRMethod;

inline void SWAP(LUISA_CONTEXT::STATE& s1,LUISA_CONTEXT::STATE& s2)
{
    WORD t1=(WORD&) s1;                     LUISA_CONTEXT* t2=s1.Successor;
    (WORD&) s1 = (WORD&) s2;                s1.Successor=s2.Successor;
    (WORD&) s2 = t1;                        s2.Successor=t2;
}

inline void StateCpy(LUISA_CONTEXT::STATE& s1,const LUISA_CONTEXT::STATE& s2)
{
    (WORD&) s1=(WORD&) s2;                  s1.Successor=s2.Successor;
}

struct PPMD_STARTUP { inline PPMD_STARTUP(); } PPMd_StartUp;

inline PPMD_STARTUP::PPMD_STARTUP()
{
    UINT i, k, m, Step;
    for (i=0,k=1;i < N1     ;i++,k += 1)    Indx2Units[i]=k;
    for (k++;i < N1+N2      ;i++,k += 2)    Indx2Units[i]=k;
    for (k++;i < N1+N2+N3   ;i++,k += 3)    Indx2Units[i]=k;
    for (k++;i < N1+N2+N3+N4;i++,k += 4)    Indx2Units[i]=k;
    for (k=i=0;k < 128;k++) {
        i += (Indx2Units[i] < k+1);         Units2Indx[k]=i;
    }
    NS2BSIndx[0]=2*0;                       NS2BSIndx[1]=2*1;
    memset(NS2BSIndx+2,2*2,9);              memset(NS2BSIndx+11,2*3,256-11);
    for (i=0;i < UP_FREQ;i++)               QTable[i]=i;
    for (m=i=UP_FREQ, k=Step=1;i < 260;i++) {
        QTable[i]=m;
        if ( !--k ) { k = ++Step;           m++; }
    }
}

static void _STDCALL StartModelRare(int MaxOrder,MR_METHOD MRMethod)
{
    UINT i, k, m;
    memset(CharMask,0,sizeof(CharMask));    EscCount=PrintCount=1;
    if (MaxOrder < 2) {
        OrderFall=::MaxOrder;
        for (LUISA_CONTEXT* pc=MaxContext;pc->Suffix != NULL;pc=pc->Suffix)
                OrderFall--;
        return;
    }
    OrderFall=::MaxOrder=MaxOrder;          ::MRMethod=MRMethod;
    InitSubAllocator();

    MaxContext = (LUISA_CONTEXT*) AllocContext();
    MaxContext->Suffix=NULL;
    MaxContext->SummFreq=(MaxContext->NumStats=255)+2;
    MaxContext->Stats = (LUISA_CONTEXT::STATE*) AllocUnits(256/2);
    for (i=0;i < 256;i++) {
        MaxContext->Stats[i].Symbol=i;      MaxContext->Stats[i].Freq=1;
        MaxContext->Stats[i].Successor=NULL;
    }


}

void LUISA_CONTEXT::refresh(int OldNU,BOOL Scale)
{
    int i=NumStats, EscFreq;               	Scale |= (SummFreq >= 32768);
    STATE* p = Stats = (STATE*) ShrinkUnits(Stats,OldNU,(i+2) >> 1);
    Flags=(Flags & (0x10+0x04*Scale))+0x08*(p->Symbol >= 0x40);
    EscFreq=SummFreq-p->Freq;
    SummFreq = (p->Freq=(p->Freq+Scale) >> Scale);
    do {
        EscFreq -= (++p)->Freq;
        SummFreq += (p->Freq=(p->Freq+Scale) >> Scale);
        Flags |= 0x08*(p->Symbol >= 0x40);
    } while ( --i );
    SummFreq += (EscFreq=(EscFreq+Scale) >> Scale);
}

#define P_CALL(F) ( PrefetchData(p->Successor), \
                    p->Successor=p->Successor->F(Order+1))

LUISA_CONTEXT* LUISA_CONTEXT::cutOff(int Order)
{
    int i, tmp;
    STATE* p;
    if ( !NumStats ) {
        if ((BYTE*) (p=&oneState())->Successor >= UnitsStart) {
            if (Order < MaxOrder)           P_CALL(cutOff);
            else                            p->Successor=NULL;
            if (!p->Successor && Order > O_BOUND)
                    goto REMOVE;
            return this;
        } else {
REMOVE:     SpecialFreeUnit(this);          return NULL;
        }
    }
    PrefetchData(Stats);
    Stats = (STATE*) MoveUnitsUp(Stats,tmp=(NumStats+2) >> 1);
    for (p=Stats+(i=NumStats);p >= Stats;p--)
            if ((BYTE*) p->Successor < UnitsStart) {
                p->Successor=NULL;          SWAP(*p,Stats[i--]);
            } else if (Order < MaxOrder)    P_CALL(cutOff);
            else                            p->Successor=NULL;
    if (i != NumStats && Order) {
        NumStats=i;                         p=Stats;
        if (i < 0) { FreeUnits(p,tmp);      goto REMOVE; }
        else if (i == 0) {
            Flags=(Flags & 0x10)+0x08*(p->Symbol >= 0x40);
            StateCpy(oneState(),*p);        FreeUnits(p,tmp);
            oneState().Freq=(oneState().Freq+11) >> 3;
        } else                              refresh(tmp,SummFreq > 16*i);
    }
    return this;
}

LUISA_CONTEXT* LUISA_CONTEXT::removeBinConts(int Order)
{
    STATE* p;
    if ( !NumStats ) {
        p=&oneState();
        if ((BYTE*) p->Successor >= UnitsStart && Order < MaxOrder)
                P_CALL(removeBinConts);
        else                                p->Successor=NULL;
        if (!p->Successor && (!Suffix->NumStats || Suffix->Flags == 0xFF)) {
            FreeUnits(this,1);              return NULL;
        } else                              return this;
    }
    PrefetchData(Stats);
    for (p=Stats+NumStats;p >= Stats;p--)
            if ((BYTE*) p->Successor >= UnitsStart && Order < MaxOrder)
                    P_CALL(removeBinConts);
            else                            p->Successor=NULL;
    return this;
}

static void RestoreModelRare(LUISA_CONTEXT* pc1,LUISA_CONTEXT* MinContext,
        LUISA_CONTEXT* FSuccessor)
{
    LUISA_CONTEXT* pc;
    LUISA_CONTEXT::STATE* p;
    for (pc=MaxContext, pText=HeapStart;pc != pc1;pc=pc->Suffix)
            if (--(pc->NumStats) == 0) {
                pc->Flags=(pc->Flags & 0x10)+0x08*(pc->Stats->Symbol >= 0x40);
                p=pc->Stats;                StateCpy(pc->oneState(),*p);
                SpecialFreeUnit(p);
                pc->oneState().Freq=(pc->oneState().Freq+11) >> 3;
            } else
                    pc->refresh((pc->NumStats+3) >> 1,FALSE);
    for ( ;pc != MinContext;pc=pc->Suffix)
            if ( !pc->NumStats )
                    pc->oneState().Freq -= pc->oneState().Freq >> 1;
            else if ((pc->SummFreq += 4) > 128+4*pc->NumStats)
                    pc->refresh((pc->NumStats+2) >> 1,TRUE);
    if (MRMethod > MRM_FREEZE) {
        MaxContext=FSuccessor;              GlueCount += !(BList[1].Stamp & 1);
    } else if (MRMethod == MRM_FREEZE) {
        while ( MaxContext->Suffix )        MaxContext=MaxContext->Suffix;
        MaxContext->removeBinConts(0);      MRMethod=MR_METHOD(MRMethod+1);
        GlueCount=0;                        OrderFall=MaxOrder;
    } else if (MRMethod == MRM_RESTART || GetUsedMemory() < (SubAllocatorSize >> 1)) {
        StartModelRare(MaxOrder,MRMethod);
        EscCount=0;                         PrintCount=0xFF;
    } else {
        while ( MaxContext->Suffix )        MaxContext=MaxContext->Suffix;
        do {
            MaxContext->cutOff(0);          ExpandTextArea();
        } while (GetUsedMemory() > 3*(SubAllocatorSize >> 2));
        GlueCount=0;                        OrderFall=MaxOrder;
    }
}

static LUISA_CONTEXT* _FASTCALL CreateSuccessors(BOOL Skip,LUISA_CONTEXT::STATE* p,
        LUISA_CONTEXT* pc);

static LUISA_CONTEXT* _FASTCALL ReduceOrder(LUISA_CONTEXT::STATE* p,LUISA_CONTEXT* pc)
{
    LUISA_CONTEXT::STATE* p1,  * ps[MAX_O+1], ** pps=ps;
    LUISA_CONTEXT* pc1=pc, * UpBranch = (LUISA_CONTEXT*) pText;
    BYTE tmp, sym=FoundState->Symbol;
    *pps++ = FoundState;                    FoundState->Successor=UpBranch;
    OrderFall++;
    if ( p ) { pc=pc->Suffix;               goto LOOP_ENTRY; }
    for ( ; ; ) {
        if ( !pc->Suffix ) {
            if (MRMethod > MRM_FREEZE) {
FROZEN:         do { (*--pps)->Successor = pc; } while (pps != ps);
                pText=HeapStart+1;          OrderFall=1;
            }
            return pc;
        }
        pc=pc->Suffix;
        if ( pc->NumStats ) {
            if ((p=pc->Stats)->Symbol != sym)
                    do { tmp=p[1].Symbol;   p++; } while (tmp != sym);
            tmp=2*(p->Freq < MAX_FREQ-9);
            p->Freq += tmp;                 pc->SummFreq += tmp;
        } else { p=&(pc->oneState());       p->Freq += (p->Freq < 32); }
LOOP_ENTRY:
        if ( p->Successor )                 break;
        *pps++ = p;                         p->Successor=UpBranch;
        OrderFall++;
    }
    if (MRMethod > MRM_FREEZE) {
        pc = p->Successor;                  goto FROZEN;
    } else if (p->Successor <= UpBranch) {
        p1=FoundState;                      FoundState=p;
        p->Successor=CreateSuccessors(FALSE,NULL,pc);
        FoundState=p1;
    }
    if (OrderFall == 1 && pc1 == MaxContext) {
        FoundState->Successor=p->Successor; pText--;
    }
    return p->Successor;
}

void LUISA_CONTEXT::rescale()
{
    UINT OldNU, Adder, EscFreq, i=NumStats;
    STATE tmp, * p1, * p;
    for (p=FoundState;p != Stats;p--)       SWAP(p[0],p[-1]);
    p->Freq += 4;                           SummFreq += 4;
    EscFreq=SummFreq-p->Freq;
    Adder=(OrderFall != 0 || MRMethod > MRM_FREEZE);
    SummFreq = (p->Freq=(p->Freq+Adder) >> 1);
    do {
        EscFreq -= (++p)->Freq;
        SummFreq += (p->Freq=(p->Freq+Adder) >> 1);
        if (p[0].Freq > p[-1].Freq) {
            StateCpy(tmp,*(p1=p));
            do StateCpy(p1[0],p1[-1]); while (tmp.Freq > (--p1)[-1].Freq);
            StateCpy(*p1,tmp);
        }
    } while ( --i );
    if (p->Freq == 0) {
        do { i++; } while ((--p)->Freq == 0);
        EscFreq += i;                       OldNU=(NumStats+2) >> 1;
        if ((NumStats -= i) == 0) {
            StateCpy(tmp,*Stats);
            tmp.Freq=(2*tmp.Freq+EscFreq-1)/EscFreq;
            if (tmp.Freq > MAX_FREQ/3)      tmp.Freq=MAX_FREQ/3;
            FreeUnits(Stats,OldNU);         StateCpy(oneState(),tmp);
            Flags=(Flags & 0x10)+0x08*(tmp.Symbol >= 0x40);
            FoundState=&oneState();         return;
        }
        Stats = (STATE*) ShrinkUnits(Stats,OldNU,(NumStats+2) >> 1);
        Flags &= ~0x08;                     i=NumStats;
        Flags |= 0x08*((p=Stats)->Symbol >= 0x40);
        do { Flags |= 0x08*((++p)->Symbol >= 0x40); } while ( --i );
    }
    SummFreq += (EscFreq -= (EscFreq >> 1));
    Flags |= 0x04;                          FoundState=Stats;
}

static LUISA_CONTEXT* _FASTCALL CreateSuccessors(BOOL Skip,LUISA_CONTEXT::STATE* p,
        LUISA_CONTEXT* pc)
{
    LUISA_CONTEXT ct, * UpBranch=FoundState->Successor;
    LUISA_CONTEXT::STATE* ps[MAX_O], ** pps=ps;
    UINT cf, s0;
    BYTE tmp, sym=FoundState->Symbol;
    if ( !Skip ) {
        *pps++ = FoundState;
        if ( !pc->Suffix )                  goto NO_LOOP;
    }
    if ( p ) { pc=pc->Suffix;               goto LOOP_ENTRY; }
    do {
        pc=pc->Suffix;
        if ( pc->NumStats ) {
            if ((p=pc->Stats)->Symbol != sym)
                    do { tmp=p[1].Symbol;   p++; } while (tmp != sym);
            tmp=(p->Freq < MAX_FREQ-9);
            p->Freq += tmp;                 pc->SummFreq += tmp;
        } else {
            p=&(pc->oneState());
            p->Freq += (!pc->Suffix->NumStats & (p->Freq < 24));
        }
LOOP_ENTRY:
        if (p->Successor != UpBranch) {
            pc=p->Successor;                break;
        }
        *pps++ = p;
    } while ( pc->Suffix );
NO_LOOP:
    if (pps == ps)                          return pc;
    ct.NumStats=0;                          ct.Flags=0x10*(sym >= 0x40);
    ct.oneState().Symbol=sym=*(BYTE*) UpBranch;
    ct.oneState().Successor=(LUISA_CONTEXT*) (((BYTE*) UpBranch)+1);
    ct.Flags |= 0x08*(sym >= 0x40);
    if ( pc->NumStats ) {
        if ((p=pc->Stats)->Symbol != sym)
                do { tmp=p[1].Symbol;       p++; } while (tmp != sym);
        s0=pc->SummFreq-pc->NumStats-(cf=p->Freq-1);
        ct.oneState().Freq=1+((2*cf <= s0)?(5*cf > s0):((cf+2*s0-3)/s0));
    } else
            ct.oneState().Freq=pc->oneState().Freq;
    do {
        LUISA_CONTEXT* pc1 = (LUISA_CONTEXT*) AllocContext();
        if ( !pc1 )                         return NULL;
        ((DWORD*) pc1)[0] = ((DWORD*) &ct)[0];
        ((DWORD*) pc1)[1] = ((DWORD*) &ct)[1];
        pc1->Suffix=pc;                     (*--pps)->Successor=pc=pc1;
    } while (pps != ps);
    return pc;
}

static inline void UpdateModel(LUISA_CONTEXT* MinContext)
{
    LUISA_CONTEXT::STATE* p=NULL;
    LUISA_CONTEXT* Successor, * FSuccessor, * pc, * pc1=MaxContext;
    UINT ns1, ns, cf, sf, s0, FFreq=FoundState->Freq;
    BYTE Flag, sym, FSymbol=FoundState->Symbol;
    FSuccessor=FoundState->Successor;       pc=MinContext->Suffix;
    if (FFreq < MAX_FREQ/4 && pc) {
        if ( pc->NumStats ) {
            if ((p=pc->Stats)->Symbol != FSymbol) {
                do { sym=p[1].Symbol;       p++; } while (sym != FSymbol);
                if (p[0].Freq >= p[-1].Freq) {
                    SWAP(p[0],p[-1]);       p--;
                }
            }
            cf=2*(p->Freq < MAX_FREQ-9);
            p->Freq += cf;                  pc->SummFreq += cf;
        } else { p=&(pc->oneState());       p->Freq += (p->Freq < 32); }
    }
    if (!OrderFall && FSuccessor) {
        FoundState->Successor=CreateSuccessors(TRUE,p,MinContext);
        if ( !FoundState->Successor )       goto RESTART_MODEL;
        MaxContext=FoundState->Successor;   return;
    }
    *pText++ = FSymbol;                     Successor = (LUISA_CONTEXT*) pText;
    if (pText >= UnitsStart)                goto RESTART_MODEL;
    if ( FSuccessor ) {
        if ((BYTE*) FSuccessor < UnitsStart)
                FSuccessor=CreateSuccessors(FALSE,p,MinContext);
    } else
                FSuccessor=ReduceOrder(p,MinContext);
    if ( !FSuccessor )                      goto RESTART_MODEL;
    if ( !--OrderFall ) {
        Successor=FSuccessor;               pText -= (MaxContext != MinContext);
    } else if (MRMethod > MRM_FREEZE) {
        Successor=FSuccessor;               pText=HeapStart;
        OrderFall=0;
    }
    s0=MinContext->SummFreq-(ns=MinContext->NumStats)-FFreq;
    for (Flag=0x08*(FSymbol >= 0x40);pc1 != MinContext;pc1=pc1->Suffix) {
        if ((ns1=pc1->NumStats) != 0) {
            if ((ns1 & 1) != 0) {
                p=(LUISA_CONTEXT::STATE*) ExpandUnits(pc1->Stats,(ns1+1) >> 1);
                if ( !p )                   goto RESTART_MODEL;
                pc1->Stats=p;
            }
            pc1->SummFreq += (3*ns1+1 < ns);
        } else {
            p=(LUISA_CONTEXT::STATE*) AllocUnits(1);
            if ( !p )                       goto RESTART_MODEL;
            StateCpy(*p,pc1->oneState());   pc1->Stats=p;
            if (p->Freq < MAX_FREQ/4-1)     p->Freq += p->Freq;
            else                            p->Freq  = MAX_FREQ-4;
            pc1->SummFreq=p->Freq+InitEsc+(ns > 2);
        }
        cf=2*FFreq*(pc1->SummFreq+6);       sf=s0+pc1->SummFreq;
        if (cf < 6*sf) {
            cf=1+(cf > sf)+(cf >= 4*sf);
            pc1->SummFreq += 4;
        } else {
            cf=4+(cf > 9*sf)+(cf > 12*sf)+(cf > 15*sf);
            pc1->SummFreq += cf;
        }
        p=pc1->Stats+(++pc1->NumStats);     p->Successor=Successor;
        p->Symbol = FSymbol;                p->Freq = cf;
        pc1->Flags |= Flag;
    }
    MaxContext=FSuccessor;                  return;
RESTART_MODEL:
    RestoreModelRare(pc1,MinContext,FSuccessor);
}

static const BYTE ExpEscape[16]={ 25,14, 9, 7, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2 };
#define GET_MEAN(SUMM,SHIFT,ROUND) ((SUMM+(1 << (SHIFT-ROUND))) >> (SHIFT))

inline void LUISA_CONTEXT::LUencodeBinSymbol(int symbol, int *position, unsigned char *EncodedArray){
    STATE& rs=oneState();

    if (rs.Symbol == symbol) {
        EncodedArray[*position] = (unsigned char) xxx;
        *position = *position + 1;
        FoundState = &rs;
        rs.Freq += (rs.Freq < 196);
    } else {
        CharMask[rs.Symbol] = EscCount;
        NumMasked = 0;
        FoundState = NULL;
    }
    xxx++;
}

inline void LUISA_CONTEXT::LUdecodeBinSymbol(int symbol, int *position, unsigned char *DecodedArray){
    STATE& rs=oneState();

    if (symbol == 0) {
        DecodedArray[*position] = rs.Symbol;
        *position = *position + 1;
        FoundState = &rs;
        rs.Freq += (rs.Freq < 196);
    } else {
        CharMask[rs.Symbol] = EscCount;
        FoundState = NULL;
        xxx++;
    }
}

inline void LUISA_CONTEXT::update1(STATE* p){
    (FoundState=p)->Freq += 4;
    SummFreq += 4;
    if (p[0].Freq > p[-1].Freq) {
        SWAP(p[0],p[-1]);
        FoundState = --p;
        if (p->Freq > MAX_FREQ) rescale();
    }
}

inline void LUISA_CONTEXT::LUencodeSymbol1(int symbol, int *position, unsigned char *EncodedArray){
    UINT i = Stats->Symbol;
    STATE* p = Stats;

    if (i == symbol) {
        EncodedArray[*position] = (unsigned char) xxx;
        *position = *position + 1;
        (FoundState=p)->Freq += 4;
        SummFreq += 4;
        if (p->Freq > MAX_FREQ) rescale();
        return;
    }

    i = NumStats;
    do {
        if ((++p)->Symbol == symbol) goto SYMBOL_FOUND;
        xxx++;
    } while ( --i );

    if ( Suffix ) PrefetchData(Suffix);
    NumMasked = NumStats;
    FoundState = NULL;
    return;

	SYMBOL_FOUND:
    EncodedArray[*position] = (unsigned char) (xxx + 1);
    *position = *position + 1;
    update1(p);
}

inline void LUISA_CONTEXT::LUdecodeSymbol1(int symbol, int *position, unsigned char *DecodedArray){
    STATE* p=Stats;

    if (symbol == 0) {
        DecodedArray[*position] = p->Symbol;
        *position = *position + 1;
        (FoundState = p)->Freq += 4;
        SummFreq += 4;
        if (p->Freq > MAX_FREQ) rescale();
        return;
    }

    xxx++;

    if (symbol<=NumStats && symbol!=-1){
        ++p;
        while (xxx != symbol) {
            xxx++;
            ++p;
        }
        DecodedArray[*position] = p->Symbol;
        *position = *position + 1;
        update1(p);
    }
    else {
        if ( Suffix ) PrefetchData(Suffix);
        FoundState = NULL;
    }
}

inline void LUISA_CONTEXT::update2(STATE* p)
{
    (FoundState=p)->Freq += 4;              SummFreq += 4;
    if (p->Freq > MAX_FREQ)                 rescale();
    EscCount++;
}

inline bool LUISA_CONTEXT::LUencodeSymbol3a(int symbol){
    UINT  i = NumStats+1;
    STATE* p = Stats-1;

    do {
        if (p++[1].Symbol == symbol) return true;
    } while ( --i );

    NumMasked = NumStats;

    return false;
}

inline void LUISA_CONTEXT::LUencodeSymbol3b(int symbol){
    UINT  i = NumStats+1;
    STATE* p = Stats-1;

    do {
        CharMask[p++[1].Symbol] = EscCount;
    } while ( --i );

    xxx = NumStats + 1;
 }

inline void LUISA_CONTEXT::LUencodeSymbol3c(int symbol, int *position, unsigned char *EncodedArray){
    UINT  Sym;
    STATE* p = Stats-1;

    xxx--;
    do {
        do {Sym=p++[1].Symbol;} while (CharMask[Sym] == EscCount);
        xxx++;
    } while (Sym != symbol);


    EncodedArray[*position] = (unsigned char) xxx;
    *position = *position + 1;
    update2(p);
}

inline void LUISA_CONTEXT::LUdecodeSymbol2(int symbol, int *position, unsigned char *DecodedArray){
	UINT Sym;
    STATE* p = Stats-1;

    xxx--;
    do {
        do { Sym=p[1].Symbol;   p++; } while (CharMask[Sym] == EscCount);
    } while (++xxx!=symbol);


    DecodedArray[*position] = p->Symbol;
    *position = *position + 1;
    update2(p);
}

inline void ClearMask(){
    EscCount = 1;
    memset(CharMask, 0, sizeof(CharMask));
}

void _STDCALL EncodeFile(FILE* EncodedFile, FILE* DecodedFile, int EntropyCoder, int MaxOrder, int BlockBytes, MR_METHOD MRMethod){
    unsigned char KeyArray[BlockBytes + (MaxOrder + 1)];
    unsigned char ByteArray[BlockBytes];
    int KeyBytes = 0, ReadBytes, Index;
    bool ok;
    LUISA_CONTEXT* ant;

    StartModelRare(MaxOrder, MRMethod);
    for (Index = 0, ReadBytes = fread(ByteArray, sizeof(char), BlockBytes, DecodedFile); ReadBytes > 0; Index = 0, ReadBytes = fread(ByteArray, sizeof(char), BlockBytes, DecodedFile)){
        for (LUISA_CONTEXT* MinContext; ; ){
            if (Index == ReadBytes) break;
            BYTE ns = (MinContext = MaxContext)->NumStats;
            int c = ByteArray[Index];
            Index++;
            xxx = 0;

            if (ns) {
                MinContext->LUencodeSymbol1(c, &KeyBytes, KeyArray);
                ant = MinContext;
            } else {
                MinContext->LUencodeBinSymbol(c, &KeyBytes, KeyArray);
                ant = NULL;
            }

            while (!FoundState){
                do {
                    OrderFall++;
                    MinContext = MinContext->Suffix;
                    if (!MinContext) goto STOP_ENCODING;
                } while (MinContext->NumStats == NumMasked);

                ok = MinContext->LUencodeSymbol3a(c);
                if (ok){
                    if (ant != NULL){
                        ant->LUencodeSymbol3b(c);
                    }
                    MinContext->LUencodeSymbol3c(c, &KeyBytes, KeyArray);
                }
                ant = MinContext;
            }

            if (KeyBytes >= BlockBytes){
                if (EntropyCoder == 0) FSEC8BBlockCompress(KeyArray, KeyBytes, BlockBytes, EncodedFile);
                else Huff0C8BBlockCompress(KeyArray, KeyBytes, BlockBytes, EncodedFile);
                KeyBytes = 0;
            }

            if (!OrderFall && (BYTE*) FoundState->Successor >= UnitsStart){
                PrefetchData(MaxContext = FoundState->Successor);
            }
            else {
                UpdateModel(MinContext);
                PrefetchData(MaxContext);
                if (EscCount == 0) ClearMask();
            }
        }
    }

    STOP_ENCODING:
    if (KeyBytes > 0){
        if (EntropyCoder == 0) FSEC8BBlockCompress(KeyArray, KeyBytes, BlockBytes, EncodedFile);
        else Huff0C8BBlockCompress(KeyArray, KeyBytes, BlockBytes, EncodedFile);
    }
    if (EntropyCoder == 0) FSEC8BBlockEnd(EncodedFile, BlockBytes);
    else Huff0C8BBlockEnd(EncodedFile, BlockBytes);
}

void _STDCALL DecodeFile(FILE* DecodedFile, FILE* EncodedFile, int EntropyCoder, int MaxOrder, int BlockBytes, MR_METHOD MRMethod){
    unsigned char KeyArray[BlockBytes + (MaxOrder + 1)];
    unsigned char ByteArray[BlockBytes + (MaxOrder + 1)];
    int KeyBytes = 0, WriteBytes = 0, Index = 0;

    StartModelRare(MaxOrder, MRMethod);
    for (LUISA_CONTEXT* MinContext; ;Index++) {
        BYTE ns = (MinContext = MaxContext)->NumStats;
        if (WriteBytes >= BlockBytes){
            fwrite(ByteArray, sizeof(char), WriteBytes, DecodedFile);
            WriteBytes = 0;
        }
        if (Index == KeyBytes){
            if (EntropyCoder == 0) KeyBytes = FSEC8BBlockDecompress(KeyArray, BlockBytes, EncodedFile);
            else KeyBytes = Huff0C8BBlockDecompress(KeyArray, BlockBytes, EncodedFile);
            if (KeyBytes == -1) goto STOP_ENCODING;
            Index = 0;
        }
        int c = (int) KeyArray[Index];
        xxx = 0;

        if (ns) {
            MinContext->LUdecodeSymbol1(c, &WriteBytes, ByteArray);
        } else {
            MinContext->LUdecodeBinSymbol(c, &WriteBytes, ByteArray);
        }

        if (!FoundState) {
            if (c == -1) goto STOP_ENCODING;
            LUISA_CONTEXT* ant = NULL;

            do {
                OrderFall++;
                ant = MinContext;
                MinContext = MinContext->Suffix;
            } while (c > MinContext->NumStats);

            if (ant && ant->NumStats){
                LUISA_CONTEXT::STATE* p = ant->Stats;
                CharMask[p->Symbol] = EscCount;
                UINT i = ant->NumStats;

                do {
                    CharMask[(++p)->Symbol]=EscCount;
                    xxx++;
                } while ( --i );
            }
            MinContext->LUdecodeSymbol2(c, &WriteBytes, ByteArray);
        }

        if (!OrderFall && (BYTE*) FoundState->Successor >= UnitsStart){
            PrefetchData(MaxContext=FoundState->Successor);
        }
        else {
            UpdateModel(MinContext);
            PrefetchData(MaxContext);
            if (EscCount == 0) ClearMask();
        }
    }

    STOP_ENCODING:
    if (WriteBytes > 0)fwrite(ByteArray, sizeof(char), WriteBytes, DecodedFile);
}
