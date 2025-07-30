//
//  ScnMemMap.h
//
//  Created by Marcos Ortega on 20/07/25.
//  Copyright (c) 2014 Marcos Ortega. All rights reserved.
//

#ifndef SCN_UTILS_MEM_MAP_H
#define SCN_UTILS_MEM_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------
//-- Usefull implementation for memory leaking
//-- detection and tracking.
//---------------------------------------------

//STScnMemStrs

typedef struct STScnMemStrs_ {
    char*            buff;
    unsigned long    buffSz;
    unsigned long    buffUse;
    unsigned long*   idxs;
    unsigned long    idxsSz;
    unsigned long    idxsUse;
} STScnMemStrs;

void ScnMemStrs_init(STScnMemStrs* obj);
void ScnMemStrs_destroy(STScnMemStrs* obj);
unsigned long ScnMemStrs_find(STScnMemStrs* obj, const char* str);
unsigned long ScnMemStrs_findOrAdd(STScnMemStrs* obj, const char* str);

//STScnPtrRetainer

typedef struct STScnPtrRetainer_ {
    unsigned long   iStrHint;
    long            balance; //(retain - release)
} STScnPtrRetainer;

void ScnPtrRetainer_init(STScnPtrRetainer* obj);
void ScnPtrRetainer_destroy(STScnPtrRetainer* obj);

//STScnMemBlock

typedef struct STScnMemBlock_ {
    char            regUsed;
    unsigned long   iStrHint;
    void*           pointer;
    unsigned long   bytes;
    //retains
    struct {
        STScnPtrRetainer*   arr;
        unsigned long       sz;
        unsigned long       use;
        long                balance; //(retain - release)
    } retains;
} STScnMemBlock;

void ScnMemBlock_init(STScnMemBlock* obj);
void ScnMemBlock_destroy(STScnMemBlock* obj);
void ScnMemBlock_retainedBy(STScnMemBlock* obj, const unsigned long iStrHint);
void ScnMemBlock_releasedBy(STScnMemBlock* obj, const unsigned long iStrHint);

//STScnMemBlocks

typedef struct STScnMemBlocks_ {
    STScnMemBlock*   arr;
    unsigned long    arrSz;
    unsigned long    arrUse;
} STScnMemBlocks;

void ScnMemBlocks_init(STScnMemBlocks* obj);
void ScnMemBlocks_destroy(STScnMemBlocks* obj);
STScnMemBlock* ScnMemBlocks_findExact(STScnMemBlocks* obj, void* pointer);
STScnMemBlock* ScnMemBlocks_findContainer(STScnMemBlocks* obj, void* pointer);
STScnMemBlock* ScnMemBlocks_add(STScnMemBlocks* obj, void* pointer, unsigned long bytes, const unsigned long iStrHint);

//STScnMemStats

typedef struct STScnMemStats_ {
    //alive (blocks of memory currently alive)
    struct {
        unsigned long   count;
        unsigned long   bytes;
    } alive;
    //total (ammount allocated during program execution)
    struct {
        unsigned long   count;
        unsigned long   bytes;
    } total;
    //max (max during any point of execution)
    struct {
        unsigned long   count;
        unsigned long   bytes;
    } max;
} STScnMemStats;

void ScnMemStats_init(STScnMemStats* obj);
void ScnMemStats_destroy(STScnMemStats* obj);

//STScnMemMap

typedef struct STScnMemMap_ {
    STScnMemStats   stats;
    STScnMemStrs    strs;
    STScnMemBlocks  blocks;
} STScnMemMap;

void ScnMemMap_init(STScnMemMap* obj);
void ScnMemMap_destroy(STScnMemMap* obj);
//mem
void ScnMemMap_ptrAdd(STScnMemMap* obj, void* pointer, unsigned long bytes, const char* strHint);
void ScnMemMap_ptrRemove(STScnMemMap* obj, void* pointer);
//dbg
void ScnMemMap_ptrRetainedBy(STScnMemMap* obj, void* pointer, const char* by);
void ScnMemMap_ptrReleasedBy(STScnMemMap* obj, void* pointer, const char* by);
//report
void ScnMemMap_printAlivePtrs(STScnMemMap* obj);
void ScnMemMap_printFinalReport(STScnMemMap* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
