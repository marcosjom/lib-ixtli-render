//
//  ScnMemMap.c
//
//  Created by Marcos Ortega on 20/07/25.
//  Copyright (c) 2014 Marcos Ortega. All rights reserved.
//

#include <stdio.h>		//NULL
#include <stdlib.h>		//malloc, free
#include <string.h>		//memcpy, memset
#include <assert.h>		//assert

#include "../utils/ScnMemMap.h"

#if defined(__ANDROID__) //Android
#   include <jni.h>            //for JNIEnv, jobject
#   include <android/log.h>    //for __android_log_print()
#   define SCN_PRINTF_INFO(STR_FMT, ...)   __android_log_print(ANDROID_LOG_INFO, "Scn", STR_FMT, ##__VA_ARGS__)
#   define SCN_PRINTF_ERROR(STR_FMT, ...)  __android_log_print(ANDROID_LOG_ERROR, "Scn", "ERROR, "STR_FMT, ##__VA_ARGS__)
#   define SCN_PRINTF_WARNING(STR_FMT, ...) __android_log_print(ANDROID_LOG_WARN, "Scn", "WARNING, "STR_FMT, ##__VA_ARGS__)
#elif defined(__QNX__) //BB10
#   define SCN_PRINTF_INFO(STR_FMT, ...)   fprintf(stdout, "Scn, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#   define SCN_PRINTF_ERROR(STR_FMT, ...)  fprintf(stderr, "Scn ERROR, " STR_FMT, ##__VA_ARGS__); fflush(stderr)
#   define SCN_PRINTF_WARNING(STR_FMT, ...) fprintf(stdout, "Scn WARNING, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#else
#   define SCN_PRINTF_INFO(STR_FMT, ...)   printf("Scn, " STR_FMT, ##__VA_ARGS__)
#   define SCN_PRINTF_ERROR(STR_FMT, ...)   printf("Scn ERROR, " STR_FMT, ##__VA_ARGS__)
#   define SCN_PRINTF_WARNING(STR_FMT, ...) printf("Scn WARNING, " STR_FMT, ##__VA_ARGS__)
#endif

#define SCN_MEM_STR_BLOCKS_SZ       2048
#define SCN_MEM_STR_IDX_BLOCKS_SZ   128
#define SCN_MEM_PTRS_BLOCKS_SZ      256

//STScnMemStrs

void ScnMemStrs_init(STScnMemStrs* obj){
    memset(obj, 0, sizeof(*obj));
    //init str buffs
    {
        unsigned long sizeN = SCN_MEM_STR_BLOCKS_SZ;
        char* buffN = (char*)malloc(sizeof(obj->buff[0]) * sizeN);
        if(buffN != NULL){
            obj->buff       = buffN;
            obj->buff[0]    = '\0'; //The first string is always <empty string>
            obj->buffUse    = 1;
            obj->buffSz     = sizeN;
        }
    }
}

void ScnMemStrs_destroy(STScnMemStrs* obj){
    if(obj->buff != NULL) {
        free(obj->buff);
        obj->buff = NULL;
    }
    if(obj->idxs != NULL) {
        free(obj->idxs);
        obj->idxs = NULL;
    }

}

unsigned long ScnMemStrs_find(STScnMemStrs* obj, const char* str){
    unsigned long r = 0;
    if(str == NULL || str[0] == '\0') return r;
    //Look for string
    unsigned long i;
    for(i = 0; i < obj->idxsUse; i++){
        assert(obj->idxs[i] < obj->buffUse);
        if(0 == strcmp(&obj->buff[obj->idxs[i]], str)){
            r = obj->idxs[i];
            break;
        }
    }
    return r;
}

unsigned long ScnMemStrs_findOrAdd(STScnMemStrs* obj, const char* str){
    unsigned long r = 0;
    if(str == NULL || str[0] == '\0') return r;
    //
    r = ScnMemStrs_find(obj, str);
    //Register new string
    if(r == 0){
        unsigned long strLen = strlen(str);
        //resize buffer (if necesary)
        if((obj->buffUse + strLen + 1) > obj->buffSz){
            unsigned long szN = (((obj->buffUse + strLen + 1) + SCN_MEM_STR_BLOCKS_SZ - 1) / SCN_MEM_STR_BLOCKS_SZ * SCN_MEM_STR_BLOCKS_SZ);
            char* buffN = (char*)realloc(obj->buff, sizeof(obj->buff[0]) * szN);
            if(buffN != NULL){
                obj->buff   = buffN;
                obj->buffSz = szN;
            }
        }
        //resize indexes (if necesary)
        if(obj->idxsUse >= obj->idxsSz){
            unsigned long szN = (((obj->idxsUse + 1) + SCN_MEM_STR_IDX_BLOCKS_SZ - 1) / SCN_MEM_STR_IDX_BLOCKS_SZ * SCN_MEM_STR_IDX_BLOCKS_SZ);
            unsigned long* idxsN = (unsigned long*)realloc(obj->idxs, sizeof(obj->idxs[0]) * szN);
            if(idxsN != NULL){
                obj->idxs   = idxsN;
                obj->idxsSz = szN;
            }
        }
        //Add index and string
        if(obj->idxsUse < obj->idxsSz && (obj->buffUse + strLen + 1) <= obj->buffSz){
            obj->idxs[obj->idxsUse++] = r = obj->buffUse;
            memcpy(&obj->buff[obj->buffUse], str, strLen + 1);
            obj->buffUse += (strLen + 1);
        }
    }
    //
    return r;
}

//STScnPtrRetainer

void ScnPtrRetainer_init(STScnPtrRetainer* obj){
    memset(obj, 0, sizeof(*obj));
}

void ScnPtrRetainer_destroy(STScnPtrRetainer* obj){
    memset(obj, 0, sizeof(*obj));
}

//STScnMemBlock

void ScnMemBlock_init(STScnMemBlock* obj){
    memset(obj, 0, sizeof(*obj));
}

void ScnMemBlock_destroy(STScnMemBlock* obj){
    //retains
    {
        if(obj->retains.arr != NULL){
            unsigned long i; for(i = 0; i < obj->retains.use; i++){
                STScnPtrRetainer* rr = &obj->retains.arr[i];
                ScnPtrRetainer_destroy(rr);
            }
            free(obj->retains.arr);
            obj->retains.arr = NULL;
        }
        obj->retains.sz = obj->retains.use = 0;
    }
    memset(obj, 0, sizeof(*obj));
}

void ScnMemBlock_retainedBy(STScnMemBlock* obj, const unsigned long iStrHint){
    //search record
    if(obj->retains.arr != NULL){
        unsigned long i; for(i = 0; i < obj->retains.use; i++){
            STScnPtrRetainer* rr = &obj->retains.arr[i];
            if(rr->iStrHint == iStrHint){
                obj->retains.balance++;
                rr->balance++;
                return;
            }
        }
    }
    //create record
    if(obj->retains.use >= obj->retains.sz){
        unsigned long szN = obj->retains.use + 32;
        STScnPtrRetainer* arrN = (STScnPtrRetainer*)realloc(obj->retains.arr, szN * sizeof(obj->retains.arr[0]));
        if(arrN != NULL){
            obj->retains.arr = arrN;
            obj->retains.sz = szN;
        }
    }
    if(obj->retains.use < obj->retains.sz){
        STScnPtrRetainer* rr = &obj->retains.arr[obj->retains.use++];
        ScnPtrRetainer_init(rr);
        rr->iStrHint = iStrHint;
        rr->balance++;
    }
    obj->retains.balance++;
}

void ScnMemBlock_releasedBy(STScnMemBlock* obj, const unsigned long iStrHint){
    //search record
    if(obj->retains.arr != NULL){
        unsigned long i; for(i = 0; i < obj->retains.use; i++){
            STScnPtrRetainer* rr = &obj->retains.arr[i];
            if(rr->iStrHint == iStrHint){
                obj->retains.balance--;
                rr->balance--;
                return;
            }
        }
    }
    //create record
    if(obj->retains.use >= obj->retains.sz){
        unsigned long szN = obj->retains.use + 32;
        STScnPtrRetainer* arrN = (STScnPtrRetainer*)realloc(obj->retains.arr, szN * sizeof(obj->retains.arr[0]));
        if(arrN != NULL){
            obj->retains.arr = arrN;
            obj->retains.sz = szN;
        }
    }
    if(obj->retains.use < obj->retains.sz){
        STScnPtrRetainer* rr = &obj->retains.arr[obj->retains.use++];
        ScnPtrRetainer_init(rr);
        rr->iStrHint = iStrHint;
        rr->balance--;
    }
    obj->retains.balance--;
}

//STScnMemBlocks

void ScnMemBlocks_init(STScnMemBlocks* obj){
    memset(obj, 0, sizeof(*obj));
}

void ScnMemBlocks_destroy(STScnMemBlocks* obj){
    if(obj->arr){
        unsigned long i; for(i = 0; i < obj->arrUse; i++){
            STScnMemBlock* b = &obj->arr[i];
            ScnMemBlock_destroy(b);
        }
        free(obj->arr);
        obj->arr = NULL;
        obj->arrSz = 0;
        obj->arrUse = 0;
    }
}

STScnMemBlock* ScnMemBlocks_findExact(STScnMemBlocks* obj, void* pointer){
    STScnMemBlock* r = NULL;
    unsigned long i; for(i = 0; i < obj->arrUse; i++){
        STScnMemBlock* b = &obj->arr[i];
        if(b->regUsed && b->pointer == pointer){
            r = b;
            break;
        }
    }
    return r;
}

STScnMemBlock* ScnMemBlocks_findContainer(STScnMemBlocks* obj, void* pointer){
    STScnMemBlock* r = NULL;
    unsigned long i; for(i = 0; i < obj->arrUse; i++){
        STScnMemBlock* b = &obj->arr[i];
        if(b->regUsed && b->pointer <= pointer && pointer <= (void*)((char*)b->pointer + b->bytes)){
            r = b;
            break;
        }
    }
    return r;
}

STScnMemBlock* ScnMemBlocks_add(STScnMemBlocks* obj, void* pointer, unsigned long bytes, const unsigned long iStrHint){
    STScnMemBlock* r = NULL;
    //search for an available record
    {
        unsigned long i; for(i = 0; i < obj->arrUse; i++){
            STScnMemBlock* b = &obj->arr[i];
            if(!b->regUsed){
                r = b;
                break;
            }
        }
    }
    //create new record
    if(r == NULL){
        //resize indexes (if necesary)
        if(obj->arrUse >= obj->arrSz){
            unsigned long szN = (((obj->arrUse + 1) + SCN_MEM_PTRS_BLOCKS_SZ - 1) / SCN_MEM_PTRS_BLOCKS_SZ * SCN_MEM_PTRS_BLOCKS_SZ);
            STScnMemBlock* arrN = (STScnMemBlock*)realloc(obj->arr, sizeof(obj->arr[0]) * szN);
            if(arrN != NULL){
                obj->arr   = arrN;
                obj->arrSz = szN;
            }
        }
        //
        if(obj->arrUse < obj->arrSz){
            r = &obj->arr[obj->arrUse++];
            ScnMemBlock_init(r);
        }
    }
    //populate
    if(r){
        r->regUsed  = 1;
        r->pointer  = pointer;
        r->bytes    = bytes;
        r->iStrHint = iStrHint;
    }
    return r;
}

//STScnMemStats

void ScnMemStats_init(STScnMemStats* obj){
    memset(obj, 0, sizeof(*obj));
}

void ScnMemStats_destroy(STScnMemStats* obj){
    memset(obj, 0, sizeof(*obj));
}

//---------------------------------------------
//-- Usefull implementation for memory leaking
//-- detection and tracking.
//---------------------------------------------

void ScnMemMap_init(STScnMemMap* obj){
    memset(obj, 0, sizeof(*obj));
    ScnMemStats_init(&obj->stats);
    ScnMemStrs_init(&obj->strs);
    ScnMemBlocks_init(&obj->blocks);
}

void ScnMemMap_destroy(STScnMemMap* obj){
    ScnMemStats_destroy(&obj->stats);
    ScnMemStrs_destroy(&obj->strs);
    ScnMemBlocks_destroy(&obj->blocks);
}

//mem

void ScnMemMap_ptrAdd(STScnMemMap* obj, void* pointer, unsigned long bytes, const char* strHint){
    assert(NULL == ScnMemBlocks_findExact(&obj->blocks, pointer)); //should not be registered
    //Find or register hint string
    const unsigned long iStrHint = ScnMemStrs_findOrAdd(&obj->strs, strHint);
    if(NULL != ScnMemBlocks_add(&obj->blocks, pointer, bytes, iStrHint)){
        obj->stats.alive.count++;
        obj->stats.alive.bytes += bytes;
        //
        obj->stats.total.count++;
        obj->stats.total.bytes += bytes;
        //
        if(obj->stats.max.count < obj->stats.alive.count) obj->stats.max.count = obj->stats.alive.count;
        if(obj->stats.max.bytes < obj->stats.alive.bytes) obj->stats.max.bytes = obj->stats.alive.bytes;
    }
}

void ScnMemMap_ptrRemove(STScnMemMap* obj, void* pointer){
    unsigned long i; const unsigned long use = obj->blocks.arrUse;
    for(i = 0; i < use; i++){
        STScnMemBlock* b = &obj->blocks.arr[i];
        if(b->regUsed && b->pointer == pointer){
            b->regUsed = 0;
            obj->stats.alive.count--;
            obj->stats.alive.bytes -= b->bytes;
            return;
        }
    }
    assert(0); //Pointer was not found
}

//dbg

void ScnMemMap_ptrRetainedBy(STScnMemMap* obj, void* pointer, const char* by){
    STScnMemBlock* b = ScnMemBlocks_findExact(&obj->blocks, pointer);
    assert(b != NULL); //should be registered
    if(b != NULL){
        const unsigned long iStrHint = ScnMemStrs_findOrAdd(&obj->strs, by);
        ScnMemBlock_retainedBy(b, iStrHint);
    }
}

void ScnMemMap_ptrReleasedBy(STScnMemMap* obj, void* pointer, const char* by){
    STScnMemBlock* b = ScnMemBlocks_findExact(&obj->blocks, pointer);
    assert(b != NULL); //should be registered
    if(b != NULL){
        const unsigned long iStrHint = ScnMemStrs_findOrAdd(&obj->strs, by);
        ScnMemBlock_releasedBy(b, iStrHint);
    }
}

//report

void ScnMemMap_printAlivePtrs(STScnMemMap* obj){
    unsigned long i, countUsed = 0, bytesUsed = 0; const unsigned long use = obj->blocks.arrUse;
    for(i = 0; i < use; i++){
        STScnMemBlock* b = &obj->blocks.arr[i];
        if(b->regUsed){
            ++countUsed;
            bytesUsed += b->bytes;
            SCN_PRINTF_INFO("#%lu) %lu, %lu bytes, '%s'\n", countUsed, (unsigned long)b->pointer, b->bytes, &obj->strs.buff[b->iStrHint]);
            if(b->retains.arr != NULL && b->retains.use > 0){
                SCN_PRINTF_INFO("        Retains balance: %ld (should be -1)\n", b->retains.balance);
                unsigned long i; for(i = 0; i < b->retains.use; i++){
                    STScnPtrRetainer* rr = &b->retains.arr[i];
                    SCN_PRINTF_INFO("        %ld by '%s'\n", rr->balance, rr->iStrHint == 0 ? "stack-or-unmanaged-obj": &obj->strs.buff[rr->iStrHint]);
                }
            }
        }
    }
    SCN_PRINTF_INFO("\n");
    SCN_PRINTF_INFO("CURRENTLY USED   : %lu blocks (%lu bytes)\n", obj->stats.alive.count, obj->stats.alive.bytes);
    SCN_PRINTF_INFO("MAX USED         : %lu blocks (%lu bytes)\n", obj->stats.max.count, obj->stats.max.bytes);
    SCN_PRINTF_INFO("TOTAL ALLOCATIONS: %lu blocks (%lu bytes)\n", obj->stats.total.count, obj->stats.total.bytes);
    assert(obj->stats.alive.count == countUsed); //program logic error
    assert(obj->stats.alive.bytes == bytesUsed); //program logic error
}

void ScnMemMap_printFinalReport(STScnMemMap* obj){
    SCN_PRINTF_INFO("-------------- MEM REPORT -----------\n");
    if(obj->stats.alive.count == 0){
        SCN_PRINTF_INFO("Scn: no memory leaking detected :)\n");
    } else {
        SCN_PRINTF_WARNING("WARNING, NIXTLA MEMORY-LEAK DETECTED! :(\n");
    }
    ScnMemMap_printAlivePtrs(obj);
    SCN_PRINTF_INFO("-------------------------------------\n");
}

