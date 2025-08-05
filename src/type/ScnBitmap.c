//
//  ScnBitmap.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/type/ScnBitmap.h"


//

typedef struct STScnBitmapColorDesc {
    ScnUI8              bitsPerPx;
    const char*         name;
    ENScnBitmapColor    color;
} STScnBitmapColorDesc;

static const STScnBitmapColorDesc ScnBitmap_colorMap[] = {
    {0, "undef", ENScnBitmapColor_undef },
    {8, "ALPHA8", ENScnBitmapColor_ALPHA8},
    {8, "GRAY8", ENScnBitmapColor_GRAY8 },
    {16, "GRAYALPHA8", ENScnBitmapColor_GRAYALPHA8 },
    {24, "RGB8", ENScnBitmapColor_RGB8 },
    {32, "RGBA8", ENScnBitmapColor_RGBA8 },
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

typedef struct STScnBitmapOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
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

STScnBitmapProps ScnBitmap_getProps(ScnBitmapRef ref, void** optDstData){
    STScnBitmapProps r = STScnBitmapProps_Zero;
    STScnBitmapOpq* opq = (STScnBitmapOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        if(optDstData != NULL) *optDstData = opq->data;
        r = opq->props;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

void* ScnBitmap_getData(ScnBitmapRef ref){
    STScnBitmapOpq* opq = (STScnBitmapOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->data;
}


//

ScnBOOL ScnBitmap_create(ScnBitmapRef ref, const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color){
    ScnBOOL r = ScnFALSE;
    STScnBitmapOpq* opq = (STScnBitmapOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(!(width > 0 && height > 0 && color > ENScnBitmapColor_undef && color < ENScnBitmapColor_Count)){
        return r;
    }
    ScnMutex_lock(opq->mutex);
    {
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
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnBitmap_pasteBitmapData(ScnBitmapRef ref, const STScnPoint2DI pDstPos, const STScnRectI pSrcRect, const STScnBitmapProps* const srcProps, const void* srcData){
    ScnBOOL r = ScnFALSE;
    STScnBitmapOpq* opq = (STScnBitmapOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(!(srcProps != NULL && srcData != NULL && pSrcRect.width >= 0 && pSrcRect.height >= 0)){
        return r;
    } else if((pSrcRect.x + pSrcRect.width) <= 0 || (pSrcRect.y + pSrcRect.height) <= 0){
        //nothing to copy from source
        r = ScnTRUE;
        return r;
    } else if(pSrcRect.x >= srcProps->size.width || pSrcRect.y >= srcProps->size.height){
        //nothing to copy from source
        r = ScnTRUE;
        return r;
    }
    //normalize srcRect
    STScnRectI srcRectNorm = pSrcRect;
    if(srcRectNorm.x < 0){ srcRectNorm.width += srcRectNorm.x; srcRectNorm.x = 0; }
    if(srcRectNorm.y < 0){ srcRectNorm.height += srcRectNorm.y; srcRectNorm.y = 0; }
    if((srcRectNorm.x + srcRectNorm.width) > srcProps->size.width) { srcRectNorm.width = srcProps->size.width - srcRectNorm.x; }
    if((srcRectNorm.y + srcRectNorm.height) > srcProps->size.height) { srcRectNorm.height = srcProps->size.height - srcRectNorm.y; }
    //
    ScnMutex_lock(opq->mutex);
    if(opq->props.color != srcProps->color){
        printf("ERROR, ScnBitmap_pasteBitmapData color missmatch.\n");
    } else if((pDstPos.x + srcRectNorm.width) <= 0 || (pDstPos.y + srcRectNorm.height) <= 0){
        //nothing top copy to dst
        r = ScnTRUE;
    } else if(pDstPos.x >= opq->props.size.width || pDstPos.y >= opq->props.size.height){
        //nothing top copy to dst
        r = ScnTRUE;
    } else {
        STScnPoint2DI dstPosNorm = pDstPos;
        if(dstPosNorm.x < 0){ srcRectNorm.width += dstPosNorm.x; dstPosNorm.x = 0; }
        if(dstPosNorm.y < 0){ srcRectNorm.height += dstPosNorm.y; dstPosNorm.y = 0; }
        if((dstPosNorm.x + srcRectNorm.width) > opq->props.size.width) { srcRectNorm.width = opq->props.size.width - dstPosNorm.x; }
        if((dstPosNorm.y + srcRectNorm.height) > opq->props.size.height) { srcRectNorm.height = opq->props.size.height - dstPosNorm.y; }
        SCN_ASSERT(dstPosNorm.x >= 0 && dstPosNorm.y >= 0 && dstPosNorm.x < opq->props.size.width && dstPosNorm.y < opq->props.size.height)
        SCN_ASSERT(srcRectNorm.width > 0 && srcRectNorm.height > 0)
        SCN_ASSERT(srcRectNorm.x >= 0 && srcRectNorm.y >= 0 && (srcRectNorm.x + srcRectNorm.width) <= srcProps->size.width && (srcRectNorm.y + srcRectNorm.height) <= srcProps->size.height)
        //copy
        if(dstPosNorm.x == 0 && srcRectNorm.width == opq->props.size.width && opq->props.bytesPerLine == srcProps->bytesPerLine){
            //full lines memcpy
            memcpy(opq->data, srcData, opq->props.bytesPerLine * opq->props.size.height);
            r = ScnTRUE;
        } else {
            //copy lines
            ScnBYTE* dst = &opq->data[(dstPosNorm.y * opq->props.bytesPerLine) + (dstPosNorm.x * (opq->props.bitsPerPx / 8))];
            const ScnBYTE* src = &((ScnBYTE*)srcData)[(srcRectNorm.y * srcProps->bytesPerLine) + (srcRectNorm.x * (srcProps->bitsPerPx / 8))];
            const ScnBYTE* srcAfterEnd = src + (srcRectNorm.height * srcProps->bytesPerLine);
            const ScnUI32 copyPerLn = srcRectNorm.width * (srcProps->bitsPerPx / 8);
            while(src < srcAfterEnd){
                memcpy(dst, src, copyPerLn);
                //next line
                dst += opq->props.bytesPerLine;
                src += srcProps->bytesPerLine;
            }
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
