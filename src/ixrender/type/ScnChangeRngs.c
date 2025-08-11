//
//  ScnChangeRngs.c
//  ixtli-render
//
//  Created by Marcos Ortega on 9/8/25.
//

#include "ixrender/type/ScnChangeRngs.h"


ScnBOOL ScnChangesRngs_mergeRng(STScnChangesRngs* obj, const STScnRangeU* rng){
    SCN_ASSERT(!obj->all) //optimization, call only if obj->all == ScnFALSE
    STScnRangeU* gStart = obj->rngs.arr;
    const ScnSI32 iNxtRng = ScnArraySorted_indexForNew(&obj->rngs, rng); SCN_ASSERT(iNxtRng >= 0)
    STScnRangeU* mrged = NULL;
#   ifdef SCN_ASSERTS_ACTIVATED
    STScnRangeU rngParam = *rng;
    ScnBOOL newRngAdded = ScnFALSE;
    ScnBOOL newRngExpandedPrevRng = ScnFALSE;
    ScnBOOL newRngExpandedNextRng = ScnFALSE;
    ScnUI32 newRngMergedAfterCount = 0;
    rngParam.size = rngParam.size; //just to silcence 'unused var' warning
#   endif
    //eval against prev range
    if(iNxtRng > 0){
        STScnRangeU* prv = &gStart[iNxtRng - 1];
        SCN_ASSERT(prv->start <= rng->start)
        if((prv->start + prv->size) >= (rng->start + rng->size)){
            //new range fully covered by previous range
            return ScnTRUE;
        } else if((prv->start + prv->size) >= rng->start){
            //new range expands prev range
            SCN_ASSERT((rng->start + rng->size) >= prv->start);
            obj->rngsBytesAccum += (rng->start + rng->size) - (prv->start + prv->size);
            prv->size = (rng->start + rng->size) - prv->start;
            mrged = prv;
            rng = NULL;
#           ifdef SCN_ASSERTS_ACTIVATED
            newRngExpandedPrevRng = ScnTRUE;
#           endif
        }
        SCN_ASSERT(rng == NULL || (prv->start + prv->size) < rng->start)
    }
    //eval against next range
    if(mrged == NULL && iNxtRng < obj->rngs.use){
        STScnRangeU* nxt = &gStart[iNxtRng];
        SCN_ASSERT(rng->start <= nxt->start)
        if(nxt->start == rng->start && (nxt->start + nxt->size) >= (rng->start + rng->size)){
            //new range fully covered by next range
            return ScnTRUE;
        } else if((rng->start + rng->size) >= nxt->start){
            //new range expands next range
            SCN_ASSERT((nxt->start + nxt->size) >= rng->start);
            obj->rngsBytesAccum += (nxt->start - rng->start);
            nxt->size = (nxt->start + nxt->size) - rng->start;
            nxt->start = rng->start;
            mrged = nxt;
            rng = NULL;
#           ifdef SCN_ASSERTS_ACTIVATED
            newRngExpandedNextRng = ScnTRUE;
#           endif
        }
    }
    //eval new merged-range consuming othe ranges
    if(mrged != NULL){
        ScnUI32 prvAfterEnd;
        STScnRangeU* prv = mrged++; SCN_ASSERT(rng == NULL) //should be consumed
        const STScnRangeU* gAfterEnd = obj->rngs.arr + obj->rngs.use;
        while(mrged < gAfterEnd){
            SCN_ASSERT(prv->start <= mrged->start)
            prvAfterEnd = (prv->start + prv->size);
            if(prvAfterEnd >= mrged->start){
                //prev range collides with next
                if(prvAfterEnd < (mrged->start + mrged->size)){
                    //prev range is expanded by next range
                    obj->rngsBytesAccum += (mrged->start + mrged->size) - prvAfterEnd;
                    prv->size = (mrged->start + mrged->size) - prv->start;
                }
                SCN_ASSERT(obj->rngsBytesAccum >= mrged->size)
                obj->rngsBytesAccum -= mrged->size;
                //remove by filling gap in array
                {
                    STScnRangeU* r = mrged + 1;
                    while(r < gAfterEnd){
                        *(r - 1) = *r;
                        r++;
                    }
                }
#               ifdef SCN_ASSERTS_ACTIVATED
                ++newRngMergedAfterCount;
#               endif
                //
                --mrged;
                --gAfterEnd;
                --obj->rngs.use;
            }
            //next
            prv = mrged++;
        }
        SCN_ASSERT(ScnChangesRngs_validate(obj));
    }
    //add new range (not merged)
    if(rng != NULL && NULL != ScnArraySorted_addPtr(obj->ctx, &obj->rngs, rng, STScnRangeU)){
        obj->rngsBytesAccum += rng->size;
#       ifdef SCN_ASSERTS_ACTIVATED
        newRngAdded = ScnTRUE;
#       endif
        SCN_ASSERT(ScnChangesRngs_validate(obj));
        return ScnTRUE;
    }
    return ScnFALSE;
}

void ScnChangesRngs_invalidateAll(STScnChangesRngs* obj){
    obj->all = ScnTRUE;
    ScnArraySorted_empty(&obj->rngs);
}

//STScnChangesRngs

void ScnChangesRngs_init(ScnContextRef ctx, STScnChangesRngs* obj){
    ScnMemory_setZeroSt(*obj);
    ScnContext_set(&obj->ctx, ctx);
    ScnArraySorted_init(ctx, &obj->rngs, 0, 128, STScnRangeU, ScnCompare_STScnRangeU);
}

void ScnChangesRngs_destroy(STScnChangesRngs* obj){
    ScnArraySorted_destroy(obj->ctx, &obj->rngs);
    ScnContext_releaseAndNull(&obj->ctx);
}

#ifdef SCN_DEBUG
ScnBOOL ScnChangesRngs_validate(STScnChangesRngs* obj){
    ScnBOOL r = ScnTRUE;
    ScnUI32 prevAfterEnd = 0, szAccum = 0;
    const STScnRangeU* rng = obj->rngs.arr;
    const STScnRangeU* rngAfterEnd = rng + obj->rngs.use;
    while(rng < rngAfterEnd){
        if(prevAfterEnd > rng->start){
            SCN_ASSERT(ScnFALSE)
            r = ScnFALSE;
            break;
        }
        prevAfterEnd = rng->start + rng->size;
        szAccum += rng->size;
        //next
        ++rng;
    }
    if(obj->rngsBytesAccum != szAccum){
        SCN_ASSERT(ScnFALSE)
        r = ScnFALSE;
    }
    return r;
}
#endif

void ScnChangesRngs_reset(STScnChangesRngs* obj){
    obj->all = ScnFALSE;
    obj->rngsBytesAccum = 0;
    ScnArraySorted_empty(&obj->rngs);
}

void ScnChangesRngs_mergeWithOther(STScnChangesRngs* obj, const STScnChangesRngs* const other){
    if(!obj->all){
        if(other->all){
            //all
            ScnChangesRngs_reset(obj);
            obj->all = ScnTRUE;
        } else {
            //merge ranges
            const STScnRangeU* rng = other->rngs.arr;
            if(obj->rngs.use == 0){
                //memcpy
                if(other->rngs.use > 0){
                    ScnArraySorted_prepareForGrowth(obj->ctx, &obj->rngs, other->rngs.use, STScnRangeU);
                    if(other->rngs.use <= obj->rngs.sz){
                        ScnMemcpy(obj->rngs.arr, other->rngs.arr, sizeof(obj->rngs.arr[0]) * other->rngs.use);
                        obj->rngs.use = other->rngs.use;
                    }
                }
                obj->rngsBytesAccum = other->rngsBytesAccum;
                SCN_ASSERT(obj->rngs.use == other->rngs.use)
                SCN_ASSERT(ScnChangesRngs_validate(obj))
            } else {
                //one by one
                const STScnRangeU* rngAfterEnd = rng + other->rngs.use;
                while(rng < rngAfterEnd){
                    if(!ScnChangesRngs_mergeRng(obj, rng)){
                        break;
                    }
                    //next
                    ++rng;
                }
            }
        }
    }
}
