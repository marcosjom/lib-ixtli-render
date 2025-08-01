//
//  main.c
//  ixtli-render-demos
//
//  Created by Marcos Ortega on 26/7/25.
//

#include <stdio.h>      //malloc
#include <stdlib.h>     //for rand()
#include <time.h>       //for rand()
#include <string.h>     //memset

//Memory Block
#include "ixrender/core/ScnMemBlock.h"

void memBlock_testsRun_(ScnContextRef ctx, const ScnUI32 ammTests);
void memblock_testRandomActions_(ScnContextRef ctx, const ScnUI32 blockSz, const ScnUI32 ammPtrsMax, const ScnUI32 ammActions);

//Memory Elastic
#include "ixrender/core/ScnMemElastic.h"

void memElastic_testsRun_(ScnContextRef ctx, const ScnUI32 ammTests);
void memElastic_testRandomActions_(ScnContextRef ctx, const ScnUI32 blockSz, const ScnUI32 ammPtrsMax, const ScnUI32 ammActions);

//

int main(int argc, const char * argv[]) {
    STScnContextItf ctxItf = ScnContextItf_getDefault();
    ScnContextRef ctx = ScnContext_alloc(&ctxItf);
    {
        //memblock test
        {
            printf("Start-of-memBlock_testsRun_.\n");
            memBlock_testsRun_(ctx, 100);
            printf("End-of-memBlock_testsRun_.\n");
        }
        //memElastic test
        {
            printf("Start-of-memElastic_testsRun_.\n");
            memElastic_testsRun_(ctx, 100);
            printf("End-of-memElastic_testsRun_.\n");
        }
    }
    ScnContext_releaseAndNull(&ctx);
    return 0;
}

//Memory Block

void memBlock_testsRun_(ScnContextRef ctx, const ScnUI32 ammTests){
    ScnUI32 i; for(i = 0; i < ammTests; i++){
        memblock_testRandomActions_(ctx, rand() % 1024 * 128, 1 + (rand() % 1024), 1 + (rand() % 1024));
        if(((i + 1) % 10) == 0){
            printf("#%d/%d runs-of-memblock_testRandomActions_ (%.1f%%).\n", i + 1, ammTests, (ScnFLOAT)i / (ScnFLOAT)ammTests * 100.f);
        }
    }
    printf("%d/%d runs-of-memblock_testRandomActions_ (%.1f%%, END).\n", i, ammTests, (ScnFLOAT)i / (ScnFLOAT)ammTests * 100.f);
}

void memblock_testRandomActions_(ScnContextRef ctx, const ScnUI32 blockSz, const ScnUI32 ammPtrsMax, const ScnUI32 ammActions){
    STScnAbsPtr* ptrs = (STScnAbsPtr*)malloc(sizeof(STScnAbsPtr) * ammPtrsMax);
    memset(ptrs, 0, sizeof(STScnAbsPtr) * ammPtrsMax);
    //
    {
        ScnMemBlockRef mb = ScnMemBlock_alloc(ctx);
        STScnMemBlockCfg cfg = STScnMemBlockCfg_Zero;
        cfg.size            = blockSz;
        cfg.idxsAlign       = (rand() % 17);
        cfg.sizeAlign       = (rand() % 257);
        cfg.idxZeroIsValid  = (rand() % 2);
        ScnMemBlock_prepare(mb, &cfg, NULL);
        {
            //random actions
            ScnUI32 i; for(i = 0; i < ammActions; i++){
                const ScnUI32 iAct = (rand() % 128);
                if(iAct < 124){
                    //random action
                    const ScnUI32 iPtr = (rand() % ammPtrsMax);
                    if(ptrs[iPtr].ptr != NULL){
                        if(!ScnMemBlock_mfree(mb, ptrs[iPtr])){
                            SCN_ASSERT(ScnFALSE); //pointer should be freed
                            break;
                        } else {
                            ptrs[iPtr].ptr = NULL;
                            SCN_ASSERT(ScnMemBlock_validateIndex(mb))
                        }
                    } else {
                        ptrs[iPtr] = ScnMemBlock_malloc(mb, 1 + (rand() % (1 + blockSz)));
                        SCN_ASSERT(ScnMemBlock_validateIndex(mb))
                    }
                } else if(iAct < 126){
                    //remove all individually
                    ScnUI32 availSz = 0;
                    ScnUI32 i2; for(i2 = 0; i2 < ammPtrsMax && (availSz = ScnMemBlock_mAvailSz(mb)) > 0; i2++){
                        if(ptrs[i2].ptr != NULL){
                            if(!ScnMemBlock_mfree(mb, ptrs[i2])){
                                SCN_ASSERT(ScnFALSE);
                                break;
                            } else {
                                SCN_ASSERT(ScnMemBlock_validateIndex(mb))
                                ptrs[i2].ptr = NULL;
                            }
                        }
                    }
                } else if(iAct < 127){
                    //clear
                    ScnMemBlock_clear(mb);
                    memset(ptrs, 0, sizeof(STScnAbsPtr) * ammPtrsMax);
                    SCN_ASSERT(ScnMemBlock_validateIndex(mb))
                } else {
                    //fill all
                    //STScnTimestampMicro end, start = ScnTimestampMicro_getMonotonicFast();
                    ScnUI32 availSzStart = ScnMemBlock_mAvailSz(mb);
                    ScnUI32 availSz = availSzStart;
                    //printf("memblock_testRandomActions_ filling all memory (START, %d bytes).\n", availSzStart);
                    //
                    ScnMemBlock_prepareForNewMallocsActions(mb, availSz);
                    //
                    ScnUI32 i2; for(i2 = 0; i2 < ammPtrsMax && (availSz = ScnMemBlock_mAvailSz(mb)) > 0; i2++){
                        if(ptrs[i2].ptr == NULL){
                            ptrs[i2] = ScnMemBlock_malloc(mb, 1);
                            SCN_ASSERT(ScnMemBlock_validateIndex(mb))
                            if(ptrs[i2].ptr == NULL){ //this happens when the block is too fragmented
                                break;
                            }
                        }
                    }
                    while((availSz = ScnMemBlock_mAvailSz(mb)) > 0){
                        STScnAbsPtr ptr = ScnMemBlock_malloc(mb, 1);
                        SCN_ASSERT(ScnMemBlock_validateIndex(mb))
                        if(ptr.ptr == NULL){ //this happens when the block is too fragmented
                            break;
                        }
                    }
                    //end = ScnTimestampMicro_getMonotonicFast();
                    //printf("memblock_testRandomActions_ filling all memory (DONE, %d bytes, %d ms).\n", availSzStart, ScnTimestampMicro_getDiffInMs(&start, &end));
                }
            }
        }
        ScnMemBlock_release(&mb);
    }
    //
    free(ptrs);
    ptrs = NULL;
}

//Memory Blocks
void memElastic_testsRun_(ScnContextRef ctx, const ScnUI32 ammTests){
    ScnUI32 i; for(i = 0; i < ammTests; i++){
        memElastic_testRandomActions_(ctx, rand() % 1024 * 128, 1 + (rand() % 1024), 1 + (rand() % 1024));
        if(((i + 1) % 10) == 0){
            printf("#%d/%d runs-of-memElastic_testRandomActions_ (%.1f%%).\n", i + 1, ammTests, (ScnFLOAT)i / (ScnFLOAT)ammTests * 100.f);
        }
    }
    printf("%d/%d runs-of-memElastic_testRandomActions_ (%.1f%%, END).\n", i, ammTests, (ScnFLOAT)i / (ScnFLOAT)ammTests * 100.f);
}

void memElastic_testRandomActions_(ScnContextRef ctx, const ScnUI32 blockSz, const ScnUI32 ammPtrsMax, const ScnUI32 ammActions){
    STScnAbsPtr* ptrs = (STScnAbsPtr*)malloc(sizeof(STScnAbsPtr) * ammPtrsMax);
    memset(ptrs, 0, sizeof(STScnAbsPtr) * ammPtrsMax);
    //
    {
        ScnMemElasticRef mb = ScnMemElastic_alloc(ctx);
        STScnMemElasticCfg cfg = STScnMemElasticCfg_Zero;
        cfg.sizePerBlock    = blockSz;
        cfg.sizeMax         = 0; //
        cfg.idxsAlign       = (rand() % 17);
        cfg.sizeAlign       = (rand() % 257);
        cfg.idxZeroIsValid  = (rand() % 2);
        if(blockSz > 0){
            cfg.sizeInitial  = (rand() % 2) == 0 ? 0 : (rand() % blockSz) * (rand() % 3);
        }
        ScnMemElastic_prepare(mb, &cfg, NULL);
        {
            //random actions
            ScnUI32 i; for(i = 0; i < ammActions; i++){
                const ScnUI32 iAct = (rand() % 128);
                if(iAct < 126){
                    //random action
                    const ScnUI32 iPtr = (rand() % ammPtrsMax);
                    if(ptrs[iPtr].ptr != NULL){
                        if(!ScnMemElastic_mfree(mb, ptrs[iPtr])){
                            SCN_ASSERT(ScnFALSE); //pointer should be freed
                            break;
                        } else {
                            ptrs[iPtr].ptr = NULL;
                            SCN_ASSERT(ScnMemElastic_validateIndex(mb))
                        }
                    } else {
                        ptrs[iPtr] = ScnMemElastic_malloc(mb, 1 + (rand() % (1 + blockSz)), NULL);
                        SCN_ASSERT(ptrs[iPtr].ptr != NULL) //should be allocated
                        SCN_ASSERT(ScnMemElastic_validateIndex(mb))
                    }
                } else if(iAct < 127){
                    //remove all individually
                    ScnUI32 i2; for(i2 = 0; i2 < ammPtrsMax; i2++){
                        if(ptrs[i2].ptr != NULL){
                            if(!ScnMemElastic_mfree(mb, ptrs[i2])){
                                SCN_ASSERT(ScnFALSE);
                                break;
                            } else {
                                SCN_ASSERT(ScnMemElastic_validateIndex(mb))
                                ptrs[i2].ptr = NULL;
                            }
                        }
                    }
                } else {
                    //clear
                    ScnMemElastic_clear(mb);
                    memset(ptrs, 0, sizeof(STScnAbsPtr) * ammPtrsMax);
                    SCN_ASSERT(ScnMemElastic_validateIndex(mb))
                }
            }
        }
        ScnMemElastic_release(&mb);
    }
    //
    free(ptrs);
    ptrs = NULL;
}
