//
//  ScnBitmap.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/type/ScnBitmap.h"


//

typedef struct STScnBitmapColorDesc_ {
    ScnUI8              bitsPerPx;
    const char*         name;
    ENScnBitmapColor    color;
} STScnBitmapColorDesc;

static const STScnBitmapColorDesc ScnBitmap_colorMap[] = {
    {0, "undef", ENScnBitmapColor_undef },
    {8, "ALPHA8", ENScnBitmapColor_ALPHA8},
    {8, "GRIS8", ENScnBitmapColor_GRIS8 },
    {16, "GRISALPHA8", ENScnBitmapColor_GRISALPHA8 },
    {12, "RGB4", ENScnBitmapColor_RGB4 },
    {24, "RGB8", ENScnBitmapColor_RGB8 },
    {16, "RGBA4", ENScnBitmapColor_RGBA4 },
    {32, "RGBA8", ENScnBitmapColor_RGBA8 },
    {16, "ARGB4", ENScnBitmapColor_ARGB4 },
    {32, "ARGB8", ENScnBitmapColor_ARGB8 },
    {32, "BGRA8", ENScnBitmapColor_BGRA8 },
    {16, "SWF_PIX15", ENScnBitmapColor_SWF_PIX15 },
    {32, "SWF_PIX24", ENScnBitmapColor_SWF_PIX24 }
};

//STScnBitmapProps

STScnBitmapProps ScnBitmapProps_build(const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color){
    STScnBitmapProps r = STScnBitmapProps_Zero;
    if(color < (sizeof(ScnBitmap_colorMap) / sizeof(ScnBitmap_colorMap[0]))){
        const STScnBitmapColorDesc* d = &ScnBitmap_colorMap[color];
        if(d->bitsPerPx == 0){
            ScnMemory_setZeroSt(r, STScnBitmapProps);
        } else {
            r.bitsPerPx = d->bitsPerPx;
            {
                const ScnUI32 align = 4;
                const ScnUI32 bitsPerLine = (width * d->bitsPerPx);
                //aligned to byte and word
                r.bytesPerLine    = (((bitsPerLine / 8) + ((bitsPerLine % 8) != 0 ? 1 : 0)) + align - 1) / align * align;
            }
            r.color         = color;
            r.size.width    = width;
            r.size.height   = height;
        }
    }
    return r;
}

//STScnBitmapOpq

typedef struct STScnBitmapOpq_ {
    ScnContextRef     ctx;
    ScnMutexRef       mutex;
    STScnBitmapProps    props;
    ScnBYTE*            data;
    ScnUI32             dataSz;
} STScnBitmapOpq;

//

ScnSI32 ScnBitmap_getOpqSz(void){
    return (ScnSI32)sizeof(STScnBitmapOpq);
}

void ScnBitmap_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnBitmapOpq* opq = (STScnBitmapOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
}

void ScnBitmap_destroyOpq(void* obj){
    STScnBitmapOpq* opq = (STScnBitmapOpq*)obj;
    //data
    if(opq->data != NULL){
        ScnContext_mfree(opq->ctx, opq->data);
        opq->data = NULL;
        opq->dataSz = 0;
    }
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnBOOL ScnBitmap_create(ScnBitmapRef ref, const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color){
    ScnBOOL r = ScnFALSE;
    STScnBitmapOpq* opq = (STScnBitmapOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(width > 0 && height > 0){
        const STScnBitmapProps props = ScnBitmapProps_build(width, height, color);
        if(props.bitsPerPx > 0){
            const ScnUI32 szN = (props.bytesPerLine * props.size.height); SCN_ASSERT(szN > 0)
            if(szN != opq->dataSz){
                ScnBYTE* dataN = ScnContext_mrealloc(opq->ctx, opq->data, szN, "ScnBitmap_createBuffer::data");
                if(dataN != NULL){
                    opq->data   = dataN;
                    opq->dataSz = szN;
                }
            }
            if(szN == opq->dataSz){
                opq->props = props;
                r = ScnTRUE;
            }
        }
    }
    return r;
}

ScnBOOL ScnBitmap_pasteBitmapData(ScnBitmapRef ref, const STScnPointI pos, const STScnBitmapProps srcProps, const ScnBYTE* srcData){
    ScnBOOL r = ScnFALSE;
    //STScnBitmapOpq* opq = (STScnBitmapOpq*)ScnSharedPtr_getOpq(ref.ptr);
    SCN_ASSERT(ScnFALSE) //implement
    return r;
}
