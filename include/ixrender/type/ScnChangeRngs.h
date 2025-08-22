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

/** @struct STScnChangesRngs
 *  @brief Describes an accumulation of changes in non-overlaping ranges.
 *  @var STScnChangesRngs::ctx
 *  ScnContext for memory allocation.
 *  @var STScnChangesRngs::all
 *  Describes if the whole object has changed (probably is a new object).
 *  @var STScnChangesRngs::rngsBytesAccum
 *  The sum of the ranges widths.
 *  @var STScnChangesRngs::rngs
 *  Sorted array of non-overlaping ranges; empty if all is ScnTRUE.
 */

#define ScnChangesRngs_Zero { ScnContextRef_Zero, ScnFALSE, 0, ScnArraySorted_Zero }

typedef struct STScnChangesRngs {
    ScnContextRef   ctx;
    ScnBOOL         all; //the whole buffer must be synchronized
    ScnUI32         rngsBytesAccum;
    ScnArraySortedStruct(rngs, STScnRangeU);
} STScnChangesRngs;

/**
 * @brief Initializes the object.
 * @param ctx ScnContext for memory allocation.
 * @param obj Objetc's pointer.
 */
void ScnChangesRngs_init(ScnContextRef ctx, STScnChangesRngs* obj);

/**
 * @brief Destroys the object.
 * @param obj Objetc's pointer.
 */
void ScnChangesRngs_destroy(STScnChangesRngs* obj);

/**
 * @brief Resets the object to it's zero-changes state.
 * @param obj Objetc's pointer.
 */
void ScnChangesRngs_reset(STScnChangesRngs* obj);

/**
 * @brief Flags as the whole range has changed. The ranges array is emptied.
 * @param obj Objetc's pointer.
 */
void ScnChangesRngs_invalidateAll(STScnChangesRngs* obj);

/**
 * @brief Adds or merges the provided range with the current ranges array.
 * @note The resulting array contains non-overlaping ranges only.
 * @param obj Objetc's pointer.
 * @param rng Range to add or merge.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnChangesRngs_mergeRng(STScnChangesRngs* obj, const STScnRangeU* rng);

/**
 * @brief Adds or merges the provided ranges with the current ranges array.
 * @note The resulting array contains non-overlaping ranges only.
 * @param obj Objetc's pointer.
 * @param other Ranges to add or merge.
 */
void ScnChangesRngs_mergeWithOther(STScnChangesRngs* obj, const STScnChangesRngs* const other);

/**
 * @brief Validates the current non-overlaping ranges array and accumulation.
 * @note This is for debug of the internal algorythm only.
 * @param obj Objetc's pointer.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnChangesRngs_validate(STScnChangesRngs* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnChangeRngs_h */
