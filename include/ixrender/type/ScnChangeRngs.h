//
//  ScnChangeRngs.h
//  ixtli-render
//
//  Created by Marcos Ortega on 9/8/25.
//

#ifndef ScnChangeRngs_h
#define ScnChangeRngs_h

#include "ixrender/type/ScnRange.h"
#include "ixrender/core/ScnArraySorted.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnChangesRngs

typedef struct STScnChangesRngs {
    ScnContextRef   ctx;
    ScnBOOL         all; //the whole buffer must be synchronized
    ScnUI32         rngsBytesAccum;
    ScnArraySortedStruct(rngs, STScnRangeU);
} STScnChangesRngs;

void    ScnChangesRngs_init(ScnContextRef ctx, STScnChangesRngs* obj);
void    ScnChangesRngs_destroy(STScnChangesRngs* obj);
void    ScnChangesRngs_reset(STScnChangesRngs* obj);
void    ScnChangesRngs_invalidateAll(STScnChangesRngs* obj);
ScnBOOL ScnChangesRngs_mergeRng(STScnChangesRngs* obj, const STScnRangeU* rng);
void    ScnChangesRngs_mergeWithOther(STScnChangesRngs* obj, const STScnChangesRngs* const other);
ScnBOOL ScnChangesRngs_validate(STScnChangesRngs* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnChangeRngs_h */
