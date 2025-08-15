//
//  ScnRange.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/type/ScnRange.h"

//STScnRange

ScnSI32 ScnCompare_STScnRange(const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRange))
    if(dataSz == sizeof(STScnRange)){
        const STScnRange* d1 = (STScnRange*)data1;
        const STScnRange* d2 = (STScnRange*)data2;
        return (d1->start < d2->start ? -1 : d1->start > d2->start ? 1 : 0);
    }
    return -2;
}

//STScnRangeI

ScnSI32 ScnCompare_STScnRangeI(const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRangeI))
    if(dataSz == sizeof(STScnRangeI)){
        const STScnRangeI* d1 = (STScnRangeI*)data1;
        const STScnRangeI* d2 = (STScnRangeI*)data2;
        return (d1->start < d2->start ? -1 : d1->start > d2->start ? 1 : 0);
    }
    return -2;
}

//STScnRangeU

ScnSI32 ScnCompare_STScnRangeU(const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRangeU))
    if(dataSz == sizeof(STScnRangeU)){
        const STScnRangeU* d1 = (STScnRangeU*)data1;
        const STScnRangeU* d2 = (STScnRangeU*)data2;
        return (d1->start < d2->start ? -1 : d1->start > d2->start ? 1 : 0);
    }
    return -2;
}

