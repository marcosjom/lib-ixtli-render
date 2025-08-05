//
//  ScnRange.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/type/ScnRange.h"

//STScnRange

ScnBOOL ScnCompare_STScnRange(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRange))
    if(dataSz == sizeof(STScnRange)){
        const STScnRange* d1 = (STScnRange*)data1;
        const STScnRange* d2 = (STScnRange*)data2;
        switch (mode) {
            case ENScnCompareMode_Equal: return d1->start == d2->start;
            case ENScnCompareMode_Lower: return d1->start < d2->start;
            case ENScnCompareMode_LowerOrEqual: return d1->start <= d2->start;
            case ENScnCompareMode_Greater: return d1->start > d2->start;
            case ENScnCompareMode_GreaterOrEqual: return d1->start >= d2->start;
            default: SCN_ASSERT(ScnFALSE) break;
        }
    }
    return ScnFALSE;
}

//STScnRangeI

ScnBOOL ScnCompare_STScnRangeI(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRangeI))
    if(dataSz == sizeof(STScnRangeI)){
        const STScnRangeI* d1 = (STScnRangeI*)data1;
        const STScnRangeI* d2 = (STScnRangeI*)data2;
        switch (mode) {
            case ENScnCompareMode_Equal: return d1->start == d2->start;
            case ENScnCompareMode_Lower: return d1->start < d2->start;
            case ENScnCompareMode_LowerOrEqual: return d1->start <= d2->start;
            case ENScnCompareMode_Greater: return d1->start > d2->start;
            case ENScnCompareMode_GreaterOrEqual: return d1->start >= d2->start;
            default: SCN_ASSERT(ScnFALSE) break;
        }
    }
    return ScnFALSE;
}

//STScnRangeU

ScnBOOL ScnCompare_STScnRangeU(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRangeU))
    if(dataSz == sizeof(STScnRangeU)){
        const STScnRangeU* d1 = (STScnRangeU*)data1;
        const STScnRangeU* d2 = (STScnRangeU*)data2;
        switch (mode) {
            case ENScnCompareMode_Equal: return d1->start == d2->start;
            case ENScnCompareMode_Lower: return d1->start < d2->start;
            case ENScnCompareMode_LowerOrEqual: return d1->start <= d2->start;
            case ENScnCompareMode_Greater: return d1->start > d2->start;
            case ENScnCompareMode_GreaterOrEqual: return d1->start >= d2->start;
            default: SCN_ASSERT(ScnFALSE) break;
        }
    }
    return ScnFALSE;
}

